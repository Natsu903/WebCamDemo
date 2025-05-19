#pragma once
#include <thread>
#include <mutex>
#include <iostream>

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
#define LOGINFO(s) PRINTLOG(s,	LOG_TYPE_INFO)
#define LOGERROR(s) PRINTLOG(s,LOG_TYPE_ERROR)
#define LOGDEBUG(s) PRINTLOG(s,LOG_TYPE_DEBUG)


class Demuxthread
{
public:
	//�����߳�
	virtual void Start();

	//�����߳�
	virtual void Stop();

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
};

class Tools
{
};

