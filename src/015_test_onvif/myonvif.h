#pragma once
struct wsdd__ProbeType;
struct __wsdd__ProbeMatches;	// 用于接收Probe应答

class MyOnvif
{
private:
	//打印错误
	void soap_perror(struct soap* soap, const char* str);

	//申请空间
	void* ONVIF_soap_malloc(struct soap* soap, unsigned int n);

	//初始化消息头
	void ONVIF_init_header(struct soap* soap);

	//初始化探测设备的范围和类型
	void ONVIF_init_ProbeType(struct soap* soap, struct wsdd__ProbeType* probe);

	//初始化soap
	struct soap* ONVIF_soap_new(int timeout);

	//清理空间
	void ONVIF_soap_delete(struct soap* soap);

	//探测设备,返回探测到的设备数量
	int ONVIF_DetectDevice(void(*cb)(char* DeviceXAddr));

	int PrintDetectedAddr();

	//获取媒体地址
	int GetMediaUrl(const char* device, char* media_url,const char* user,const char* psd);

	//获取profile
	int GetProfile(const char* media_url, char* main_token,char* sub_token, const char* user,const char* psd);

	//获取流地址
	int GetStreamUrl(const char* media_url,const char* token,char* rtsp,const char* user,const char* psd);

public:
	//获取onvif的rtsp
	int GetOnvifRtsp(char* main_rtsp, char* sub_rtsp,const char* user,const char* psd);
	MyOnvif();
	~MyOnvif();

private:
	unsigned int count = 0;				// 搜索到的设备个数
	struct soap* soap = nullptr;		// soap环境变量
	char cams[128][1024] = {0};
	char media_url[1024] = { 0 };
	char main_token[1024] = { 0 };
	char sub_token[1024] = { 0 };
};

