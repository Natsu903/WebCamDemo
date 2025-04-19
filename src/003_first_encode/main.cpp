#include<iostream>
#include<fstream>
extern "C"
{
	#include<libavcodec/avcodec.h>
}
#undef main


#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avutil.lib")

int main(int argc,char* argv[])
{
	std::string filename = "400_300_25";
	AVCodecID codec_id = AV_CODEC_ID_H264;
	if (argc > 1)
	{
		std::string codec = argv[1];
		if (codec == "h265" || codec == "hevc")
		{
			codec_id = AV_CODEC_ID_HEVC;
		}
	}
	if (codec_id == AV_CODEC_ID_H264)
	{
		filename += ".h264";
	}
	if (codec_id == AV_CODEC_ID_HEVC)
	{
		filename += ".h265";
	}
	std::ofstream ofs;
	ofs.open(filename, std::ios::binary);

	auto codec = avcodec_find_encoder(codec_id);
	if (!codec)
	{
		std::cerr << "codec find failed" << std::endl;
		return -1;
	}
	auto c = avcodec_alloc_context3(codec);
	if (!c)
	{
		std::cerr << "avcodec_alloc_context3 failed" << std::endl;
		return -1;
	}
	c->width = 400;
	c->height = 300;
	c->time_base = { 1,25 };
	c->pix_fmt = AV_PIX_FMT_YUV420P;
	c->thread_count = 16;



	int re = avcodec_open2(c, codec, NULL);
	if (re != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		std::cerr << "avcodec_open2 failed!" << buf << std::endl;
		return -1;
	}
	std::cout << "avcodec_open2 success!" << std::endl;

	auto frame = av_frame_alloc();
	frame->width = c->width;
	frame->height = c->height;
	frame->format = c->pix_fmt;
	re = av_frame_get_buffer(frame, 0);
	if (re != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		std::cerr << "avcodec_open2 failed!" << buf << std::endl;
		return -1;
	}
	auto pkt = av_packet_alloc();
	for (int i = 0; i < 250; i++)
	{
		//Y
		for (int y = 0; y < c->height; y++)
		{
			for (int x = 0; x < c->width; x++)
			{
				frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
			}
		}
		//U,V
		for (int y = 0; y < c->height / 2; y++)
		{
			for (int x = 0; x < c->width / 2; x++)
			{
				frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
				frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
			}
		}
		frame->pts = i;

		re = avcodec_send_frame(c, frame);
		if (re != 0)
		{
			break;
		}
		while (re >= 0)
		{
			re = avcodec_receive_packet(c,pkt);
			if (re == AVERROR(EAGAIN) || re == AVERROR_EOF)
				break;
			if (re < 0)
			{
				char buf[1024] = { 0 };
				av_strerror(re, buf, sizeof(buf) - 1);
				std::cerr << "avcodec_receive_packet failed!" << buf << std::endl;
				break;
			}
			std::cout << pkt->size << " " << std::flush;
			ofs.write((char*)pkt->data, pkt->size);
			av_packet_unref(pkt);
		}
	}
	ofs.close();
	av_packet_free(&pkt);
	av_frame_free(&frame);
	avcodec_free_context(&c);
	return 0;
}