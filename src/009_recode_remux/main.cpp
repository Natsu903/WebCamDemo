#include <iostream>
#include <thread>
#include "demux.h"
#include "decode.h"
#include "encode.h"
#include "mux.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

using namespace std;

int main(int argc,char* argv[])
{
	//使用说明
	string useage = "recode_remux 输入文件 输出文件 开始时间（秒） 结束时间（秒） 视频宽 视频高\n";
	useage += "recode_remux v1080.mp4 test_out.mp4 10 20 400 300";
	cout << useage << endl;

	//参数不够返回-1
	if (argc < 3)
	{
		return -1;
	}

	string in_file = argv[1];//源文件
	string out_file = argv[2];//转换好的文件

	//截取音视频
	int begin_sec = 0;//截取开始时间
	int end_sec = 0;//截取结束世界
	if (argc > 3)
		begin_sec = atoi(argv[3]);
	if (argc > 4)
		end_sec = atoi(argv[4]);

	int video_width = 0;
	int video_height = 0;
	if (argc > 6)
	{
		int video_width = atoi(argv[5]);
		int video_height = atoi(argv[6]);
	}

	//解封装上下文
	Demux demux;
	auto demux_c = demux.Open(in_file.c_str());
	demux.set_c(demux_c);

	long long video_begin_pts = 0;
	long long audio_begin_pts = 0;
	long long video_end_pts = 0;

	//开始截断秒数，计算输入视频的pts
	if (demux.video_index() >= 0 && demux.video_time_base().num > 0)
	{
		double t = (double)demux.video_time_base().den / (double)demux.video_time_base().num;
		video_begin_pts = t * begin_sec;
		video_end_pts = t * end_sec;
		demux.Seek(video_begin_pts, demux.video_index());
	}
	//计算音频的开始播放pts
	if (demux.audio_index() >= 0 && demux.audio_time_base().num > 0)
	{
		double t = (double)demux.audio_time_base().den / (double)demux.audio_time_base().num;
		audio_begin_pts = t * begin_sec;
	}

	//视频解码初始化
	Decode decode;
	auto decode_c = decode.Create(demux.video_codec_id(), false);
	//设置视频解码器参数
	demux.CopyPara(demux.video_index(), decode_c);
	decode.set_c(decode_c);
	decode.Open();
	auto frame = decode.CreateFrame();//解码后存储

	//视频编码初始化
	if (demux.video_index() >= 0)
	{
		if (video_width <= 0)
			video_width = demux_c->streams[demux.video_index()]->codecpar->width;
		if (video_height <= 0)
			video_height = demux_c->streams[demux.video_index()]->codecpar->height;
	}

	Encode encode;
	auto encode_c = encode.Create(AV_CODEC_ID_H265, true);
	encode_c->pix_fmt = AV_PIX_FMT_YUV420P;
	encode_c->width = video_width;
	encode_c->height = video_height;
	encode.set_c(encode_c);
	encode.Open();

	//封装
	Mux mux;
	auto mux_c = mux.Open(out_file.c_str());
	mux.set_c(mux_c);
	auto mvs = mux_c->streams[mux.video_index()];
	auto mas = mux_c->streams[mux.audio_index()];

	//有视频
	if (demux.video_index() >= 0)
	{
		mvs->time_base.num = demux.video_time_base().num;
		mvs->time_base.den = demux.video_time_base().den;
		// 复制编码器格式
		avcodec_parameters_from_context(mvs->codecpar, encode_c);
	}

	//有音频
	if (demux.audio_index() >= 0)
	{
		mas->time_base.num = demux.audio_time_base().num;
		mas->time_base.den = demux.audio_time_base().den;
		//复制音频参数
		demux.CopyPara(demux.audio_index(), mas->codecpar);
	}

	//写入头部
	mux.WriteHead();

	int audio_count = 0;
	int video_count = 0;
	double total_sec = 0;
	AVPacket pkt;
	for (;;)
	{
		if (!demux.Read(&pkt))
		{
			break;
		}

		// 视频 时间大于结束时间
		if (video_end_pts > 0
			&& pkt.stream_index == demux.video_index()
			&& pkt.pts > video_end_pts)
		{
			av_packet_unref(&pkt);
			break;
		}

		if (pkt.stream_index == demux.video_index()) //视频
		{
			mux.RescaleTime(&pkt, video_begin_pts, demux.video_time_base());

			//解码视频
			if (decode.Send(&pkt))
			{
				while (decode.Recv(frame))
				{
					// 修改图像尺寸 
					//视频编码
					auto epkt = encode.DoEncode(frame);
					if (epkt)
					{
						epkt->stream_index = mux.video_index();
						//写入视频帧 会清理pkt
						mux.Write(epkt);
						//av_packet_free(&epkt);
					}
				}
			}

			video_count++;
			if (demux.video_time_base().den > 0)
				total_sec += pkt.duration * ((double)demux.video_time_base().num / (double)demux.video_time_base().den);
			av_packet_unref(&pkt);
		}
		else if (pkt.stream_index == demux.audio_index())
		{
			mux.RescaleTime(&pkt, audio_begin_pts, demux.audio_time_base());
			audio_count++;
			//写入音频帧 会清理pkt
			mux.Write(&pkt);
		}
		else
		{
			av_packet_unref(&pkt);
		}


	}

	//写入结尾 包含文件偏移索引
	mux.WriteEnd();
	demux.set_c(nullptr);
	mux.set_c(nullptr);
	encode.set_c(nullptr);
	cout << "输出文件" << out_file << ":" << endl;
	cout << "视频帧:" << video_count << endl;
	cout << "音频帧:" << audio_count << endl;
	cout << "总时长:" << total_sec << endl;

	return 0;
}