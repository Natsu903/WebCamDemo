#include "mux.h"
#include <iostream>
#include <thread>

extern "C"
{
#include <libavformat/avformat.h>
}

void PrintErr(int err);

#define BERR(err) if(err!= 0){PrintErr(err);return 0;}

AVFormatContext* Mux::Open(const char* url)
{
	//����������
	AVFormatContext* c = nullptr;
	auto re = avformat_alloc_output_context2(&c, NULL, NULL, url);
	BERR(re);
	//�������Ƶ��
	auto vs = avformat_new_stream(c, NULL);
	vs->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
	auto as = avformat_new_stream(c, NULL);
	as->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;

	//��IO
	re = avio_open(&c->pb, url, AVIO_FLAG_WRITE);
	BERR(re);
	return c;
}

bool Mux::WriteHead()
{
	std::unique_lock<std::mutex>lock(mux_);
	if (!c_)return false;
	//��ı�timebase
	auto re = avformat_write_header(c_, nullptr);
	BERR(re);

	//��ӡ���������
	av_dump_format(c_, 0, c_->url, 1);
	return true;
}

bool Mux::Write(AVPacket* pkt)
{
	std::unique_lock<std::mutex>lock(mux_);
	if (!c_)return false;
	//д��һ֡���ݣ��ڲ���������dts��ͨ��pkt=null ����д�뻺��
	auto re = av_interleaved_write_frame(c_, pkt);
	BERR(re);
	return true;
}

bool Mux::WriteEnd()
{
	std::unique_lock<std::mutex>lock(mux_);
	if (!c_)return false;
	int re = 0;
	re = av_write_trailer(c_);
	BERR(re);
	return true;
}
