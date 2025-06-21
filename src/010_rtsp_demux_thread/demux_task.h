#pragma once
#include "tools.h"
#include "demux.h"
class DemuxTask :public BaseThread
{
public:
	void Main();

	/**
	 * �򿪽��װ.
	 * 
	 * \param url rtsp��ַ
	 * \param timeout_ms ��ʱʱ��
	 * \return 
	 */
	bool Open(std::string url,int timeout_ms = 1000);

	//��������ָ�룬������Ƶ����
	std::shared_ptr<BasePara> CopyVideoPara()
	{
		return demux_.CopyVideoPara();
	}
private:
	Demux demux_;
	std::string url_;
	int timeout_ms_ = 0; //��ʱʱ��
};

