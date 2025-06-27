#pragma once
#include <thread>
#include <mutex>
#include <iostream>
#include <list>

//��־
enum LogLevel
{
	LOG_TYPE_DEBUG,
	LOG_TYPE_INFO,
	LOG_TYPE_ERROR,
	LOG_TYPE_FATAL
};

#define LOG_MIN_LEVEL LOG_TYPE_DEBUG
#define PRINTLOG(s,level)\
if(level>=LOG_MIN_LEVEL)\
std::cout<<level<<":"<<__FILE__<<":"<<__LINE__<<":\n"\
<<s<<std::endl;

#define LOGDEBUG(s) PRINTLOG(s,LOG_TYPE_DEBUG)
#define LOGINFO(s) PRINTLOG(s,LOG_TYPE_INFO)
#define LOGERROR(s) PRINTLOG(s,LOG_TYPE_ERROR)
#define LOGDEBUG(s) PRINTLOG(s,LOG_TYPE_DEBUG)

struct AVPacket;
struct AVCodecParameters;
struct AVRational;
struct AVFrame;

void FreeFrame(AVFrame** frame);

class BaseThread
{
public:
	//�����߳�
	virtual void Start();

	//�����߳�
	virtual void Stop();

	//ִ��������Ҫ����
	virtual void Do(AVPacket* pkt){}

	//���ݵ��¸�����������
	virtual void Next(AVPacket* pkt)
	{
		std::unique_lock<std::mutex> lock(m_);
		if (next_)
		{
			next_->Do(pkt);
		}
	}

	//������������һ���ڵ�
	void set_next(BaseThread* bt)
	{
		std::unique_lock<std::mutex> lock(m_);
		next_ = bt;
	}

protected:
	//�߳���ں���
	virtual void Main()=0;

	//��־�߳��˳�
	bool is_exit_ = false;

	//�߳�������
	int index_ = 0;

private:
	std::thread th_;
	std::mutex m_;
	BaseThread* next_ = nullptr;//��������һ���ڵ�
};

class Tools
{
};

//����Ƶ����
class BasePara
{
public:
	AVCodecParameters* para = nullptr;//����Ƶ����
	AVRational* time_base = nullptr;//ʱ�����

	//��������
	static BasePara* Create();
	~BasePara();
private:
	//��ֹ����ջ�ж���
	BasePara();
};

/**
 * �̰߳�ȫavpacket list.
 * �������յ������Ƚ��ɵ��б��У��߳��ٴ��б���ȡ�ô���
 */
class SafetyAVPacketList
{
public:
	AVPacket* Pop();
	void Push(AVPacket* pkt);

private:
	std::list<AVPacket*> pkts_;
	int max_packets_ = 100;//����б���������������
	std::mutex mux_;
};
