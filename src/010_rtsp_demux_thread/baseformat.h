#pragma once
#include <mutex>
#include "video_view.h"
#include "tools.h"

struct AVFormatContext;
struct AVCodecParameters;
struct AVCodecContext;
struct AVPacket;
struct XRational
{
	int num;//����
	int den;//��ĸ
};

class BaseFormat
{
public:
	/**
	 * ���������Ĳ������ϴ����õ�ֵ���������NULL���൱�ڹر������ģ��̰߳�ȫ.
	 *
	 * \param c
	 */
	void set_c(AVFormatContext* c);

	/**
	 * ���Ʋ������̰߳�ȫ.
	 * 
	 * \param stream_index ��Ӧc_->streams�±�
	 * \param dst �������
	 * \return �Ƿ񷵻سɹ�
	 */
	bool CopyPara(int stream_index, AVCodecParameters* dst);
	bool CopyPara(int stream_index, AVCodecContext* dst);

	//��������ָ�룬������Ƶ����
	std::shared_ptr<BasePara> CopyVideoPara();

	//����timebase����ʱ��
	bool RescaleTime(AVPacket* pkt, long long offset_pts, XRational time_base);

	//���ر�����Ա
	int video_index() { return video_index_; }
	int audio_index() { return audio_index_; }

	XRational video_time_base() { return video_time_base_; }
	XRational audio_time_base() { return audio_time_base_; }

	int video_codec_id() { return video_codec_id_; }

	//�ж��Ƿ�ʱ
	bool IsTimeout()
	{
		if (NowMs() - last_time_ > timeout_ms_)
		{
			last_time_ = NowMs();
			is_connected_ = false;
			return true;
		}
		return false;
	}

	void set_timeout_ms(int ms);

	bool is_connected() { return is_connected_; }

protected:
	int timeout_ms_ = 0;		//��ʱʱ��
	long long last_time_ = 0;	//�ϴν��յ����ݵ�ʱ��
	bool is_connected_ = false;	//�Ƿ����ӳɹ�
	AVFormatContext* c_;		//��װ���װ������
	std::mutex mux_;			//������
	int video_index_ = 0;		//video��audio��stream�е�����
	int audio_index_ = 1;
	XRational video_time_base_ = { 1,25 };
	XRational audio_time_base_ = { 1,9000 };
	int video_codec_id_ = 0;	//������ID
};

