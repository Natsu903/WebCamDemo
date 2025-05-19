#pragma once
#include <thread>
#include <mutex>
#include <iostream>

//日志
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
	//启动线程
	virtual void Start();

	//结束线程
	virtual void Stop();

protected:
	//线程入口函数
	virtual void Main()=0;

	//标志线程退出
	bool is_exit_ = false;

	//线程索引号
	int index_ = 0;

private:
	std::thread th_;
	std::mutex m_;
};

class Tools
{
};

