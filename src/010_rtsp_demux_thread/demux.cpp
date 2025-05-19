#include "demux.h"
#include <iostream>
#include <thread>

extern "C"
{
#include <libavformat/avformat.h>
}

void PrintErr(int err);

#define BERR(err) if(err!= 0){PrintErr(err);return 0;}

AVFormatContext* Demux::Open(const char* url)
{
    AVFormatContext* c = nullptr;
    auto re = avformat_open_input(&c, url, nullptr, nullptr);
    BERR(re);
    //获取媒体信息
    re = avformat_find_stream_info(c, nullptr);
    BERR(re);
    //打印封装信息
    av_dump_format(c,0,url,0);
    return c;
}

bool Demux::Read(AVPacket* pkt)
{
	std::unique_lock<std::mutex>lock(mux_);
	if (!c_)return false;
    auto re = av_read_frame(c_, pkt);
    BERR(re);
    return true;
}

bool Demux::Seek(long long pts, int stream_index)
{
	std::unique_lock<std::mutex>lock(mux_);
	if (!c_)return false;
    auto re = av_seek_frame(c_, stream_index, pts, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
    BERR(re);
    return true;
}
