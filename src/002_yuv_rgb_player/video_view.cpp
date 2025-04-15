#include "video_view.h"
#include "private_sdl.h"
#include <thread>
#include <iostream>
#include <chrono>
using namespace std;

extern "C"
{
#include <libavcodec/avcodec.h>
}
#pragma comment(lib,"avutil.lib")

//高精度定时休眠
void MSleep(unsigned int ms)
{
	//高精度计时器初始化
	//使用std::chrono::high_resolution_clock获取当前时间点beg，作为计时起点。
	//该时钟提供系统支持的最高精度（通常是纳秒或微秒级）
	auto beg = std::chrono::high_resolution_clock::now();
	while (true) {
		//在无限循环中，每次迭代通过now记录当前时间点，用于计算已流逝的时间
		auto now = std::chrono::high_resolution_clock::now();
		//​计算时间差
		//now - beg：得到时间间隔（duration类型），单位为时钟周期。
		//duration_cast<std::chrono::milliseconds>：将时间间隔转换为毫秒级整数值
		//.count()：提取毫秒数的整数值
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - beg).count();
		//若累计时间elapsed达到或超过目标延迟ms，则退出循环
		if (elapsed >= ms) break;
		//每次循环休眠1毫秒
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

long long NowMs()
{
	return clock()/(CLOCKS_PER_SEC/1000);
}

Video_View* Video_View::Create(RenderType type)
{
	switch (type)
	{
	case Video_View::SDL:
		return new PrivateSDL();
		break;
	default:
		break;
	}
	return nullptr;
}

bool Video_View::DrawFrame(AVFrame* frame)
{
	if (!frame || !frame->data[0])return false;
	count_++;
	//记录当前FPS计算的时间起点。若首次调用（beg_ms_ <= 0），初始化为当前系统时间
	if (beg_ms_ <= 0)
	{
		beg_ms_ = clock();
	}
	//当累计时间超过1000ms（1秒）时,记录过去1秒内的平均帧率（count_值),
	//重置count_为0，更新beg_ms_为当前时间，开始下一秒的帧数统计
	else if ((clock()-beg_ms_)/(CLOCKS_PER_SEC/1000)>=1000)
	{
		render_fps_ = count_;
		count_ = 0;
		beg_ms_ = clock();
	}
	switch (frame->format)
	{
	case AV_PIX_FMT_YUV420P:
		return Draw(
			frame->data[0], frame->linesize[0],
			frame->data[1], frame->linesize[1],
			frame->data[2], frame->linesize[2]);
	case AV_PIX_FMT_BGRA:
	case AV_PIX_FMT_ARGB:
	case AV_PIX_FMT_RGBA:
		return Draw(frame->data[0], frame->linesize[0]);
	default:
		break;
	}
	return true;
}

bool Video_View::Open(std::string filepath)
{
	if (ifs_.is_open())
	{
		ifs_.close();
	}
	ifs_.open(filepath, ios::binary);
	return ifs_.is_open();
}

AVFrame* Video_View::Read()
{
	if (width_ <= 0 || height_ <= 0 || !ifs_) return NULL;
	if (frame_)
	{
		if (frame_->width != width_ || frame_->height != height_ || frame_->format != format_);
		{
			av_frame_free(&frame_);
		}
	}
	if (!frame_)
	{
		frame_ = av_frame_alloc();
		frame_->width = width_;
		frame_->height = height_;
		frame_->format = format_;
		frame_->linesize[0] = width_ * 4;
		if (frame_->format == AV_PIX_FMT_YUV420P)
		{
			frame_->linesize[0] = width_;
			frame_->linesize[1] = width_ / 2;
			frame_->linesize[2] = width_ / 2;
		}
		auto re = av_frame_get_buffer(frame_,0);
		if (re != 0)
			if (re != 0)
			{
				char buf[1024] = { 0 };
				av_strerror(re, buf, sizeof(buf) - 1);
				std::cout << buf << std::endl;
				av_frame_free(&frame_);
				return NULL;
			}
	}
	if (!frame_)return NULL;

	//yuv420p读取
	if (frame_->format == AV_PIX_FMT_YUV420P)
	{
		ifs_.read((char*)frame_->data[0],frame_->linesize[0]*height_);
		ifs_.read((char*)frame_->data[1],frame_->linesize[1]*height_/2);
		ifs_.read((char*)frame_->data[2],frame_->linesize[2]*height_/2);
	}
	else //RGBA ARGB BGRA 32
	{
		ifs_.read((char*)frame_->data[0], frame_->linesize[0] * height_);
	}
	//获取最后一次读取操作的实际字节数。若为 0，表示未读取到有效数据（如流结束），返回 NULL
	if (ifs_.gcount() == 0) return NULL;
	return frame_;
}
