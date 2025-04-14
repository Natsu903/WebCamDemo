#include "sdlqtrgb.h"
#include <fstream>
#include <QMessageBox>
#include <iostream>
#include <QSpinBox>
#include <sstream>
#include "video_view.h"
extern "C"
{
#include <libavcodec/avcodec.h>
}

static std::ifstream yuv_file;
static int sdl_width = 0;
static int sdl_height = 0;
static const int pix_size = 2;
static Video_View* view = nullptr;
static AVFrame* frame = nullptr;
static long long file_size = 0;
static QLabel* view_fps = nullptr;
static QSpinBox* set_fps = nullptr;
int fps = 25;

SdlQtRGB::SdlQtRGB(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
    yuv_file.open("400_300_25.yuv", std::ios::binary);
    if (!yuv_file)
    {
        QMessageBox::information(this, "error", "read file error");
        return;
    }

	yuv_file.seekg(0, std::ios::end);//将文件指针移动到末尾
	file_size = yuv_file.tellg();//获取文件大小
	yuv_file.seekg(0, std::ios::beg);//将文件指针移动到开头

	connect(this, SIGNAL(ViewS()), this, SLOT(View()));

	view_fps = new QLabel(this);
	view_fps->setText("fps:100");

	set_fps = new QSpinBox(this);
	set_fps->move(200, 0);
	set_fps->setValue(25);
	set_fps->setRange(1, 300);

    sdl_width = 400;
    sdl_height = 300;
    ui.label->resize(sdl_width, sdl_height);
	view = Video_View::Create();
	view->Init(sdl_width, sdl_height, Video_View::YUV420P, (void*)ui.label->winId());
    
	//AVFrame初始化
	frame = av_frame_alloc();
	frame->width = sdl_width;
	frame->height = sdl_height;
	frame->format = AV_PIX_FMT_YUV420P;

	frame->linesize[0] = sdl_width;
	frame->linesize[1] = sdl_width / 2;
	frame->linesize[2] = sdl_width / 2;

	auto re = av_frame_get_buffer(frame, 0);
	if (re != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf));
		std::cerr << buf << std::endl;
	}

	//startTimer(10);
	//启动渲染线程
	th_ = std::thread(&SdlQtRGB::Main, this);
}

//void SdlQtRGB::timerEvent(QTimerEvent* ev)
//{
//	//yuv_file.read((char*)yuv, sdl_width * sdl_height * 1.5);
//	yuv_file.read((char*)frame->data[0], sdl_width * sdl_height);
//	yuv_file.read((char*)frame->data[1], sdl_width * sdl_height/4);
//	yuv_file.read((char*)frame->data[2], sdl_width * sdl_height/4);
//	if (view->IsExit())
//	{
//		view->Close();
//		exit(0);
//	}
//	//view->Draw(yuv);
//	view->DrawFrame(frame);
//}

void SdlQtRGB::resizeEvent(QResizeEvent* ev)
{
	ui.label->resize(size());
	ui.label->move(0, 0);
	//view->Scale(width(), height());
}

void SdlQtRGB::View()
{
	yuv_file.read((char*)frame->data[0], sdl_width * sdl_height);
	yuv_file.read((char*)frame->data[1], sdl_width * sdl_height / 4);
	yuv_file.read((char*)frame->data[2], sdl_width * sdl_height / 4);
	//循环播放
	if (yuv_file.tellg() == file_size)
	{
		yuv_file.seekg(0, std::ios::beg);
	}
	if (view->IsExit())
	{
		view->Close();
		exit(0);
	}
	view->DrawFrame(frame);
	std::stringstream ss;
	ss << "fps::" << view->render_fps();
	view_fps->setText(ss.str().c_str());
	fps = set_fps->value();
}

SdlQtRGB::~SdlQtRGB()
{
	th_.join();
}

void SdlQtRGB::Main()
{
	while (!is_exit_)
	{
		ViewS();
		if (fps > 0)
		{
			MSleep(1000 / fps);
		}
		else
			MSleep(10);
	}
}
