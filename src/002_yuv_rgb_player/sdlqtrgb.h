#pragma once

#include <QtWidgets/QWidget>
#include "ui_sdlqtrgb.h"
#include <thread>

class SdlQtRGB : public QWidget
{
    Q_OBJECT

public:
    SdlQtRGB(QWidget *parent = nullptr);
    //��ʱ�������ڸ��»���
    void timerEvent(QTimerEvent* ev) override;
    //���ڵ�����������ʵ�ִ�������
    void resizeEvent(QResizeEvent* ev)override;
    ~SdlQtRGB();
    void Main();

signals:
    void ViewS();

public slots:
    void View();
    void Open1();
    void Open2();
    void Open(int i);

private:
    Ui::SdlQtRGBClass ui;
    std::thread th_;
    bool is_exit_ = false;
};
