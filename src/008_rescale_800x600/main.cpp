#include <iostream>
#include <fstream>
#include <string>
#include "video_view.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"swscale.lib")

#ifdef av_err2str
#undef av_err2str
#endif
av_always_inline char* av_err2str(int errnum) {
	thread_local char str[AV_ERROR_MAX_STRING_SIZE];
	memset(str, 0, sizeof(str));
	return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}

int main(int argc,char* argv[])
{
	auto view = Video_View::Create();
	std::string filename = "800x600.h264";
	std::ifstream ifs(filename, std::ios::binary);
	if (!ifs)
	{
		std::cerr << "��ȡ�ļ�ʧ��" << std::endl;
		return -1;
	}
	unsigned char inbuf[4096] = { 0 };
	AVCodecID codec_id = AV_CODEC_ID_H264;

	auto codec = avcodec_find_decoder(codec_id);
    if (!codec) {
        fprintf(stderr, "Unsupported codec\n");
        return -1;
    }

	////��ӡ����֧�ֵ�Ӳ�����ٷ�ʽ
	//for (int i = 0; ; i++)
	//{
	//	auto config = avcodec_get_hw_config(codec, i);
	//	if (!config)break;
	//	if (config->device_type)
	//	{
	//		std::cout << av_hwdevice_get_type_name(config->device_type)<<std::endl;
	//	}
	//}

	auto c = avcodec_alloc_context3(codec);
	if (!c) {
		fprintf(stderr, "Could not allocate video codec context\n");
		return -1;
	}

	//Ӳ�����ٸ�ʽDXVA2
	//auto hw_type = AV_HWDEVICE_TYPE_DXVA2;

	////��ʼ��Ӳ������������
	//AVBufferRef* hw_ctx = nullptr;
	//av_hwdevice_ctx_create(&hw_ctx, hw_type, NULL, NULL, 0);

	//�趨Ӳ��gpu����
	//c->hw_device_ctx = av_buffer_ref(hw_ctx);
	c->thread_count = std::thread::hardware_concurrency();

	if (avcodec_open2(c, NULL, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		return -1;
	}


	//���Ų���
	int targetWidth = 400;
	int targetHeight = 300;

	auto parser = av_parser_init(codec_id);
	auto packet = av_packet_alloc();
	auto srcframe = av_frame_alloc();//ԭʼ֡
	auto scaled_frame = av_frame_alloc();//����֡
	auto hw_frame = av_frame_alloc();//Ӳ����ת����
	SwsContext* sws_ctx = nullptr;//��ʼ������������


	auto begin = NowMs();
	int count = 0;//����ͳ��
	bool is_init_win = false;

	while (!ifs.eof())
	{
		ifs.read((char*)inbuf, sizeof(inbuf));
		int data_size = ifs.gcount();
		if (data_size <= 0)break;
		//if (ifs.eof())
		//{
		//	ifs.clear();
		//	ifs.seekg(0, std::ios::beg);
		//}
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
				if (ret < 0) {
					std::cerr << "Error sending packet for decoding: " << av_err2str(ret) << std::endl;
					break;
				}
				while (ret>=0)
				{
					ret = avcodec_receive_frame(c, srcframe);
					if (ret == AVERROR(EAGAIN)) {
						break;
					}
					else if (ret < 0) {
						std::cerr << "Error receiving frame from decoder: " << av_err2str(ret) << std::endl;
						break;
					}
					
					//����Ӳ������
					auto pframe = srcframe;

					if (c->hw_device_ctx)//Ӳ����
					{
						av_hwframe_transfer_data(hw_frame, srcframe, 0);
						pframe = hw_frame;
					}

					//����������
					if (targetWidth && targetHeight > 0 && pframe->width && pframe->height)
					{
						sws_ctx = sws_getContext(pframe->width, pframe->height, (AVPixelFormat)pframe->format, targetWidth, targetHeight, AV_PIX_FMT_YUV420P, SWS_BILINEAR, nullptr, nullptr, nullptr);
						if (!sws_ctx)
						{
							fprintf(stderr, "Failed to create scaling context\n");
							return -1;
						}
					}
					
					if (sws_ctx)
					{
						av_frame_get_buffer(scaled_frame, 32);
						uint8_t* dstData[4] = { scaled_frame->data[0], scaled_frame->data[1], scaled_frame->data[2], nullptr };
						int dstLinesize[4] = { scaled_frame->linesize[0], scaled_frame->linesize[1], scaled_frame->linesize[2], 0 };
						sws_scale(sws_ctx,pframe->data,pframe->linesize,0,pframe->height, dstData, dstLinesize);
					}

					std::cout << pframe->format << " " << std::flush;
					//��һ֡��ʼ������
					if (!is_init_win)
					{
						is_init_win = true;
						view->Init(targetWidth, targetHeight, (Video_View::Format)pframe->format);
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
			if (packet->data) {
				av_packet_unref(packet);
			}
		}
	}

	// ���� NULL ����ˢ�½�����
	int ret = avcodec_send_packet(c, NULL);
	while (ret >= 0)
	{
		ret = avcodec_receive_frame(c, srcframe);
		if (ret == AVERROR(EAGAIN)) {
			continue;
		}
		else if (ret < 0) {
			std::cerr << "Error receiving frame from decoder after sending NULL packet: " << av_err2str(ret) << std::endl;
			break;
		}
	}

	av_parser_close(parser);
	avcodec_free_context(&c);
	av_packet_free(&packet);
	av_frame_free(&srcframe);
	av_frame_free(&hw_frame);
	//av_buffer_unref(&hw_ctx);
	return 0;
}