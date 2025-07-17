#include "audioplay.h"
#include "demux_task.h"
#include "decode_task.h"
#include <iostream>

int main(int argc, char *argv[])
{
	//解封装
	DemuxTask demux;
	if (!demux.Open("疾速追杀.mp4"))
	{
		std::cerr << "demux.Open(疾速追杀.mp4) failed" << std::endl;
		return -1;
	}
	auto ap = demux.CopyAudioPara();
	if (!ap)
	{
		std::cerr << "demux.CopyAudioPara() failed" << std::endl;
		return -1;
	}
	//解码
	DecodeTask decode;
	if (!decode.Open(ap->para))
	{
		std::cerr << "decode.Open(ap->para)" << std::endl;
		return -1;
	}
	decode.set_stream_index(demux.audio_index());
	demux.set_next(&decode);
	demux.Start();
	decode.Start();
	//播放
	auto audio = AudioPlay::Instance();
	if (!audio->Open(ap->para))
	{
		std::cerr << "audio->Open(spec) failed" << std::endl;
		return -1;
	}
	decode.set_frame_cache(true);
	for (;;)
	{
		auto f = decode.GetFrame();
		if (!f)
		{
			MSleep(10);
			continue;
		}
		audio->Push(f);
		FreeFrame(&f);
	}
	getchar();
	return 0;
}
