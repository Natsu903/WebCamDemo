#pragma once
#include <mutex>
#include <vector>

struct AVCodecContext;
struct AVFrame;
struct AVPacket;

class Encode
{
public:
	/**
	 * ��ʼ��������������.
	 * 
	 * \param codec_id ���������id��AV_CODEC_ID_H264
	 * \return �������úõ�AVCodecContextָ��
	 */
	static AVCodecContext* Create(int codec_id);

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

	/**
	 * ����֡����.
	 * 
	 * \param frame�����õ�frame
	 * \return ����ɹ��򷵻�pkt��������β����nullptr������ʧ�ܴ�ӡ������Ϣ
	 */
	AVPacket* DoEncode(const AVFrame* frame);

	std::vector<AVPacket*>End();

private:
	AVCodecContext* c_ = nullptr;
	std::mutex mux_;
};

