#include <iostream>
#include <fstream>
#include <string>
#include "video_view.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"swscale.lib")

#ifdef av_err2str
#undef av_err2str
#endif
av_always_inline char* av_err2str(int errnum) {
	thread_local char str[AV_ERROR_MAX_STRING_SIZE];
	memset(str, 0, sizeof(str));
	return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}

int main(int argc,char* argv[])
{
	auto view = Video_View::Create();
	std::string filename = "800x600.h264";
	std::ifstream ifs(filename, std::ios::binary);
	if (!ifs)
	{
		std::cerr << "读取文件失败" << std::endl;
		return -1;
	}
	unsigned char inbuf[4096] = { 0 };
	AVCodecID codec_id = AV_CODEC_ID_H264;

	auto codec = avcodec_find_decoder(codec_id);
    if (!codec) {
        fprintf(stderr, "Unsupported codec\n");
        return -1;
    }

	////打印所有支持的硬件加速方式
	//for (int i = 0; ; i++)
	//{
	//	auto config = avcodec_get_hw_config(codec, i);
	//	if (!config)break;
	//	if (config->device_type)
	//	{
	//		std::cout << av_hwdevice_get_type_name(config->device_type)<<std::endl;
	//	}
	//}

	auto c = avcodec_alloc_context3(codec);
	if (!c) {
		fprintf(stderr, "Could not allocate video codec context\n");
		return -1;
	}

	//硬件加速格式DXVA2
	//auto hw_type = AV_HWDEVICE_TYPE_DXVA2;

	////初始化硬件加速上下文
	//AVBufferRef* hw_ctx = nullptr;
	//av_hwdevice_ctx_create(&hw_ctx, hw_type, NULL, NULL, 0);

	//设定硬件gpu加速
	//c->hw_device_ctx = av_buffer_ref(hw_ctx);
	c->thread_count = std::thread::hardware_concurrency();

	if (avcodec_open2(c, NULL, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		return -1;
	}


	//缩放参数
	int targetWidth = 400;
	int targetHeight = 300;

	auto parser = av_parser_init(codec_id);
	auto packet = av_packet_alloc();
	auto srcframe = av_frame_alloc();//原始帧
	auto scaled_frame = av_frame_alloc();//缩放帧
	auto hw_frame = av_frame_alloc();//硬解码转换用
	SwsContext* sws_ctx = nullptr;//初始化缩放上下文


	auto begin = NowMs();
	int count = 0;//解码统计
	bool is_init_win = false;

	while (!ifs.eof())
	{
		ifs.read((char*)inbuf, sizeof(inbuf));
		int data_size = ifs.gcount();
		if (data_size <= 0)break;
		//if (ifs.eof())
		//{
		//	ifs.clear();
		//	ifs.seekg(0, std::ios::beg);
		//}
		auto data = inbuf;
		while (data_size > 0) 
		{
			/**
			 *	parser：解析器上下文，用于管理解析过程。
				c：解码器上下文，用于提供解码所需的信息。
				&pkt->data：指向 AVPacket 数据缓冲区的指针，用于存储解析后的数据。
				&pkt->size：指向 AVPacket 数据大小的指针，用于存储解析后的数据大小。
				data：输入的数据缓冲区。
				data_size：输入的数据大小。
				AV_NOPTS_VALUE：表示没有时间戳信息。
				AV_NOPTS_VALUE：表示没有解码时间戳信息。
				0：表示数据在流中的位置。
			 */
			int ret = av_parser_parse2(parser, c, 
				&packet->data, &packet->size, 
				data, data_size, AV_NOPTS_VALUE,
				AV_NOPTS_VALUE, 0);
			data += ret;
			data_size -= ret;
			if (packet->size)
			{
				ret = avcodec_send_packet(c, packet);
				if (ret < 0) {
					std::cerr << "Error sending packet for decoding: " << av_err2str(ret) << std::endl;
					break;
				}
				while (ret>=0)
				{
					ret = avcodec_receive_frame(c, srcframe);
					if (ret == AVERROR(EAGAIN)) {
						break;
					}
					else if (ret < 0) {
						std::cerr << "Error receiving frame from decoder: " << av_err2str(ret) << std::endl;
						break;
					}
					
					//兼容硬件解码
					auto pframe = srcframe;

					if (c->hw_device_ctx)//硬解码
					{
						av_hwframe_transfer_data(hw_frame, srcframe, 0);
						pframe = hw_frame;
					}

					//缩放上下文
					if (targetWidth && targetHeight > 0 && pframe->width && pframe->height)
					{
						sws_ctx = sws_getContext(pframe->width, pframe->height, (AVPixelFormat)pframe->format, targetWidth, targetHeight, AV_PIX_FMT_YUV420P, SWS_BILINEAR, nullptr, nullptr, nullptr);
						if (!sws_ctx)
						{
							fprintf(stderr, "Failed to create scaling context\n");
							return -1;
						}
					}
					
					if (sws_ctx)
					{
						av_frame_get_buffer(scaled_frame, 32);
						uint8_t* dstData[4] = { scaled_frame->data[0], scaled_frame->data[1], scaled_frame->data[2], nullptr };
						int dstLinesize[4] = { scaled_frame->linesize[0], scaled_frame->linesize[1], scaled_frame->linesize[2], 0 };
						sws_scale(sws_ctx,pframe->data,pframe->linesize,0,pframe->height, dstData, dstLinesize);
					}

					std::cout << pframe->format << " " << std::flush;
					//第一帧初始化窗口
					if (!is_init_win)
					{
						is_init_win = true;
						view->Init(targetWidth, targetHeight, (Video_View::Format)pframe->format);
					}
					view->DrawFrame(pframe);

					count++;
					auto cur = NowMs();
					if (cur - begin >= 100)
					{
						std::cout << "\nfps" << count * 10 << std::endl;
						count = 0;
						begin = cur;
					}
				}
			}
			if (packet->data) {
				av_packet_unref(packet);
			}
		}
	}

	// 发送 NULL 包以刷新解码器
	int ret = avcodec_send_packet(c, NULL);
	while (ret >= 0)
	{
		ret = avcodec_receive_frame(c, srcframe);
		if (ret == AVERROR(EAGAIN)) {
			continue;
		}
		else if (ret < 0) {
			std::cerr << "Error receiving frame from decoder after sending NULL packet: " << av_err2str(ret) << std::endl;
			break;
		}
	}

	av_parser_close(parser);
	avcodec_free_context(&c);
	av_packet_free(&packet);
	av_frame_free(&srcframe);
	av_frame_free(&hw_frame);
	//av_buffer_unref(&hw_ctx);
	return 0;
}