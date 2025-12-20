#pragma once

#include <QWidget>
#include "common.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; };
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

public slots:
    //1
    void selectFile_1();
    void saveAsFile_1();
    void muxing();
    //2
    void selectFile_2();
    void saveAsFile_2();
    void cutMuxing();
    //3
    void saveAsFile_3();
    void encodeMuxing();
    void encode_mux_fun_3(AVCodecContext* codecCtx, AVFrame* frame, AVPacket* pkt, AVFormatContext* oFmtCtx, AVStream* oStream);
private:
    Ui::Widget *ui;
};
