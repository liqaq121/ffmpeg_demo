#include "Widget.h"
#include "ui_Widget.h"

#include "common.h"

#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextEdit>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget())
{
    ui->setupUi(this);

    //1
    connect(ui->btnSrcFile, &QPushButton::clicked, this, &Widget::selectFile_1);
    connect(ui->btnDestFile, &QPushButton::clicked, this, &Widget::saveAsFile_1);
    connect(ui->btnMuxing, &QPushButton::clicked, this, &Widget::muxing);

    //2.
    connect(ui->btnSrcFile_2, &QPushButton::clicked, this, &Widget::selectFile_2);
    connect(ui->btnDestFile_2, &QPushButton::clicked, this, &Widget::saveAsFile_2);
    connect(ui->btnCutMuxing, &QPushButton::clicked, this, &Widget::cutMuxing);

    //3
    connect(ui->btnDestFile_3, &QPushButton::clicked, this, &Widget::saveAsFile_3);
    connect(ui->btnGenEncodeMuxing, &QPushButton::clicked, this, &Widget::encodeMuxing);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::selectFile_1()
{
    QString path = QFileDialog::getOpenFileName(this);
    if (!path.isEmpty())
    {
        ui->leSrcFile->setText(path);
    }
}

void Widget::saveAsFile_1()
{
    QString path = QFileDialog::getSaveFileName(this, "保存文件", "output.mp4");
    if (!path.isEmpty())
    {
        ui->leDestFile->setText(path);
    }
}

void Widget::muxing()
{
    av_log_set_level(AV_LOG_DEBUG);
    
    //mp4(H264/AAC) > mov
    std::string a = ui->leSrcFile->text().toStdString();
    std::string b = ui->leDestFile->text().toStdString();
    const char* inFile = a.c_str();
    const char* outFile = b.c_str();
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

void Widget::selectFile_2()
{
    QString path = QFileDialog::getOpenFileName(this);
    if (!path.isEmpty())
    {
        ui->leSrcFile_2->setText(path);
    }
}

void Widget::saveAsFile_2()
{
    QString path = QFileDialog::getSaveFileName(this, "保存文件", "output.mp4");
    if (!path.isEmpty())
    {
        ui->leDestFile_2->setText(path);
    }
}

void Widget::cutMuxing()
{
    ui->textEdit->append("----------------cut+muxing");

    std::string  a = ui->leSrcFile_2->text().toStdString();
    std::string  b = ui->leDestFile_2->text().toStdString();
    const char* inFile = a.c_str();
    const char* outFile = b.c_str();

    int64_t start_time = ui->leStartTime->text().toInt();
    int64_t end_time = ui->leEndTime->text().toInt();

    ui->textEdit->append(QString::fromStdString("inFile: " + a));
    ui->textEdit->append(QString::fromStdString("outFile: " + b));
    ui->textEdit->append(QString("%1~%2s").arg(start_time).arg(end_time));

    int ret = -1;

    AVFormatContext* iFmtCtx = nullptr;
    AVFormatContext* oFmtCtx = nullptr;

    ret = avformat_open_input(&iFmtCtx, inFile, nullptr, nullptr);
    ret = avformat_find_stream_info(iFmtCtx, nullptr);
    ret = avformat_alloc_output_context2(&oFmtCtx, nullptr, nullptr, outFile);

    std::map<int, AVStream*> outStreams;
    for (int i = 0; i < iFmtCtx->nb_streams; i++)
    {
        AVStream* inStream = iFmtCtx->streams[i];
        AVCodecParameters* codePar = inStream->codecpar;

        if (codePar->codec_type != AVMEDIA_TYPE_VIDEO && codePar->codec_type != AVMEDIA_TYPE_AUDIO && codePar->codec_type != AVMEDIA_TYPE_SUBTITLE)
            continue;

        AVStream* outStream = avformat_new_stream(oFmtCtx, nullptr);
        ret = avcodec_parameters_copy(outStream->codecpar, inStream->codecpar);
        outStream->codecpar->codec_tag = 0;
        outStreams.emplace(inStream->index, outStream);
    }

    //跳转到指定位置         流索引（-1：全部）
    ret = av_seek_frame(iFmtCtx, -1, start_time * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);

    ret = avio_open(&oFmtCtx->pb, outFile, AVIO_FLAG_WRITE);

    ret = avformat_write_header(oFmtCtx, nullptr);

    //每个流第一个包pts时间戳, dts时间戳
    std::map<int, uint64_t> pts_stamp;
    std::map<int, uint64_t> dts_stamp;

    AVPacket* pkt = av_packet_alloc();
    while (av_read_frame(iFmtCtx, pkt) >= 0)
    {
        auto it = outStreams.find(pkt->stream_index);
        if (it == outStreams.end())
        {
            av_packet_unref(pkt);
            continue;
        }

        if (pkt->pts < 0 || pkt->dts < 0 || pkt->pts == AV_NOPTS_VALUE || pkt->dts == AV_NOPTS_VALUE)
        {
            av_packet_unref(pkt);
            continue;
        }

        //记录每个流的首包时间戳
        if (pts_stamp.find(pkt->stream_index) == pts_stamp.end())
        {
            pts_stamp[it->first] = pkt->pts;
        }
        if (dts_stamp.find(pkt->stream_index) == dts_stamp.end())
        {
            dts_stamp[it->first] = pkt->dts;
        }

        AVStream* inStream = iFmtCtx->streams[it->first]; //iFmtCtx->streams[pkt->stream_index];
        AVStream* outStream = it->second;

        //std::cout << "时间: " << av_q2d(inStream->time_base) * pkt->pts << std::endl;

        if (pkt->pts * av_q2d(inStream->time_base) > end_time)
        {
            av_packet_unref(pkt);
            break;
        }

        pkt->pts = pkt->pts - pts_stamp[it->first];
        pkt->dts = pkt->dts - dts_stamp[it->first];
        if (pkt->pts < pkt->dts)
            pkt->pts = pkt->dts;

        av_packet_rescale_ts(pkt, inStream->time_base, outStream->time_base);
        pkt->pos = -1;

        ret = av_interleaved_write_frame(oFmtCtx, pkt);

        av_packet_unref(pkt);
    }
    av_packet_free(&pkt);

    av_write_trailer(oFmtCtx);

    avio_closep(&oFmtCtx->pb);
    avformat_free_context(oFmtCtx);
    avformat_close_input(&iFmtCtx);

    ui->textEdit->append("cut-muxing finished");
    QMessageBox::information(this, "提示", "操作结束");
}

void Widget::saveAsFile_3()
{
    QString path = QFileDialog::getSaveFileName(this, "保存文件", "output.mp4");
    if (!path.isEmpty())
    {
        ui->leDestFile_3->setText(path);
    }
}

void Widget::encodeMuxing()
{
    std::string a = ui->leDestFile_3->text().toStdString();
    std::string b = ui->leEncoderName->text().toStdString();
    const char* fileName = a.c_str();
    const char* encoderName = b.c_str();

    const AVCodec* codec = avcodec_find_encoder_by_name(encoderName);
    AVCodecContext* codecCtx = avcodec_alloc_context3(codec);
    codecCtx->width = 640;
    codecCtx->height = 360;
    codecCtx->bit_rate = 500000;

    codecCtx->gop_size = 10;
    codecCtx->max_b_frames = 1;
    codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

    codecCtx->time_base = { 1,60 };
    codecCtx->framerate = { 60,1 };

    if (codecCtx->codec_id == AV_CODEC_ID_H264)
        av_opt_set(codecCtx->priv_data, "preset", "slow", 0);

    AVFrame* frame = av_frame_alloc();
    frame->width = codecCtx->width;
    frame->height = codecCtx->height;
    frame->format = codecCtx->pix_fmt;
    av_frame_get_buffer(frame, 0);

    AVPacket* pkt = av_packet_alloc();

    avcodec_open2(codecCtx, codec, nullptr);

    AVFormatContext* ofmtCtx = nullptr;
    avformat_alloc_output_context2(&ofmtCtx, nullptr, nullptr, fileName);
    AVStream* oStream = avformat_new_stream(ofmtCtx, codec);
    avcodec_parameters_from_context(oStream->codecpar, codecCtx);
    oStream->codecpar->codec_tag = 0;
    oStream->index = 0;

    avio_open2(&ofmtCtx->pb, fileName, AVIO_FLAG_WRITE, nullptr, nullptr);

    avformat_write_header(ofmtCtx, nullptr);

    for (int i = 0; i < 60; i++)
    {
        av_frame_make_writable(frame);
        //y
        for (int y = 0; y < codecCtx->height; y++)
        {
            for (int x = 0; x < codecCtx->width; x++)
            {
                frame->data[0][y * frame->linesize[0] + x] = x + y + 3 * i;
            }
        }
        //u/v
        for (int y = 0; y < codecCtx->height / 2; y++)
        {
            for (int x = 0; x < codecCtx->width / 2; x++)
            {
                frame->data[1][y * frame->linesize[1] + x] = 128 + y + 2 * i;
                frame->data[2][y * frame->linesize[2] + x] = 64 + x + 5 * i;
            }
        }

        frame->pts = i;

        encode_mux_fun_3(codecCtx, frame, pkt, ofmtCtx, oStream);
    }
    encode_mux_fun_3(codecCtx, nullptr, pkt, ofmtCtx, oStream);
    av_write_trailer(ofmtCtx);

    av_frame_unref(frame);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avio_closep(&ofmtCtx->pb);
    avformat_free_context(ofmtCtx);
    avcodec_free_context(&codecCtx);
}

void Widget::encode_mux_fun_3(AVCodecContext* codecCtx, AVFrame* frame, AVPacket* pkt, AVFormatContext* oFmtCtx, AVStream* oStream)
{
    int ret = avcodec_send_frame(codecCtx, frame);

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(codecCtx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            av_packet_unref(pkt);
            break;
        }
        else if (ret < 0)
        {
            std::cout << "编码错误: " << ret << std::endl;
            exit(-1);
        }

        av_packet_rescale_ts(pkt, codecCtx->time_base, oStream->time_base);

        av_interleaved_write_frame(oFmtCtx, pkt);
        av_packet_unref(pkt);
    }
}
