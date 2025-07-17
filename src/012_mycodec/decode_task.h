#pragma once
#include "tools.h"
#include "decode.h"
class WEBCAM_API DecodeTask :public BaseThread
{
public:
	//线程主函数
	void Main() override;
	//责任链处理函数
	void Do(AVPacket* pkt) override;
	//打开解码器
	bool Open(AVCodecParameters* para);


	/**	
	* 返回当前需要渲染的Frame, 如果没有返回null
	* need_view_控制渲染
	* 线程安全.
	* 返回结果需要用FreeFrame释放
	*/
	AVFrame* GetFrame();

	void set_stream_index(int i) { stream_index_ = i; }

	void set_frame_cache(bool is) { frame_cache_ = is; }

	bool is_open() { return is_open_; }

	//设置同步时间
	void set_syn_pts(long long p) { syn_pts_ = p; }
	void set_block_size(int s) { block_size_ = s; }
private:
	int block_size_ = 0;//阻塞大小
	long long syn_pts_ = -1;//同步时间 -1不同步
	bool is_open_ = false;
	int stream_index_ = 0;
	std::mutex mux_;
	Decode decode_;
	SafetyAVPacketList pkt_list_;
	AVFrame* frame_ = nullptr;//解码后存储
	bool need_view_ = false;//是否需要渲染，每帧只渲染一次，通过getframe获取
	std::list<AVFrame*>frames_;//存储音频缓冲
	bool frame_cache_=false;//是否缓冲frame队列
};

