#include <iostream>
#include "myonvif.h"

int main(int argc, char* argv[])
{
	MyOnvif myonvif;
	//auto count = myonvif.ONVIF_DetectDevice(nullptr);
	//myonvif.PrintDetectedAddr();
	//char media_url[1024] = {0};
 //   myonvif.GetMediaUrl("http://192.168.31.235/onvif/device_service", media_url, "admin", "GZH&password");
	//std::cout << "media_url:"<<media_url << std::endl;

	//char main_token[1024] = {0};
	//char sub_token[1024] = {0};
	//myonvif.GetProfile(media_url, main_token,sub_token,"admin", "GZH&password");
	//std::cout << "main_token:" << main_token << std::endl;
	//std::cout << "sub_token:" << sub_token << std::endl;

	//获取rtsp地址
	char main_rtsp[1024] = {0};
	char sub_rtsp[1024] = {0};
	//myonvif.GetStreamUrl(media_url, main_token, main_rtsp, "admin", "GZH&password");
	//myonvif.GetStreamUrl(media_url, sub_token, main_rtsp, "admin", "GZH&password");
	myonvif.GetOnvifRtsp("http://192.168.31.235/onvif/device_service", main_rtsp, sub_rtsp, "admin", "GZH&password");
	getchar();
	return 0;
}
