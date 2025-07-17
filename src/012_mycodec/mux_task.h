#pragma once
#include "tools.h"
#include "mux.h"
class WEBCAM_API MuxTask :public BaseThread
{
public:
	void Main() override;
	
	/**
	 * �򿪷�װ�ļ�.
	 * 
	 * \param url	�����ַ
	 * \param video_para	��Ƶ����
	 * \param video_time_base	��Ƶʱ�����
	 * \param audio_para	��Ƶ����
	 * \param audio_time_base	��Ƶʱ�����
	 * \return 
	 */
	bool Open(const char* url,
		AVCodecParameters* video_para = nullptr,
		AVRational* video_time_base = nullptr,
		AVCodecParameters* audio_para = nullptr,
		AVRational* audio_time_base = nullptr
	);

	//��������
	void Do(AVPacket* pkt);

private:
	Mux xmux_;
	SafetyAVPacketList pkts_;
	std::mutex mux_;
};

