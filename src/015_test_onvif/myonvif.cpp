#include "myonvif.h"
#include "soapH.h"
#include "wsdd.nsmap"
#include "stdsoap2.h"
#include "wsseapi.h"
#include "wsaapi.h"
#include <assert.h>
#include <iostream>
#define SOAP_ASSERT     assert
#define SOAP_DBGLOG     printf
#define SOAP_DBGERR     printf

#define SOAP_TO "urn:schemas-xmlsoap-org:ws:2005:04:discovery"
#define SOAP_ACTION "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"
#define SOAP_ITEM ""
#define SOAP_TYPES "dn:NetworkVideoTransmitter"
#define SOAP_MCAST_ADDR "soap.udp://239.255.255.250:3702"

#define SOAP_SOCK_TIMEOUT (3) 

#pragma comment(lib,"libssl.lib")
#pragma comment(lib,"libcrypto.lib")

void MyOnvif::soap_perror(struct soap* soap, const char* str)
{
	if (NULL == str) {
		SOAP_DBGERR("[soap] error: %d, %s, %s\n", soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	}
	else {
		SOAP_DBGERR("[soap] %s error: %d, %s, %s\n", str, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	}
	return;
}


void* MyOnvif::ONVIF_soap_malloc(struct soap* soap, unsigned int n)
{
	void* p = nullptr;
	if (n > 0)
	{
		p = soap_malloc(soap, n);
		SOAP_ASSERT(NULL != p);
		memset(p, 0x00, n);
	}
	return p;
}

//初始化消息头
void MyOnvif::ONVIF_init_header(struct soap* soap)
{
	struct SOAP_ENV__Header* header = nullptr;
	SOAP_ASSERT(NULL != soap);
	header = (struct SOAP_ENV__Header*)ONVIF_soap_malloc(soap, sizeof(struct SOAP_ENV__Header));
	soap_default_SOAP_ENV__Header(soap, header);
	header->wsa__MessageID = (char*)soap_wsa_rand_uuid(soap);
	header->wsa__To = (char*)ONVIF_soap_malloc(soap, strlen(SOAP_TO) + 1);
	header->wsa__Action = (char*)ONVIF_soap_malloc(soap, strlen(SOAP_ACTION) + 1);
	strcpy(header->wsa__To, SOAP_TO);
	strcpy(header->wsa__Action, SOAP_ACTION);
	soap->header = header;
	return;
}

//初始化探测设备的范围和类型
void MyOnvif::ONVIF_init_ProbeType(struct soap* soap, struct wsdd__ProbeType* probe)
{
	// 用于描述查找哪类的Web服务
	struct wsdd__ScopesType* scope = nullptr;

	SOAP_ASSERT(NULL != soap);
	SOAP_ASSERT(NULL != probe);

	scope = (struct wsdd__ScopesType*)ONVIF_soap_malloc(soap, sizeof(struct wsdd__ScopesType));
	// 设置寻找设备的范围
	soap_default_wsdd__ScopesType(soap, scope);
	scope->__item = (char*)ONVIF_soap_malloc(soap, strlen(SOAP_ITEM) + 1);
	strcpy(scope->__item, SOAP_ITEM);

	memset(probe, 0x00, sizeof(struct wsdd__ProbeType));
	soap_default_wsdd__ProbeType(soap, probe);
	probe->Scopes = scope;
	// 设置寻找设备的类型
	probe->Types = (char*)ONVIF_soap_malloc(soap, strlen(SOAP_TYPES) + 1);
	strcpy(probe->Types, SOAP_TYPES);
	return;
}

struct soap* MyOnvif::ONVIF_soap_new(int timeout)
{
	SOAP_ASSERT(NULL != (soap = soap_new()));
	// 在创建soap对象后添加UDP模式设置
	soap_set_mode(soap, SOAP_IO_UDP);
	// 设置soap的namespaces
	soap_set_namespaces(soap, namespaces);
	// 设置超时（超过指定时间没有数据就退出）
	soap->recv_timeout = timeout;
	soap->send_timeout = timeout;
	soap->connect_timeout = timeout;
	// 设置为UTF-8编码，否则叠加中文OSD会乱码
	soap_set_mode(soap, SOAP_C_UTFSTRING);
	return soap;
}

void MyOnvif::ONVIF_soap_delete(struct soap* soap)
{
	//清理soap
	soap_destroy(soap);//c++析构
	soap_end(soap);//清理临时空间
	soap_done(soap);//关闭通信，移除回调
	soap_free(soap);//释放空间
}

int MyOnvif::ONVIF_DetectDevice(void(*cb)(char* DeviceXAddr))
{
	count = 0;	//每次探测前清空之前的探测信息
	struct wsdd__ProbeType req;			// 用于发送Probe消息
	struct __wsdd__ProbeMatches rep;	// 用于接收Probe应答
	struct wsdd__ProbeMatchType* probeMatch;


	ONVIF_init_header(soap);// 设置消息头描述		
	ONVIF_init_ProbeType(soap, &req);// 设置寻找的设备的范围和类型
	auto result = soap_send___wsdd__Probe(soap, SOAP_MCAST_ADDR, nullptr, &req);// 向组播地址广播Probe消息
	
	while (SOAP_OK == result)// 开始循环接收设备发送过来的消息
	{
		memset(&req, 0x00, sizeof(req));
		result = soap_recv___wsdd__ProbeMatches(soap, &rep);
		if (SOAP_OK == result)
		{
			if (soap->error)
			{
				soap_perror(soap, "ProbeMatches");
			}
			else// 成功接收到设备的应答消息
			{
				if (NULL != rep.wsdd__ProbeMatches)
				{
					char* device_service = rep.wsdd__ProbeMatches->ProbeMatch->XAddrs;
					if (device_service) strcpy(cams[count], device_service);
					count += rep.wsdd__ProbeMatches->__sizeProbeMatch;
					for (int i = 0; i < rep.wsdd__ProbeMatches->__sizeProbeMatch; i++)
					{
						probeMatch = rep.wsdd__ProbeMatches->ProbeMatch + 1;
						if (NULL != cb)
						{
							cb(probeMatch->XAddrs);// 使用设备服务地址执行函数回调
						}
					}
				}
			}
		}
		else if (soap->error)
		{
			break;
		}
	}
	SOAP_DBGLOG("\ndetect end! It has detected %d devices!\n", count);
	return count;
}

int MyOnvif::PrintDetectedAddr()
{
	for (int i = 0; i < count; i++)
	{
		std::cout << cams[i] << std::endl;
	}
	return 0;
}

int MyOnvif::GetMediaUrl(const char* device, char* media_url, const char* user, const char* psd)
{
	if (!device || !media_url)return -1;
	if (!soap) return -1;
	if (user && psd)
	{
		//添加鉴权信息
		soap_wsse_add_UsernameTokenDigest(soap,nullptr,user,psd);
	}
	//设定获取所有类型
	struct _tds__GetCapabilities *req= soap_new__tds__GetCapabilities(soap);
	struct _tds__GetCapabilitiesResponse *resp=soap_new__tds__GetCapabilitiesResponse(soap);
	//获取能力
	auto re = soap_call___tds__GetCapabilities(soap, device, nullptr, req, *resp);
	if (re == SOAP_OK)
	{
		strcpy(media_url, resp->Capabilities->Media->XAddr.c_str());
		return 0;
	}
	return -1;
}

int MyOnvif::GetProfile(const char* media_url, char* main_token, char* sub_token, const char* user, const char* psd)
{
	if(!media_url||!main_token||!sub_token) return -1;
	if (!soap) return -1;
	if (user && psd)
	{
		//添加鉴权信息
		soap_wsse_add_UsernameTokenDigest(soap, nullptr, user, psd);
	}
	struct _trt__GetProfiles *req = soap_new__trt__GetProfiles(soap);
	struct _trt__GetProfilesResponse *resp = soap_new__trt__GetProfilesResponse(soap);
    int re = soap_call___trt__GetProfiles(soap, media_url, nullptr, req, *resp);
    if (re == SOAP_OK)
	{
		if (resp->Profiles.size() >= 1)
		{
			//主码流
            strcpy(main_token, resp->Profiles[0]->token.c_str());
		}
		if (resp->Profiles.size() >= 2)
		{
			//辅码流
            strcpy(sub_token, resp->Profiles[1]->token.c_str());
		}
        return 0;
	}
	printf("[soap error:]%s",*soap_faultstring(soap));
	return -1;
}

int MyOnvif::GetStreamUrl(const char* media_url, const char* token, char* rtsp, const char* user, const char* psd)
{
	if (!media_url || !token || !rtsp) return -1;
	if (!soap) return -1;
	if (user && psd)
	{
		//添加鉴权信息
		soap_wsse_add_UsernameTokenDigest(soap, nullptr, user, psd);
	}
	struct _trt__GetStreamUri *req=soap_new__trt__GetStreamUri(soap);
	struct _trt__GetStreamUriResponse *resp=soap_new__trt__GetStreamUriResponse(soap);

	struct tt__StreamSetup *streamSetup=soap_new_tt__StreamSetup(soap);
	struct tt__Transport *transport=soap_new_tt__Transport(soap);

	streamSetup->Stream = tt__StreamType::RTP_Unicast;
	streamSetup->Transport = transport;
	streamSetup->Transport->Protocol = tt__TransportProtocol::RTSP;
	req->StreamSetup = streamSetup;
	req->ProfileToken = token;

	int ret = soap_call___trt__GetStreamUri(soap, media_url, nullptr, req, *resp);
	if (ret == SOAP_OK)
	{
		printf("GetStreamUri: %s\n", resp->MediaUri->Uri.c_str());
		strcpy(rtsp, resp->MediaUri->Uri.c_str());
		return 0;
	}

	return -1;
}

int MyOnvif::GetOnvifRtsp(char* main_rtsp, char* sub_rtsp, const char* user, const char* psd)
{
	if(!main_rtsp||!sub_rtsp) return -1;
	ONVIF_DetectDevice(nullptr);
	PrintDetectedAddr();
	for (auto device : cams)
	{
		if (*device=='\0')break;
		int ret = GetMediaUrl(device, media_url, user, psd);
		if (ret != 0) return ret;
		ret = GetProfile(media_url, main_token, sub_token, user, psd);
		if (ret != 0) return ret;
		ret = GetStreamUrl(media_url, main_token, main_rtsp, user, psd);
		if (ret != 0) return ret;
		ret = GetStreamUrl(media_url, sub_token, sub_rtsp, user, psd);
		if (ret != 0) return ret;
	}
	return 0;
}

MyOnvif::MyOnvif()
{
	SOAP_ASSERT(NULL != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));
}

MyOnvif::~MyOnvif()
{
	if (NULL != soap)
	{
		ONVIF_soap_delete(soap);
	}
}
