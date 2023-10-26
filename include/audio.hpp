#ifndef AUDIO_H
#define AUDIO_H

#include <iostream>

#include "options.hpp"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
}

namespace Vid2ASCII
{
    namespace AudioPlayer
    {
        struct AudioInfo
        {
            const AVCodec *decoder;
            AVFormatContext *format_ctx;
            AVCodecContext *codec_ctx;
            AVStream *stream;
            AVPacket *packet;
            AVFrame *frame;
            SwrContext *swr;
            AVAudioFifo *buffer;
        };

        std::string get_audio_stream(std::string);
        std::string open_file(Options);
        std::string decode_file();
        void play_file();
    }
}

#endif