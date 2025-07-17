#include "encode.h"
#include <thread>
#include <iostream>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}


AVPacket* Encode::DoEncode(const AVFrame* frame)
{
    if (!frame)return nullptr;
    std::unique_lock<std::mutex>lock(mux_);
    if (!c_) return nullptr;
    auto re = avcodec_send_frame(c_,frame);
    if (re != 0)return nullptr;
    auto pkt = av_packet_alloc();
    re = avcodec_receive_packet(c_, pkt);
    //���ճɹ��򷵻�pkt
    if (re == 0)
    {
        return pkt;
    }
    //����ʧ�ܣ�ִ�д�����
    av_packet_free(&pkt);
    //����Ǵ���������ؿ�
	if (re == AVERROR(EAGAIN) || re == AVERROR_EOF)
	{
		return nullptr;
	}
    //��ӡ�������
	if (re < 0)
	{
		PrintErr(re);
	}
	return nullptr;
}

std::vector<AVPacket*> Encode::End()
{
    std::vector<AVPacket*>res;
    std::unique_lock<std::mutex>lock(mux_);
    if (!c_)return res;
    auto re = avcodec_send_frame(c_, NULL);
    while (re >= 0)
    {
        auto pkt = av_packet_alloc();
        re = avcodec_receive_packet(c_, pkt);
        if (re != 0)
        {
            av_packet_free(&pkt);
            break;
        }
        res.push_back(pkt);
    }
    return res;
}
