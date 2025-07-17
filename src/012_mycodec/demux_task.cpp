#include "demux_task.h"
#include <chrono>
extern "C"
{
#include <libavformat/avformat.h>
}

void DemuxTask::Main()
{
    AVPacket pkt;
	while (!is_exit_)
	{
		if (!demux_.Read(&pkt))
		{
			//读取失败
			std::cout << "-" << std::flush;
			if (!demux_.is_connected())
			{
				Open(url_, timeout_ms_);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		//播放速度控制
		std::cout << "." << std::flush;
		if (syn_type_ == SYN_VIDEO && pkt.stream_index == demux_.video_index())
		{
			auto dur = demux_.RescaleToMs(pkt.duration, pkt.stream_index);
			if (dur <= 0)
			{
				dur = 40;
			}
			MSleep(40);
		}
		Next(&pkt);
		av_packet_unref(&pkt);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

bool DemuxTask::Open(std::string url, int timeout_ms)
{
	LOGDEBUG("DemuxTask::Open begin");
	demux_.set_c(nullptr);//断开之前的连接
	this->url_ = url;
	this->timeout_ms_ = timeout_ms;
	auto c = demux_.Open(url.c_str());
	if (!c)return false;
	demux_.set_c(c);
	demux_.set_timeout_ms(timeout_ms);
	LOGDEBUG("XDemuxTask::Open end!");
    return true;
}
