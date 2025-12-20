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

void exract_audio()
{
    const char* filePath = "res/test.mp4";

    int ret = -1;
    AVFormatContext* fmt_ctx = nullptr;
    int audio_index = -1;
    AVStream* audio_stream = nullptr;
    AVPacket packet;

    AVFormatContext* out_fmt_ctx = nullptr;
    int out_audio_index = 0;
    AVStream* out_stream = nullptr;

    ret = avformat_open_input(&fmt_ctx, filePath, nullptr, nullptr);
    ret = avformat_find_stream_info(fmt_ctx, nullptr);

    audio_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    audio_stream = fmt_ctx->streams[audio_index];

    //创建输出格式上下文（容器）
    //参数0，1，2，3，1>23，2>3
    char* fmt_name = nullptr;
    char* file_name = nullptr;

    switch (audio_stream->codecpar->codec_id)
    {
    case AVCodecID::AV_CODEC_ID_AAC:
        fmt_name = "mp4";//容器格式
        file_name = "output_audio.m4a";
        break;
    case AVCodecID::AV_CODEC_ID_MP3:
        break;
    }
    if (!fmt_name || !file_name)
    {
        std::cout << "源音频格式: " << audio_stream->codecpar->codec_id << std::endl;
        avformat_close_input(&fmt_ctx);
        return;
    }
    ret = avformat_alloc_output_context2(&out_fmt_ctx, nullptr, fmt_name, file_name);

    //创建流
    out_stream = avformat_new_stream(out_fmt_ctx, nullptr);
    //复制编解码参数
    ret = avcodec_parameters_copy(out_stream->codecpar, audio_stream->codecpar);
    //自动选择tag
    out_stream->codecpar->codec_tag = 0;

    //本地文件
    if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        //打开输出文件
        ret = avio_open(&out_fmt_ctx->pb, file_name, AVIO_FLAG_WRITE);
        //写头
        ret = avformat_write_header(out_fmt_ctx, nullptr);
        //初始化包
        av_init_packet(&packet);
        //读取
        while (av_read_frame(fmt_ctx, &packet) >= 0)
        {
            if (packet.stream_index != audio_index)
            {
                av_packet_unref(&packet);
                continue;
            }

            //调整时间戳等
            packet.pts = av_rescale_q(packet.pts, audio_stream->time_base, out_stream->time_base);
            packet.dts = packet.pts; //音频相等
            packet.duration = av_rescale_q(packet.duration, audio_stream->time_base, out_stream->time_base);

            packet.pos = -1;
            packet.stream_index = out_audio_index;//0

            //写入
            ret = av_interleaved_write_frame(out_fmt_ctx, &packet);

            av_packet_unref(&packet);
        }
        //写尾
        av_write_trailer(out_fmt_ctx);

        avio_closep(&out_fmt_ctx->pb);
    }


    avformat_free_context(out_fmt_ctx);
    avformat_close_input(&fmt_ctx);
}

void exercise_extract_audio()
{
    const char* inFilePath = "res/test.mp4";

    int ret = -1;

    AVFormatContext* inFmtCtx = nullptr;
    AVStream* inStream = nullptr;
    int inIndex = -1;

    AVFormatContext* outFmtCtx = nullptr;
    AVStream* outStream = nullptr;
    
    //打开容器
    ret = avformat_open_input(&inFmtCtx, inFilePath, nullptr, nullptr);
    ret = avformat_find_stream_info(inFmtCtx, nullptr);

    //查找音频流
    inIndex = av_find_best_stream(inFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    inStream = inFmtCtx->streams[inIndex];

    //创建输出容器
    const char* fmt_name = nullptr;
    const char* fileName = nullptr;

    //const AVOutputFormat* outFmt = av_guess_format(inFmtCtx->oformat->name, nullptr, nullptr);
    ret = avformat_query_codec(inFmtCtx->oformat, inStream->codecpar->codec_id, FF_COMPLIANCE_NORMAL);

    switch (inStream->codecpar->codec_id)
    {
    case AV_CODEC_ID_AAC:
        fmt_name = "mp4";//容器名称
        fileName = "output_audio.m4a";
        break;
    }
    if (!fmt_name || !fileName)
    {
        std::cout << "源音频编码: " << inStream->codecpar->codec_id << std::endl;
        avformat_close_input(&inFmtCtx);
        return;
    }
    ret = avformat_alloc_output_context2(&outFmtCtx, nullptr, fmt_name, fileName);

    //创建输出音频流
    outStream = avformat_new_stream(outFmtCtx, nullptr);
    //复制编解码参数
    ret = avcodec_parameters_copy(outStream->codecpar, inStream->codecpar);
    //调整tag
    outStream->codecpar->codec_tag = 0;

    //打开输出文件
    ret = avio_open2(&outFmtCtx->pb, fileName, AVIO_FLAG_WRITE, nullptr, nullptr);

    //写头
    ret = avformat_write_header(outFmtCtx, nullptr);

    //读取
    AVPacket packet;
    av_init_packet(&packet);
    while (av_read_frame(inFmtCtx, &packet) >= 0)
    {
        if (packet.stream_index != inIndex)
        {
            av_packet_unref(&packet);
            continue;
        }

        //调整时间戳等
        packet.pts = av_rescale_q(packet.pts, inStream->time_base, outStream->time_base);
        packet.dts = packet.pts;
        packet.duration = av_rescale_q(packet.duration, inStream->time_base, outStream->time_base);
        packet.pos = -1;
        packet.stream_index = 0;
        
        //写入
        ret = av_interleaved_write_frame(outFmtCtx, &packet);

        av_packet_unref(&packet);
    }
    //写尾
    ret = av_write_trailer(outFmtCtx);

    //关闭释放
    ret = avio_closep(&outFmtCtx->pb);
    avformat_free_context(outFmtCtx);
    avformat_close_input(&inFmtCtx);
}

void extract_video()
{
    const char* inFilePath = "res/test.mp4";

    int ret = -1;

    AVFormatContext* inFmtCtx = nullptr;
    AVStream* inStream = nullptr;
    int inIndex = -1;

    AVFormatContext* outFmtCtx = nullptr;
    AVStream* outStream = nullptr;

    //打开容器
    ret = avformat_open_input(&inFmtCtx, inFilePath, nullptr, nullptr);
    ret = avformat_find_stream_info(inFmtCtx, nullptr);

    //查找音频流
    inIndex = av_find_best_stream(inFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    inStream = inFmtCtx->streams[inIndex];

    //创建输出容器
    const char* fmt_name = nullptr;
    const char* fileName = nullptr;

    //const AVOutputFormat* outFmt = av_guess_format(inFmtCtx->oformat->name, nullptr, nullptr);
    //ret = avformat_query_codec(inFmtCtx->oformat, inStream->codecpar->codec_id, FF_COMPLIANCE_NORMAL);

    switch (inStream->codecpar->codec_id)
    {
    case AV_CODEC_ID_AAC:
        fmt_name = "mp4";//容器名称
        fileName = "output_audio.m4a";
        break;
    case AV_CODEC_ID_H264:
        fmt_name = "mp4";//容器名称
        fileName = "output_audio.mp4";
        break;
    }
    if (!fmt_name || !fileName)
    {
        std::cout << "源音频编码: " << inStream->codecpar->codec_id << std::endl;
        avformat_close_input(&inFmtCtx);
        return;
    }
    ret = avformat_alloc_output_context2(&outFmtCtx, nullptr, fmt_name, fileName);
  
    //创建输出音频流
    outStream = avformat_new_stream(outFmtCtx, nullptr);
    //复制编解码参数
    ret = avcodec_parameters_copy(outStream->codecpar, inStream->codecpar);
    //调整tag
    outStream->codecpar->codec_tag = 0;

    //打开输出文件
    ret = avio_open2(&outFmtCtx->pb, fileName, AVIO_FLAG_WRITE, nullptr, nullptr);

    //写头
    ret = avformat_write_header(outFmtCtx, nullptr);

    //读取
    AVPacket packet;
    av_init_packet(&packet);
    while (av_read_frame(inFmtCtx, &packet) >= 0)
    {
        if (packet.stream_index != inIndex)
        {
            av_packet_unref(&packet);
            continue;
        }

        //调整时间戳等
        packet.pts = av_rescale_q(packet.pts, inStream->time_base, outStream->time_base);
        packet.dts = av_rescale_q(packet.dts, inStream->time_base, outStream->time_base);
        packet.duration = av_rescale_q(packet.duration, inStream->time_base, outStream->time_base);
        packet.pos = -1;
        packet.stream_index = 0;

        //写入
        ret = av_interleaved_write_frame(outFmtCtx, &packet);

        av_packet_unref(&packet);
    }
    //写尾
    ret = av_write_trailer(outFmtCtx);

    //关闭释放
    ret = avio_closep(&outFmtCtx->pb);
    avformat_free_context(outFmtCtx);
    avformat_close_input(&inFmtCtx);
}

void av_convert_wrap()
{
    av_log_set_level(AV_LOG_DEBUG);

    //mp4(H264/AAC) > mov
    const char* inFile = "res/test.mp4";
    const char* outFile = "output_av.mov";

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
        {
            av_packet_unref(&pkt);
            continue;
        }

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

void av_cut()
{
    const char* inFile = "res/b.mp4";
    const char* outFile = "output_cut.mov";

    int64_t start_time = 5;
    int64_t end_time = 10;

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
}

void encode_fun(AVCodecContext* codecCtx, AVFrame* frm, AVPacket* pkt, std::fstream* fs)
{
   int ret = avcodec_send_frame(codecCtx, frm);

   while (ret >= 0)
   {
       ret = avcodec_receive_packet(codecCtx, pkt);
       if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
           break; // 当前帧处理完或结束
       }
       else if (ret < 0) {
           // 真正的错误
           break;
       }

       // 写入数据
       fs->write(reinterpret_cast<const char*>(pkt->data), pkt->size);
       av_packet_unref(pkt);
   }
}

void encode_video()
{
    const char* outFile = "output_encode.h264";
    const char* encoderName = "libx264";

    int ret = -1;
    const AVCodec* codec = avcodec_find_encoder_by_name(encoderName);
    std::fstream fs;
    
    AVCodecContext* codecCtx = avcodec_alloc_context3(codec);
    //设置编码器上下文属性
    codecCtx->width = 1280;
    codecCtx->height = 960;
    codecCtx->bit_rate = 500000;
    
    codecCtx->gop_size = 10;
    codecCtx->time_base = { 1, 25 };
    codecCtx->framerate = { 25, 1 };

    codecCtx->max_b_frames = 1;
    codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

    if (codecCtx->codec_id == AV_CODEC_ID_H264)
        av_opt_set(codecCtx->priv_data, "preset", "slow", 0);

    //打开编码器
    ret = avcodec_open2(codecCtx, codec, nullptr);

    //打开输出文件
    fs.open(outFile, std::ios::out | std::ios::binary);

    //创建frame
    AVFrame* frame = av_frame_alloc();
    //设置frame属性
    frame->width = codecCtx->width;
    frame->height = codecCtx->height;
    frame->format = codecCtx->pix_fmt;
    
    //分配空间
    ret = av_frame_get_buffer(frame, 0);

    //创建packet
    AVPacket* pkt = av_packet_alloc();

    //生成视频内容
    //yuv420p
    //25帧
// 1. 修正循环变量冲突
    for (int i = 0; i < 25; i++)
    {
        ret = av_frame_make_writable(frame);

        // Y 分量
        for (int y = 0; y < codecCtx->height; y++) {
            for (int x = 0; x < codecCtx->width; x++) {
                frame->data[0][y * frame->linesize[0] + x] = y + x + i * 3;
            }
        }
        // U/V 分量 (注意范围)
        for (int y = 0; y < codecCtx->height / 2; y++) {
            for (int x = 0; x < codecCtx->width / 2; x++) {
                frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
                frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
            }
        }

        frame->pts = i;
        encode_fun(codecCtx, frame, pkt, &fs);
    }

    //刷新编码器
    encode_fun(codecCtx, nullptr, pkt, &fs);


    av_packet_free(&pkt);
    av_frame_free(&frame);
    avcodec_free_context(&codecCtx);
    fs.close();
}

void exercise_encode_video()
{
    const char* fileName = "output_exercise_encode_video.h264";
    const char* encoderName = "libx264";

    int ret = -1;

    //找编码器
    const AVCodec* codec = avcodec_find_encoder_by_name(encoderName);
    //编解码上下文
    AVCodecContext* codecCtx = avcodec_alloc_context3(codec);
    //设置参数
    codecCtx->width = 1280;
    codecCtx->height = 680;
    codecCtx->bit_rate = 500000;

    codecCtx->gop_size = 10;//10帧一个i帧
    codecCtx->max_b_frames = 1;//b帧
    codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

    codecCtx->time_base = { 1,25 };
    codecCtx->framerate = { 25, 1 };

    if (codecCtx->codec_id == AV_CODEC_ID_H264)
        ret = av_opt_set(codecCtx->priv_data, "preset", "slow", 0);

    //打开编码器
    ret = avcodec_open2(codecCtx, codec, nullptr);

    //打开输出文件
    std::fstream fs;
    fs.open(fileName, std::ios::out | std::ios::binary);

    //frame
    AVFrame* frm = av_frame_alloc();

    //设置frame参数
    frm->width = codecCtx->width;
    frm->height = codecCtx->height;
    frm->format = codecCtx->pix_fmt;
    //申请data空间
    ret = av_frame_get_buffer(frm, 0);

    //packet
    AVPacket* pkt = av_packet_alloc();

    //写帧
    for (int i = 0; i < 30; i++)
    {
        //确保可写
        ret = av_frame_make_writable(frm);
        
        //y
        for (int y = 0; y < codecCtx->height; y++)
        {
            for (int x = 0; x < codecCtx->width; x++)
            {
                frm->data[0][x + y * frm->linesize[0]] = x + y + i * 3;
            }
        }
        //u/v
        for (int y = 0; y < codecCtx->height / 2; y++)
        {
            for (int x = 0; x < codecCtx->width / 2; x++)
            {
                frm->data[1][x + y * frm->linesize[1]] = 128 + y + i * 2;
                frm->data[2][x + y * frm->linesize[2]] = 64 + x + i * 5;
            }
        }

        frm->pts = i;

        //编码
        exercise_encode_fun(codecCtx, frm, pkt, &fs);
    }
    //刷新
    exercise_encode_fun(codecCtx, nullptr, pkt, &fs);
    //解引用
    av_frame_unref(frm);

    //清理关闭
    fs.close();
    av_packet_free(&pkt);
    av_frame_free(&frm);
    avcodec_free_context(&codecCtx);
}

void exercise_encode_fun(AVCodecContext* codecCtx, AVFrame* frm, AVPacket* pkt, std::fstream* fs)
{
    //发送数据帧给编码器
    int ret = avcodec_send_frame(codecCtx, frm);

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(codecCtx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            //结束
            av_packet_unref(pkt);
            break;
        }
        else if (ret < 0)
        {
            //错误
            exit(-1);
        }

        //写文件
        fs->write(reinterpret_cast<const char*>(pkt->data), pkt->size);

        av_packet_unref(pkt);
    }
}

void encode_video_mux()
{
    const char* fileName = "output_encode_mux.mp4";
    const char* encoderName = "libx264";

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

        encode_mux_fun(codecCtx, frame, pkt, ofmtCtx, oStream);
    }
    encode_mux_fun(codecCtx, nullptr, pkt, ofmtCtx, oStream);
    av_write_trailer(ofmtCtx);

    av_frame_unref(frame);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avio_closep(&ofmtCtx->pb);
    avformat_free_context(ofmtCtx);
    avcodec_free_context(&codecCtx);
}

void encode_mux_fun(AVCodecContext* codecCtx, AVFrame* frame, AVPacket* pkt, AVFormatContext* oFmtCtx, AVStream* oStream)
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
