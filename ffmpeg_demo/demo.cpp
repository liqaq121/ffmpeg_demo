#include "demo.h"

using namespace std;

void read_stream_info()
{
    const string filePath = "res/test.mp4";

    av_log_set_level(AV_LOG_INFO);

    AVFormatContext* fmt_ctx = nullptr;
    
    //打开
    if (avformat_open_input(&fmt_ctx, filePath.c_str(), nullptr, nullptr) < 0)
    {
        avformat_close_input(&fmt_ctx);
        cout << "无法打开文件" << endl;
        return;
    }

    //读取
    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0)
    {
        avformat_close_input(&fmt_ctx);
        cout << "无法读取流信息" << endl;
        return;
    }

    std::cout << "===== 文件信息 =====" << std::endl;
    std::cout << "文件名: " << filePath << std::endl;
    std::cout << "格式: " << (fmt_ctx->iformat->long_name ? fmt_ctx->iformat->long_name : fmt_ctx->iformat->name) << std::endl;
    std::cout << "时长: " << (fmt_ctx->duration != AV_NOPTS_VALUE ? fmt_ctx->duration / AV_TIME_BASE : 0) << "秒" << std::endl;
    std::cout << "流数量: " << fmt_ctx->nb_streams << std::endl;

    for (int i = 0; i < fmt_ctx->nb_streams; i++)
    {
        av_dump_format(fmt_ctx, i, filePath.c_str(), 0);
    }

    for (int i = 0; i < fmt_ctx->nb_streams; i++)
    {
        AVStream* stream = fmt_ctx->streams[i];
        AVCodecParameters* codecPar = stream->codecpar;

        switch (codecPar->codec_type)
        {
        case AVMEDIA_TYPE_AUDIO:
            cout << "音频采样率: " << codecPar->sample_rate << endl;

#if LIBAVUTIL_VERSION_MAJOR >= 57
// FFmpeg 5.0+ 使用新的声道布局系统
            char channel_layout[64];
            av_channel_layout_describe(&codecPar->ch_layout, channel_layout, sizeof(channel_layout));
            std::cout << "音频声道布局: " << channel_layout << endl;
            std::cout << "音频声道数: " << codecPar->ch_layout.nb_channels << endl;
#else
// 旧版本 FFmpeg
            std::cout << ", 声道数: " << codecParams->channels;
            if (codecParams->channel_layout) {
                char layout[64];
                av_get_channel_layout_string(layout, sizeof(layout), codecParams->channels, codecParams->channel_layout);
                std::cout << ", 声道布局: " << layout;
            }
#endif

            break;
        case AVMEDIA_TYPE_VIDEO:
            cout << "视频分辨率: " << codecPar->width << "x" << codecPar->height << endl;

            const AVCodec* codec = avcodec_find_decoder(codecPar->codec_id);
            if (codec)
            {
                cout << "解码器: " << codec->name << endl;
                //cout << "wrapper_name: " << codec->wrapper_name << endl;
                cout << "long_name: " << codec->long_name << endl;
            }
            break;
        }
    }

    avformat_close_input(&fmt_ctx);
}

void read_packet()
{
    const char* fileName = "res/test.mp4";
    AVFormatContext* fmt_ctx = nullptr;
    avformat_open_input(&fmt_ctx, fileName, nullptr, nullptr);

    int vs_index = -1;
    for (int i = 0; i < fmt_ctx->nb_streams; i++)
    {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            vs_index = i;
            break;
        }
    }

    AVStream* stream = fmt_ctx->streams[vs_index];
    AVCodecParameters* codecPar = stream->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecPar->codec_id);
    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_ctx, codecPar);
    avcodec_open2(codec_ctx, codec, nullptr);

    //cout << "像素格式: " << av_get_pix_fmt_name(codec_ctx->pix_fmt) << endl;

    AVPacket packet;
    for (int i = 0; i < 5; i++)
    {
        if (av_read_frame(fmt_ctx, &packet) < 0)
            break;

        if (packet.stream_index == vs_index)
        {
            if (packet.flags & AV_PKT_FLAG_KEY)
                cout << "关键帧" << endl;
            cout << "视频帧: " << packet.size << " " << packet.pts * av_q2d(stream->time_base) << " " << packet.duration << endl;
        }
        else
            cout << "音频帧" << endl;

        av_packet_unref(&packet);
    }
    
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&fmt_ctx);
}

void create_rgb_frame()
{
    const char* fileName = "res/test.mp4";

    AVFormatContext* fmt_ctx = nullptr;
    avformat_open_input(&fmt_ctx, fileName, nullptr, nullptr);
    //avformat_find_stream_info(fmt_ctx, nullptr);

    int vs_index = -1;
    for (int i = 0; i < fmt_ctx->nb_streams; i++)
    {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            vs_index = i;
            break;
        }
    }


    AVStream* v_stream = fmt_ctx->streams[vs_index];
    AVCodecParameters* codecPar = v_stream->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecPar->codec_id);
    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_ctx, codecPar);

    AVFrame* rgb_frame = av_frame_alloc();
    int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codec_ctx->width, codec_ctx->height, 1);
    uint8_t* buffer = (uint8_t*)av_malloc(buffer_size);
    //给帧分配内存
    av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, buffer, AV_PIX_FMT_RGB24, codec_ctx->width, codec_ctx->height, 1);

    av_free(buffer);
    av_frame_free(&rgb_frame);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&fmt_ctx);
}

void read_frame()
{
    av_log_set_level(AV_LOG_INFO);

    const char* fileName = "res/test.mp4";

    AVFormatContext* fmt_ctx = nullptr;

    //打开
    avformat_open_input(&fmt_ctx, fileName, nullptr, nullptr);
    //找流信息
    avformat_find_stream_info(fmt_ctx, nullptr);

    //找视频流
    int vs_index = -1;
    for (int i = 0; i < fmt_ctx->nb_streams; i++)
    {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            vs_index = i;
            break;
        }
    }
    
    AVStream* v_stream = fmt_ctx->streams[vs_index];
    AVCodecParameters* codecPar = v_stream->codecpar;

    //找解码器
    const AVCodec* codec = avcodec_find_decoder(codecPar->codec_id);
    //解码器上下文
    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    //复制解码器参数到上下文
    avcodec_parameters_to_context(codec_ctx, codecPar);
    //打开
    avcodec_open2(codec_ctx, codec, nullptr);

    AVFrame* rgb_frame = av_frame_alloc();
    //每帧图像大小
    int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codec_ctx->width, codec_ctx->height, 1);
    //申请缓冲区
    uint8_t* buffer = (uint8_t*)av_malloc(buffer_size);
    //应用到frame
    av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, buffer, AV_PIX_FMT_RGB24, codec_ctx->width, codec_ctx->height, 1);

    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    SwsContext* sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
        codec_ctx->width, codec_ctx->height, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    int frame_count = 0;
    while (av_read_frame(fmt_ctx, packet) >= 0)
    {
        if (packet->stream_index != vs_index)
            continue;

        avcodec_send_packet(codec_ctx, packet);
        while (avcodec_receive_frame(codec_ctx, frame) >= 0)
        {
            frame_count++;
            sws_scale(sws_ctx, frame->data, frame->linesize, 0, codec_ctx->height, rgb_frame->data, rgb_frame->linesize);

            //TODO: 保存为图片
            save_as_ppm(rgb_frame, codec_ctx->width, codec_ctx->height, frame_count);
        }

        av_packet_unref(packet);
    }

    av_packet_free(&packet);
    av_frame_free(&frame);
    av_frame_free(&rgb_frame);
    av_free(buffer);
    sws_freeContext(sws_ctx);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&fmt_ctx);
}

void save_as_ppm(AVFrame* rgb_frame, int width, int height, int frame_index)
{
    char filename[256];
    snprintf(filename, sizeof(filename), "frame_%04d.ppm", frame_index);

    FILE* file = fopen(filename, "wb");
    if (!file) return;

    fprintf(file, "P6\n%d %d\n255\n", width, height);
    for (int y = 0; y < height; y++) {
        fwrite(rgb_frame->data[0] + y * rgb_frame->linesize[0], 1, width * 3, file);
    }
    fclose(file);

    std::cout << "保存: " << filename << std::endl;
}

