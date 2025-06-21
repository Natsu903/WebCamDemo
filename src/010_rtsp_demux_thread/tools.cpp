#include "tools.h"
#include "sstream"
extern "C"
{
#include <libavcodec/avcodec.h>
}

void BaseThread::Start()
{
	std::unique_lock<std::mutex>lock(m_);
	static int i = 0;
	i++;
	index_ = i;
	is_exit_ = false;
	//启动线程
	th_ = std::thread(&BaseThread::Main, this);
	std::stringstream ss;
	ss << "Demuxthread::Start()" << index_;
	LOGINFO(ss.str());
}

void BaseThread::Stop()
{
	std::stringstream ss;
	ss << "Demuxthread::Stop() begin" << index_;
	LOGINFO(ss.str());
	is_exit_ = true;
	if (th_.joinable())
	{
		th_.join();
	}
	ss.str("");
	ss << "Demuxthread::Stop() end" << index_;
	LOGINFO(ss.str());

}

BasePara* BasePara::Create()
{
	return new BasePara();
}

BasePara::~BasePara()
{
	if (para)avcodec_parameters_free(&para);
	if (time_base)
	{
		delete time_base;
		time_base = nullptr;
	}
}

BasePara::BasePara()
{
	para = avcodec_parameters_alloc();
	time_base = new AVRational();
}

AVPacket* SafetyAVPacketList::Pop()
{
	std::unique_lock<std::mutex>lock(mux_);
	if (pkts_.empty())return nullptr;
	auto pkt = pkts_.front();
	pkts_.pop_front();
	return pkt;
}

void SafetyAVPacketList::Push(AVPacket* pkt)
{
	std::unique_lock<std::mutex>lock(mux_);
	//生成新的AVPacket对象，引用技术+1
	auto p = av_packet_alloc();
	av_packet_ref(p, pkt);//引用计数，减少数据复制，线程安全
	pkts_.push_back(p);
	//超出最大空间，清理数据到关键帧位置
	if (pkts_.size() > max_packets_)
	{
		//处理第一帧
		if (pkts_.front()->flags & AV_PKT_FLAG_KEY)//关键帧
		{
			av_packet_free(&pkts_.front());//清理
			pkts_.pop_front();//出栈
			return;
		}
		//清理所有非关键帧数据
		while (!pkts_.empty())
		{
			if (pkts_.front()->flags & AV_PKT_FLAG_KEY)//关键帧
			{
				return;
			}
			av_packet_free(&pkts_.front());//清理
			pkts_.pop_front();//出栈
		}
	}
}

void FreeFrame(AVFrame** frame)
{
	if (!frame||!(*frame))return;
	av_frame_free(frame);
}
