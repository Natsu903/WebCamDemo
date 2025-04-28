#include "encode.h"
#include <iostream>
#include <thread>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avutil.lib")

static void PrintErr(int err)
{
	char buf[1024] = { 0 };
	av_strerror(err, buf, sizeof(buf) - 1);
	std::cerr << buf << std::endl;
}


AVCodecContext* Encode::Create(int codec_id)
{
    //查找编码器
    auto codec =avcodec_find_encoder((AVCodecID)codec_id);
    if (!codec)
    {
        std::cerr << "avcodec_find_encoder error" << std::endl;
    }
    //创建编码器上下文
    auto c =avcodec_alloc_context3(codec);
    if (!c)
    {
        std::cerr << "avcodec_alloc_context3 failed" << std::endl;
    }
    c->time_base = { 1,25 };
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    c->thread_count = std::thread::hardware_concurrency();
    return c;
}

void Encode::set_c(AVCodecContext* c)
{
    std::unique_lock<std::mutex>lock(mux_);
    if (c_)
    {
        avcodec_free_context(&c_);
    }
    this->c_ = c;
}

bool Encode::SetOpt(const char* key, const char* val)
{
    std::unique_lock<std::mutex>lock(mux_);
    if (!c_)return false;
    auto re = av_opt_set(c_->priv_data, key, val, 0);
    if (re != 0)
    {
        std::cerr << "SetOpt by char val failed" << std::endl;
        PrintErr(re);
        return false;
    }
    return true;
}

bool Encode::SetOpt(const char* key, int val)
{
	std::unique_lock<std::mutex>lock(mux_);
	if (!c_)return false;
	auto re = av_opt_set_int(c_->priv_data, key, val, 0);
	if (re != 0)
	{
		std::cerr << "SetOpt by int val failed" << std::endl;
		PrintErr(re);
        return false;
	}
	return true;
}

bool Encode::Open()
{
    std::unique_lock<std::mutex>lock(mux_);
    if (!c_) return false;
    auto re = avcodec_open2(c_,NULL,NULL);
    if (re != 0)
    {
        PrintErr(re);
        return false;
    }
    return true;
}

AVFrame* Encode::CreateFrame()
{
    std::unique_lock<std::mutex>lock(mux_);
    if (!c_)return nullptr;
    auto frame = av_frame_alloc();
    frame->width = c_->width;
    frame->height = c_->height;
    frame->format = c_->pix_fmt;
    auto re = av_frame_get_buffer(frame, 0);
    if (re != 0)
    {
        PrintErr(re);
        return nullptr;
    }
    return frame;
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
    //接收成功则返回pkt
    if (re == 0)
    {
        return pkt;
    }
    //接收失败，执行错误处理
    av_packet_free(&pkt);
    //如果是处理结束返回空
	if (re == AVERROR(EAGAIN) || re == AVERROR_EOF)
	{
		return nullptr;
	}
    //打印错误代码
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
