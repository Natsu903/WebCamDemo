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
	//�����߳�
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
	//�����µ�AVPacket�������ü���+1
	auto p = av_packet_alloc();
	av_packet_ref(p, pkt);//���ü������������ݸ��ƣ��̰߳�ȫ
	pkts_.push_back(p);
	//�������ռ䣬�������ݵ��ؼ�֡λ��
	if (pkts_.size() > max_packets_)
	{
		//�����һ֡
		if (pkts_.front()->flags & AV_PKT_FLAG_KEY)//�ؼ�֡
		{
			av_packet_free(&pkts_.front());//����
			pkts_.pop_front();//��ջ
			return;
		}
		//�������зǹؼ�֡����
		while (!pkts_.empty())
		{
			if (pkts_.front()->flags & AV_PKT_FLAG_KEY)//�ؼ�֡
			{
				return;
			}
			av_packet_free(&pkts_.front());//����
			pkts_.pop_front();//��ջ
		}
	}
}

void FreeFrame(AVFrame** frame)
{
	if (!frame||!(*frame))return;
	av_frame_free(frame);
}
