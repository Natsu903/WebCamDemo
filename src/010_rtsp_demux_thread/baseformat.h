#pragma once
#include <mutex>

struct AVFormatContext;
struct AVCodecParameters;
struct AVCodecContext;
struct AVPacket;
struct XRational
{
	int num;//分子
	int den;//分母
};

class BaseFormat
{
public:
	/**
	 * 设置上下文并清理上次设置的值，如果传递NULL，相当于关闭上下文，线程安全.
	 *
	 * \param c
	 */
	void set_c(AVFormatContext* c);

	/**
	 * 复制参数，线程安全.
	 * 
	 * \param stream_index 对应c_->streams下标
	 * \param dst 输出参数
	 * \return 是否返回成功
	 */
	bool CopyPara(int stream_index, AVCodecParameters* dst);
	bool CopyPara(int stream_index, AVCodecContext* dst);

	//根据timebase换算时间
	bool RescaleTime(AVPacket* pkt, long long offset_pts, XRational time_base);

	//返回保护成员
	int video_index() { return video_index_; }
	int audio_index() { return audio_index_; }

	XRational video_time_base() { return video_time_base_; }
	XRational audio_time_base() { return audio_time_base_; }

	int video_codec_id() { return video_codec_id_; }

protected:
	AVFormatContext* c_;//封装解封装上下文
	std::mutex mux_;//互斥量
	int video_index_ = 0;//video和audio在stream中的索引
	int audio_index_ = 1;
	XRational video_time_base_ = { 1,25 };
	XRational audio_time_base_ = { 1,9000 };
	int video_codec_id_ = 0;//编码器ID
};

