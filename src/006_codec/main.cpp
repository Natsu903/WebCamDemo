#include <iostream>
#include <fstream>
#include <string>
#include "video_view.h"
#include "decode.h"

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

	Decode de;
	auto c = de.Create(codec_id, false);
	de.set_c(c);

	de.InitHW();

	de.Open();

	//�ָ�������
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
			auto ret = av_parser_parse2(parser, c, 
				&packet->data, &packet->size, 
				data, data_size, AV_NOPTS_VALUE,
				AV_NOPTS_VALUE, 0);
			data += ret;
			data_size -= ret;
			if (packet->size)
			{
				if (!de.Send(packet))break;
				while (de.Recv(frame))
				{
					//��һ֡��ʼ������
					if (!is_init_win)
					{
						is_init_win = true;
						view->Init(frame->width, frame->height, (Video_View::Format)frame->format);
					}
					view->DrawFrame(frame);

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
	auto frames = de.End();
	for (auto f : frames)
	{
		view->DrawFrame(f);
		av_frame_free(&f);
	}

	av_parser_close(parser);
	avcodec_free_context(&c);
	av_packet_free(&packet);
	av_frame_free(&frame);
	return 0;
}