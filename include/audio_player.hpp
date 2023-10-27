#ifndef AUDIO_H
#define AUDIO_H

#include <iostream>

#include "media.hpp"
#include "options.hpp"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
}

namespace TermVideo
{
    struct AudioInfo
    {
        AVFormatContext *format_ctx;
        AVStream *stream;
        AVAudioFifo *buffer;
    };

    class AudioPlayer
    {
    private:
        AudioInfo audio_info;
        std::string get_audio_stream(std::string);
        std::string get_decoder(const AVCodec **, AVCodecContext **, SwrContext **);
        std::string write_packet_to_buffer(AVCodecContext *, SwrContext *, AVPacket *, AVFrame *);

    public:
        AudioPlayer(MediaInfo);
        std::string decode_file(Options);
        void play_file();
    };
}

#endif