#include <iostream>
#include <fstream>
#include <string>
#include "video_view.h"
#include "decode.h"
#include <windows.h>
#include <d3d9.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avutil.lib")

/**
 * 封装DXVA2硬件解码所需的资源，包括Direct3D9库、DXVA2接口及设备上下文。d3d9device用于管理GPU加速的渲染流程.
 */
struct DXVA2DevicePriv {
	HMODULE d3dlib;
	HMODULE dxva2lib;
	HANDLE device_handle;
	IDirect3D9* d3d9;
	IDirect3DDevice9* d3d9device;
};

void DrawFrame(AVFrame* frame, AVCodecContext* c)
{
	if (!frame->data[3] || !c)return;
	//std::cout << "dxva" << std::endl;
	//提取 DXVA2 表面​：frame->data[3] 存储硬件解码后的 GPU 表面指针，需强制转换为 IDirect3DSurface9。
	auto surface = (IDirect3DSurface9*)frame->data[3];
	//获取硬件设备上下文​：通过 AVCodecContext 的 hw_device_ctx 字段，
	auto ctx = (AVHWDeviceContext*)c->hw_device_ctx->data;
	//获取自定义的 DXVA2DevicePriv 结构体，最终得到 Direct3D9 设备指针 d3d9device。
	auto priv = (DXVA2DevicePriv*)ctx->user_opaque;
	auto device = priv->d3d9device;

	// 窗口与视口初始化
	static HWND hwnd = nullptr;
	static RECT viewport;
	if (!hwnd)
	{
		hwnd = CreateWindow(L"DX", L"Test DXVA", WS_OVERLAPPEDWINDOW,
			200, 200, frame->width, frame->height, 0, 0, 0, 0);
		ShowWindow(hwnd, 1);
		UpdateWindow(hwnd);
		viewport.left = 0;
		viewport.right = frame->width;
		viewport.top = 0;
		viewport.bottom = frame->height;
	}
	//提交渲染请求 后台缓冲区内容显示到窗口。
	device->Present(&viewport, &viewport, hwnd, 0);
	//后台缓冲表面
	static IDirect3DSurface9* back = nullptr;
	if (!back)
		//获取 Direct3D 的后台缓冲表面 back，用于存储待渲染的帧。
		//D3DBACKBUFFER_TYPE_MONO 表示单缓冲模式（无立体视觉）。
		device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back);
	device->StretchRect(surface, 0, back, &viewport, D3DTEXF_LINEAR);
}

int main(int argc,char* argv[])
{
	//初始化windows窗口
	WNDCLASSEX wc;
	//初始化结构体内存
	memset(&wc, 0, sizeof(wc));
	//设置结构体大小
	wc.cbSize=sizeof(wc);
	//指定窗口消息处理函数
	wc.lpfnWndProc = DefWindowProc;
	//定义窗口类名称
	wc.lpszClassName = L"DX";
	RegisterClassEx(&wc);

	//auto view = Video_View::Create();
	std::string filename = "test.h264";
	std::ifstream ifs(filename, std::ios::binary);
	if (!ifs)
	{
		std::cerr << "读取文件失败" << std::endl;
		return -1;
	}
	unsigned char inbuf[4096] = { 0 };
	AVCodecID codec_id = AV_CODEC_ID_H264;

	Decode de;
	auto c = de.Create(codec_id, false);
	de.set_c(c);

	de.InitHW();

	de.Open();

	//分割上下文
	auto parser = av_parser_init(codec_id);
	auto packet = av_packet_alloc();
	auto frame = av_frame_alloc();
	auto hw_frame = av_frame_alloc();//硬解码转换用

	auto begin = NowMs();
	int count = 0;//解码统计
	bool is_init_win = false;

	while (!ifs.eof())
	{
		ifs.read((char*)inbuf, sizeof(inbuf));
		auto data_size = ifs.gcount();
		if (data_size <= 0)break;
		if (ifs.eof())
		{
			ifs.clear();
			ifs.seekg(0, std::ios::beg);
		}
		auto data = inbuf;
		while (data_size > 0) 
		{
			/**
			 *	通过0001 截断输出到AVPacket 返回帧大小
			 *	parser：解析器上下文，用于管理解析过程。
				c：解码器上下文，用于提供解码所需的信息。
				&pkt->data：指向 AVPacket 数据缓冲区的指针，用于存储解析后的数据。
				&pkt->size：指向 AVPacket 数据大小的指针，用于存储解析后的数据大小。
				data：输入的数据缓冲区。
				data_size：输入的数据大小。
				AV_NOPTS_VALUE：表示没有时间戳信息。
				AV_NOPTS_VALUE：表示没有解码时间戳信息。
				0：表示数据在流中的位置。
			 */
			auto ret = av_parser_parse2(parser, c, 
				&packet->data, &packet->size, 
				data, data_size, AV_NOPTS_VALUE,
				AV_NOPTS_VALUE, 0);
			data += ret;
			data_size -= ret;
			if (packet->size)
			{
				if (!de.Send(packet))break;
				while (de.Recv(frame,false))
				{
					//第一帧初始化窗口
					if (!is_init_win)
					{
						is_init_win = true;
						//view->Init(frame->width, frame->height, (Video_View::Format)frame->format);
					}
					DrawFrame(frame, c);
					//view->DrawFrame(frame);

					count++;
					auto cur = NowMs();
					if (cur - begin >= 1000)
					{
						std::cout << "\nfps" << count << std::endl;
						count = 0;
						begin = cur;
					}
				}
			}
		}
	}
	//auto frames = de.End();
	//for (auto f : frames)
	//{
	//	view->DrawFrame(f);
	//	av_frame_free(&f);
	//}

	av_parser_close(parser);
	avcodec_free_context(&c);
	av_packet_free(&packet);
	av_frame_free(&frame);
	return 0;
}