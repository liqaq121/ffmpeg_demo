#include "Widget.h"
#include "ui_Widget.h"

#include "common.h"

#include <QPushButton>
#include <QFileDialog>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget())
{
    ui->setupUi(this);

    connect(ui->btnSrcFile, &QPushButton::clicked, this, &Widget::selectFile);
    connect(ui->btnDestFile, &QPushButton::clicked, this, &Widget::saveAsFile);
    connect(ui->btnConvert, &QPushButton::clicked, this, &Widget::convert);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::selectFile()
{
    QString path = QFileDialog::getOpenFileName(this);
    if (!path.isEmpty())
    {
        ui->leSrcFile->setText(path);
    }
}

void Widget::saveAsFile()
{
    QString path = QFileDialog::getSaveFileName(this, "保存文件", "output.mp4");
    if (!path.isEmpty())
    {
        ui->leDestFile->setText(path);
    }
}

void Widget::convert()
{
    av_log_set_level(AV_LOG_DEBUG);
    
    //mp4(H264/AAC) > mov
    std::string a = ui->leSrcFile->text().toStdString();
    std::string b = ui->leDestFile->text().toStdString();
    const char* inFile = a.c_str();
    const char* outFile = b.c_str();
    const char* inFile = ui->leSrcFile->text().toStdString().c_str();
    int ret = 0;

    AVFormatContext* iFmtCtx = nullptr;
    AVFormatContext* oFmtCtx = nullptr;

    ret = avformat_open_input(&iFmtCtx, inFile, nullptr, nullptr);
    ret = avformat_find_stream_info(iFmtCtx, nullptr);

    ret = avformat_alloc_output_context2(&oFmtCtx, nullptr, nullptr, outFile);
    ret = avio_open2(&oFmtCtx->pb, outFile, AVIO_FLAG_WRITE, nullptr, nullptr);

    std::map<int, AVStream*> oStreams;
    for (int i = 0; i < iFmtCtx->nb_streams; i++)
    {
        AVStream* iStream = iFmtCtx->streams[i];
        AVCodecParameters* codecPar = iStream->codecpar;
        if (codecPar->codec_type != AVMEDIA_TYPE_VIDEO && codecPar->codec_type != AVMEDIA_TYPE_AUDIO && codecPar->codec_type != AVMEDIA_TYPE_SUBTITLE)
            continue;

        AVStream* oStream = avformat_new_stream(oFmtCtx, nullptr);
        oStreams[i] = oStream;
        ret = avcodec_parameters_copy(oStream->codecpar, iStream->codecpar);
    }
    ret = avformat_write_header(oFmtCtx, nullptr);


    AVPacket pkt;
    av_init_packet(&pkt);

    while (av_read_frame(iFmtCtx, &pkt) >= 0)
    {
        auto it = std::find_if(oStreams.cbegin(), oStreams.cend(), [index = pkt.stream_index](const std::pair<int, AVStream*>& p) {
            return p.second->index == index;
            });
        if (it == oStreams.cend())
            continue;

        AVStream* iStream = iFmtCtx->streams[pkt.stream_index];
        AVStream* oStream = it->second;

        av_packet_rescale_ts(&pkt, iStream->time_base, oStream->time_base);

        ret = av_interleaved_write_frame(oFmtCtx, &pkt);
        av_packet_unref(&pkt);
    }
    
    ret = av_write_trailer(oFmtCtx);

    ret = avio_closep(&oFmtCtx->pb);
    avformat_free_context(oFmtCtx);
    avformat_close_input(&iFmtCtx);
}