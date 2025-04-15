#include "sdlqtrgb.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "video_view.h"
#include <QFileDialog>

static std::vector<Video_View*> views;

SdlQtRGB::SdlQtRGB(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
    
	connect(this, SIGNAL(ViewS()), this, SLOT(View()));
	//初始化传入两个渲染窗口的winid
	views.push_back(Video_View::Create());
	views.push_back(Video_View::Create());
	views[0]->set_win_id((void*)ui.label_video1->winId());
	views[1]->set_win_id((void*)ui.label_video2->winId());

	//启动渲染线程
	th_ = std::thread(&SdlQtRGB::Main, this);
}

void SdlQtRGB::timerEvent(QTimerEvent* ev)
{
	
}

void SdlQtRGB::resizeEvent(QResizeEvent* ev)
{
	
}

void SdlQtRGB::View()
{
	static int last_pts[32] = { 0 };// 记录每个视图上次渲染的时间戳（毫秒）
	static int fps_arr[32] = { 0 };//记录每一个试图的帧率值
	fps_arr[0] = ui.spinBox_fps1->value();
	fps_arr[1] = ui.spinBox_fps2->value();
	for (int i = 0; i <= views.size(); i++)
	{
		//如果设置的帧率不合规则跳过本次循环，如果合规计算帧间隔时间
		if(fps_arr[i]<=0)continue;
		int ms = 1000 / fps_arr[i];
		//通过 NowMs() 函数获取当前时间戳，判断是否达到渲染时间点。未达间隔则跳过渲染，避免过度绘制
		if (NowMs() - last_pts[i] < ms)continue;
		last_pts[i] = NowMs();
		
		auto frame = views[i]->Read();
		if (!frame)continue;
		views[i]->DrawFrame(frame);
		std::stringstream ss;
		ss << "fps:" << views[i]->render_fps();
		if (i == 0)
			ui.label_fps1->setText(ss.str().c_str());
		if(i==1)
			ui.label_fps2->setText(ss.str().c_str());
	}

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
		MSleep(10);
	}
}

void SdlQtRGB::Open1()
{
	Open(0);
}

void SdlQtRGB::Open2()
{
	Open(1);
}

void SdlQtRGB::Open(int i)
{
	QFileDialog fd;
	auto filename = fd.getOpenFileName();
	if (filename.isEmpty())return;
	std::cout << filename.toLocal8Bit().data() << std::endl;
	if (!views[i]->Open(filename.toLocal8Bit().toStdString())) return;
	int w = 0;
	int h = 0;
	QString pix = 0;
	if (i == 0)
	{
		w = ui.spinBox_width1->value();
		h = ui.spinBox_height1->value();
		pix = ui.comboBox_frame1->currentText();
	}
	if (i == 1)
	{
		w = ui.spinBox_width2->value();
		h = ui.spinBox_height2->value();
		pix = ui.comboBox_frame2->currentText();
	}
	Video_View::Format fmt = Video_View::YUV420P;
	if (pix == "YUV420P")
	{

	}
	else if (pix == "RGBA")
	{
		fmt = Video_View::RGBA;
	}
	else if (pix == "ARGB")
	{
		fmt = Video_View::ARGB;
	}
	else if (pix == "BGRA")
	{
		fmt = Video_View::BGRA;
	}
	views[i]->Init(w,h,fmt);
}
