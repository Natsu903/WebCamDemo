#include <iostream>
#include <thread>
#include <chrono>
#include "tools.h"
#include "demux_task.h"
#include "decode_task.h"
#include "video_view.h"

#define CAM1 "rtsp://admin:GZH&password@192.168.31.235/cam/realmonitor?channel=1&subtype=0"

int main(int argc, char* argv[])
{
	DemuxTask demux_task;
	for (;;)
	{
		if(demux_task.Open(CAM1))break;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		continue;
	}
	auto para = demux_task.CopyVideoPara();

	//初始化渲染
	auto view = Video_View::Create();
	view->Init(para->para);

	DecodeTask decode_task;
	if (!decode_task.Open(para->para))
	{
		LOGERROR("decode_task.Open(para->para) error")
	}
	else
	{
		//设定下一个责任链
		demux_task.set_next(&decode_task);
		demux_task.Start();
		decode_task.Start();
	}

	for (;;)
	{
		auto f = decode_task.GetFrame();
		if (!f)
		{
			MSleep(1);
			continue;
		}
		view->DrawFrame(f);
		FreeFrame(&f);
	}

	getchar();
	return 0;
}
