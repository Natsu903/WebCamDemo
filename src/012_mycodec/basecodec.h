#pragma once
#include <mutex>
#include <vector>
#include "tools.h"

/**
 * ����ͽ���Ļ���
 */
class WEBCAM_API Codec
{
public:

	/**
	 * ��ʼ��������������.
	 *
	 * \param codec_id ���������id��AV_CODEC_ID_H264
	 * \return �������úõ�AVCodecContextָ��
	 */
	static AVCodecContext* Create(int codec_id, bool is_encode);

	/**
	 * �������������Ĵ洢������.
	 *
	 * \param c
	 */
	void set_c(AVCodecContext* c);

	/**
	 * ����ffmpeg������ѡ�����.
	 *
	 * \param key ��������
	 * \param val ����
	 * \return �ɹ�����trueʧ�ܷ���false
	 */
	bool SetOpt(const char* key, const char* val);
	bool SetOpt(const char* key, int val);

	/**
	 * ��ʼ��������������Ĳ������ı����������.
	 *
	 * \return �ɹ�����true
	 */
	bool Open();

	AVFrame* CreateFrame();

protected:
	AVCodecContext* c_ = nullptr;
	std::mutex mux_;
};

