#pragma once

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

#include <iostream>
#include <string>
#include <fstream>
#include <libavutil/frame.h>

void read_stream_info();
void read_packet();
void create_rgb_frame();
void read_frame();
void save_as_ppm(AVFrame* rgb_frame, int width, int height, int frame_index);