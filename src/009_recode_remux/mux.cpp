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
	//创建上下文
	AVFormatContext* c = nullptr;
	auto re = avformat_alloc_output_context2(&c, NULL, NULL, url);
	BERR(re);
	//添加音视频流
	auto vs = avformat_new_stream(c, NULL);
	vs->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
	auto as = avformat_new_stream(c, NULL);
	as->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;

	//打开IO
	re = avio_open(&c->pb, url, AVIO_FLAG_WRITE);
	BERR(re);
	return c;
}

bool Mux::WriteHead()
{
	std::unique_lock<std::mutex>lock(mux_);
	if (!c_)return false;
	//会改变timebase
	auto re = avformat_write_header(c_, nullptr);
	BERR(re);

	//打印输出上下文
	av_dump_format(c_, 0, c_->url, 1);
	return true;
}

bool Mux::Write(AVPacket* pkt)
{
	std::unique_lock<std::mutex>lock(mux_);
	if (!c_)return false;
	//写入一帧数据，内部缓冲排序dts，通过pkt=null 可以写入缓冲
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
