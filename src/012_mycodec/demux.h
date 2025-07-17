#pragma once
#include "baseformat.h"
class WEBCAM_API Demux :public BaseFormat
{
public:
	/**
	 * �򿪽��װ.
	 * 
	 * \param url ���װ��ַ
	 * \return ʧ�ܷ���nullptr
	 */
	static AVFormatContext* Open(const char* url);

	/**
	 * ��ȡһ֡����.
	 *
	 * \param pkt �������
	 * \return �Ƿ�ɹ�
	 */
	bool Read(AVPacket* pkt);

	bool Seek(long long pts, int stream_index);

};

