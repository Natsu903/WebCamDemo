#include <iostream>
#include <fstream>
#include <string>
#include "video_view.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avutil.lib")

int main(int argc,char* argv[])
{
	auto view = Video_View::Create();
	std::string filename = "test.h264";
	std::ifstream ifs(filename, std::ios::binary);
	if (!ifs)
	{
		std::cerr << "��ȡ�ļ�ʧ��" << std::endl;
		return -1;
	}
	unsigned char inbuf[4096] = { 0 };
	AVCodecID codec_id = AV_CODEC_ID_H264;

	auto codec = avcodec_find_decoder(codec_id);

	//��ӡ����֧�ֵ�Ӳ�����ٷ�ʽ
	for (int i = 0; ; i++)
	{
		auto config = avcodec_get_hw_config(codec, i);
		if (!config)break;
		if (config->device_type)
		{
			std::cout << av_hwdevice_get_type_name(config->device_type)<<std::endl;
		}
	}

	auto c = avcodec_alloc_context3(codec);

	//Ӳ�����ٸ�ʽDXVA2
	auto hw_type = AV_HWDEVICE_TYPE_DXVA2;

	//��ʼ��Ӳ������������
	AVBufferRef* hw_ctx = nullptr;
	av_hwdevice_ctx_create(&hw_ctx, hw_type, NULL, NULL, 0);

	//�趨Ӳ��gpu����
	c->hw_device_ctx = av_buffer_ref(hw_ctx);
	c->thread_count = std::thread::hardware_concurrency();

	avcodec_open2(c, NULL, NULL);

	auto parser = av_parser_init(codec_id);
	auto packet = av_packet_alloc();
	auto frame = av_frame_alloc();
	auto hw_frame = av_frame_alloc();//Ӳ����ת����

	auto begin = NowMs();
	int count = 0;//����ͳ��
	bool is_init_win = false;

	while (!ifs.eof())
	{
		ifs.read((char*)inbuf, sizeof(inbuf));
		int data_size = ifs.gcount();
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
			 *	parser�������������ģ����ڹ���������̡�
				c�������������ģ������ṩ�����������Ϣ��
				&pkt->data��ָ�� AVPacket ���ݻ�������ָ�룬���ڴ洢����������ݡ�
				&pkt->size��ָ�� AVPacket ���ݴ�С��ָ�룬���ڴ洢����������ݴ�С��
				data����������ݻ�������
				data_size����������ݴ�С��
				AV_NOPTS_VALUE����ʾû��ʱ�����Ϣ��
				AV_NOPTS_VALUE����ʾû�н���ʱ�����Ϣ��
				0����ʾ���������е�λ�á�
			 */
			int ret = av_parser_parse2(parser, c, 
				&packet->data, &packet->size, 
				data, data_size, AV_NOPTS_VALUE,
				AV_NOPTS_VALUE, 0);
			data += ret;
			data_size -= ret;
			if (packet->size)
			{
				ret = avcodec_send_packet(c, packet);
				if (ret < 0)break;
				while (ret>=0)
				{
					ret = avcodec_receive_frame(c, frame);
					if (ret < 0)break;
					
					auto pframe = frame;//Ϊ��ͬʱ֧��Ӳ�����
					if (c->hw_device_ctx)//Ӳ����
					{
						av_hwframe_transfer_data(hw_frame, frame, 0);
						pframe = hw_frame;
					}
					AV_PIX_FMT_DXVA2_VLD;
					
					std::cout << pframe->format << " " << std::flush;
					//��һ֡��ʼ������
					if (!is_init_win)
					{
						is_init_win = true;
						view->Init(pframe->width, pframe->height, (Video_View::Format)pframe->format);
					}
					view->DrawFrame(pframe);

					count++;
					auto cur = NowMs();
					if (cur - begin >= 100)
					{
						std::cout << "\nfps" << count * 10 << std::endl;
						count = 0;
						begin = cur;
					}
					
				}
			}
		}
	}

	int ret = avcodec_send_packet(c, NULL);
	while (ret>=0)
	{
		ret = avcodec_send_frame(c, frame);
		if (ret < 0)break;
		std:: cout << frame->format << "-" << std::flush;
	}

	av_parser_close(parser);
	avcodec_free_context(&c);
	av_packet_free(&packet);
	av_frame_free(&frame);
	return 0;
}