#include "tools.h"
#include "sstream"

void Demuxthread::Start()
{
	std::unique_lock<std::mutex>lock(m_);
	static int i = 0;
	i++;
	index_ = i;
	is_exit_ = false;
	//Æô¶¯Ïß³Ì
	th_ = std::thread(&Demuxthread::Main, this);
	std::stringstream ss;
	ss << "Demuxthread::Start()" << index_;
	LOGINFO(ss.str());
}

void Demuxthread::Stop()
{
	std::stringstream ss;
	ss << "Demuxthread::Stop() begin" << index_;
	LOGINFO(ss.str());
	is_exit_ = true;
	if (th_.joinable())
	{
		th_.join();
	}
	ss.str("");
	ss << "Demuxthread::Stop() end" << index_;
	LOGINFO(ss.str());

}

