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