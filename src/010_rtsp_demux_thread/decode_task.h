#pragma once
#include "tools.h"
#include "decode.h"
class DecodeTask :public BaseThread
{
public:
	//�߳�������
	void Main() override;
	//������������
	void Do(AVPacket* pkt) override;
	//�򿪽�����
	bool Open(AVCodecParameters* para);


	/**	
	* ���ص�ǰ��Ҫ��Ⱦ��Frame, ���û�з���null
	* need_view_������Ⱦ
	* �̰߳�ȫ.
	* ���ؽ����Ҫ��FreeFrame�ͷ�
	*/
	AVFrame* GetFrame();

private:
	std::mutex mux_;
	Decode decode_;
	SafetyAVPacketList pkt_list_;
	AVFrame* frame_ = nullptr;//�����洢
	bool need_view_ = false;//�Ƿ���Ҫ��Ⱦ��ÿֻ֡��Ⱦһ�Σ�ͨ��getframe��ȡ
};

