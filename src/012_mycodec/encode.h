#pragma once
#include "basecodec.h"

class WEBCAM_API Encode:public Codec
{
public:
	
	/**
	 * ����֡����.
	 * 
	 * \param frame�����õ�frame
	 * \return ����ɹ��򷵻�pkt��������β����nullptr������ʧ�ܴ�ӡ������Ϣ
	 */
	AVPacket* DoEncode(const AVFrame* frame);

	std::vector<AVPacket*>End();

};

