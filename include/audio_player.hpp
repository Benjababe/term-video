#ifndef AUDIO_H
#define AUDIO_H

#include <iostream>

#include "media.hpp"
#include "options.hpp"

extern "C"
{
#include <ao/ao.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
}

#define SAMPLE_FORMAT AV_SAMPLE_FMT_S32
#define SAMPLE_BITS 32

namespace TermVideo
{
    struct AudioInfo
    {
        AVFormatContext *format_ctx;
        AVCodecContext *codec_ctx;
        AVStream *stream;
        SwrContext *swr_ctx;
    };

    class AudioPlayer
    {
    private:
        AudioInfo audio_info;
        ao_sample_format ao_s_format;
        ao_device *a_device;

        std::string get_audio_stream(std::string);
        std::string get_decoder(const AVCodec **);
        std::string decode_file(Options);
        void init_output_device();

    public:
        AudioPlayer();
        ~AudioPlayer();
        std::string init_player(Options);
        void play_file();
    };
}

#endif