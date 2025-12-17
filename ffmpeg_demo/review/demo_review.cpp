#include "demo_review.h"

void read_stream_info_251215()
{
    const std::string filePath = "res/test.mp4";

    AVFormatContext* fmt_ctx = nullptr;

    // 打开文件
    if (avformat_open_input(&fmt_ctx, filePath.c_str(), nullptr, nullptr) < 0)
    {
        std::cout << "avformat_open_input失败" << std::endl;
        return;
    }

    //文件信息
    std::cout << "文件格式: " << fmt_ctx->iformat->long_name << std::endl;
    std::cout << "时长: " << fmt_ctx->duration / AV_TIME_BASE << std::endl;
    std::cout << "流数: " << fmt_ctx->nb_streams << std::endl;

    //获取流信息
    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0)
    {
        avformat_close_input(&fmt_ctx);
        std::cout << "avformat_find_stream_info失败" << std::endl;
        return;
    }
    //dump流详细信息
    for (int i = 0; i < fmt_ctx->nb_streams; i++)
    {
        av_dump_format(fmt_ctx, i, filePath.c_str(), 0);
    }

    std::cout << "traverse stream info" << std::endl;
    //遍历流
    for (int i = 0; i < fmt_ctx->nb_streams; i++)
    {
        AVStream* stream = fmt_ctx->streams[i];
        AVCodecParameters* codecPar = stream->codecpar;

        //视频
        if (codecPar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            std::cout << "vedio stream " << i << std::endl;
            const AVCodec* codec = avcodec_find_decoder(codecPar->codec_id);
            if (codec)
            {
                std::cout << "codec name: " << codec->long_name << std::endl;
            }
        }
        //音频
        else if (codecPar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            std::cout << "audio stream " << i << std::endl;
            std::cout << "audio channel amount: " << codecPar->ch_layout.nb_channels << std::endl;

            char buf[256];
            if (av_channel_layout_describe(&codecPar->ch_layout, buf, sizeof(buf)) > 0)
            {
                std::cout << "audio channel layout: " << buf << std::endl;
            }
        }
    }

    avformat_close_input(&fmt_ctx);
}

void read_packet_251215()
{
    const std::string filePath = "res/test.mp4";

    AVFormatContext* fmt_ctx = nullptr;

    if (avformat_open_input(&fmt_ctx, filePath.c_str(), nullptr, nullptr) < 0)
    {
        std::cout << "avformat_open_input failed" << std::endl;
        return;
    }

    int vs_index = -1;
    for (int i = 0; i < fmt_ctx->nb_streams; i++)
    {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            vs_index = i;
            break;
        }
    }
    if (vs_index < 0)
    {
        std::cout << "no video stream" << std::endl;
        avformat_close_input(&fmt_ctx);
        return;
    }

    AVStream* video_stream = fmt_ctx->streams[vs_index];
    AVCodecParameters* codecPar = video_stream->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecPar->codec_id);
    if (!codec)
    {
        std::cout << "未找到解码器: " << codecPar->codec_id << std::endl;
        avformat_close_input(&fmt_ctx);
        return;
    }
    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    //复制参数
    avcodec_parameters_to_context(codec_ctx, codecPar);
    //打开解码器上下文
    avcodec_open2(codec_ctx, codec, nullptr);

    //读数据包
    AVPacket packet;
    for (int i = 0; i < 5; i++)
    {
        if (av_read_frame(fmt_ctx, &packet) < 0)
            break;

        if (packet.stream_index != vs_index)
            continue;

        //视频包
        if (packet.flags & AV_PKT_FLAG_KEY)
            std::cout << "key frame" << std::endl;
        std::cout << "packet size: " << packet.size << std::endl;
        std::cout << "packet pos: " << packet.pts * av_q2d(video_stream->time_base) << std::endl;
        std::cout << "packet duration: " << packet.duration << std::endl;

        av_packet_unref(&packet);
    }

    avcodec_free_context(&codec_ctx);
    avformat_close_input(&fmt_ctx);
}

void extract_audio_251217()
{
    const char* inFile = "res/test.mp4";
    const char* outFile = "output_audio.m4a";

    int ret = 0;
    int inAudioIndex = -1;

    //输入
    AVFormatContext* inFmtCtx = nullptr;
    AVStream* inAudioStream = nullptr;

    //输出
    AVFormatContext* outFmtCtx = nullptr;
    AVStream* outAudioStream = nullptr;

    ret = avformat_open_input(&inFmtCtx, inFile, nullptr, nullptr);
    ret = avformat_find_stream_info(inFmtCtx, nullptr);
    inAudioIndex = av_find_best_stream(inFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    inAudioStream = inFmtCtx->streams[inAudioIndex];

    ret = avformat_alloc_output_context2(&outFmtCtx, nullptr, nullptr, outFile);
    outAudioStream = avformat_new_stream(outFmtCtx, nullptr);
    ret = avcodec_parameters_copy(outAudioStream->codecpar, inAudioStream->codecpar);
    outAudioStream->codecpar->codec_tag = 0;

    ret = avio_open2(&outFmtCtx->pb, outFile, AVIO_FLAG_WRITE, nullptr, nullptr);
    avformat_write_header(outFmtCtx, nullptr);

    AVPacket pkt;
    av_init_packet(&pkt);

    while (av_read_frame(inFmtCtx, &pkt) >= 0)
    {
        if (pkt.stream_index == inAudioIndex)
        {
            pkt.pts = av_rescale_q(pkt.pts, inAudioStream->time_base, outAudioStream->time_base);
            pkt.dts = pkt.pts;
            pkt.duration = av_rescale_q(pkt.duration, inAudioStream->time_base, outAudioStream->time_base);
            pkt.pos = -1;
            pkt.stream_index = 0;

            av_interleaved_write_frame(outFmtCtx, &pkt);
        }

        av_packet_unref(&pkt);
    }

    av_write_trailer(outFmtCtx);
     

    avio_closep(&outFmtCtx->pb);
    avformat_close_input(&inFmtCtx);
    avformat_free_context(outFmtCtx);
}
