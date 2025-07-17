#pragma once
#include "basecodec.h"

struct AVPacket;
struct AVFrame;
class WEBCAM_API Decode :public Codec
{
public:
	/**
	 * ���ͽ�������.
	 * 
	 * \param pkt Ҫ���͵Ľ�������
	 * \return �Ƿ��ͳɹ�
	 */
	bool Send(const AVPacket* pkt);

	/**
	 * ���ս�������.
	 * 
	 * \param frame ���պ�Ҫ�洢����frame
	 * \param is_hw_copy �Ƿ񿽱��Դ����ݵ��ڴ�
	 * \return �Ƿ���ճɹ�
	 */
	bool Recv(AVFrame* frame, bool is_hw_copy=true);

	/**
	 * ��ȡ����.
	 * 
	 * \return �洢Ϊvector�Ļ�������
	 */
	std::vector<AVFrame*>End();

	/**
	 * ��ʼ��Ӳ������.
	 * 
	 * \param type Ӳ����������Ĭ��Ϊ4�� AV_HWDEVICE_TYPE_DXVA2
	 * \return �Ƿ��ʼ���ɹ�
	 */
	bool InitHW(int type=4);
};

