#pragma once

#include "common.h"

void read_stream_info();
void read_packet();


/*********/
void create_rgb_frame();
void read_frame();
void save_as_ppm(AVFrame* rgb_frame, int width, int height, int frame_index);
/*********/


void exract_audio();
void exercise_extract_audio();
void extract_video();
//音视频转封装
void av_convert_wrap();
//音视频裁剪
void av_cut();
//视频编码
void encode_fun(AVCodecContext* codecCtx, AVFrame* frm, AVPacket* pkt, std::fstream* fs);
void encode_video();
//视频裸流编码练习
void exercise_encode_video();
void exercise_encode_fun(AVCodecContext* codecCtx, AVFrame* frm, AVPacket* pkt, std::fstream* fs);
//视频裸流编码封装
void encode_video_mux();
void encode_mux_fun(AVCodecContext* codecCtx, AVFrame* frame, AVPacket* pkt, AVFormatContext* oFmtCtx, AVStream* oStream);

//音频编码AAC
void encode_audio();
//检查采样格式是否支持
bool check_sample_fmt(const AVCodec* codec, AVSampleFormat fmt);
//获取最合适采样率
int get_best_sample_rate(const AVCodec* codec);
//编码
void encode_audio_fun(AVFormatContext* fmtCtx, AVCodecContext* codecCtx, AVFrame* frame, AVStream* stream);

//视频解码生成图片
void decode_video_to_pic();
void decode_fun(AVCodecContext* codecCtx, AVPacket* pkt, const char* name);