#pragma once
#include "baseformat.h"
class Mux:public BaseFormat
{
public:
	//´ò¿ª·â×°
	static AVFormatContext* Open(const char* url);

	bool WriteHead();

	bool Write(AVPacket* pkt);

	bool WriteEnd();
};

