#include "audio_player.hpp"
#include "media.hpp"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

namespace TermVideo
{
    void data_callback(ma_device *p_device, void *p_output, const void *p_input, ma_uint32 frame_count)
    {
        AVAudioFifo *fifo = (AVAudioFifo *)p_device->pUserData;
        av_audio_fifo_read(fifo, &p_output, frame_count);
    }

    AudioPlayer::AudioPlayer()
    {
        this->audio_info.format_ctx = nullptr;
        this->audio_info.buffer = NULL;
    }

    std::string AudioPlayer::get_audio_stream(std::string audio_language)
    {
        // Find preferred audio stream language if exists
        if (audio_language.length() > 0)
        {
            for (size_t i = 0; i < this->audio_info.format_ctx->nb_streams; ++i)
            {
                AVStream *stream = this->audio_info.format_ctx->streams[i];
                if (stream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO)
                    continue;

                AVDictionaryEntry *lang = av_dict_get(stream->metadata, "language", NULL, 0);
                if (!strncmp(lang->value, audio_language.c_str(), 3))
                {
                    this->audio_info.stream = this->audio_info.format_ctx->streams[i];
                    return "";
                }
            }
        }

        // Fall back to best stream available
        int stream_index = av_find_best_stream(
            this->audio_info.format_ctx,
            AVMEDIA_TYPE_AUDIO,
            -1,
            -1,
            nullptr,
            0);
        if (stream_index < 0)
            return "No audio streams found in file!";

        this->audio_info.stream = this->audio_info.format_ctx->streams[stream_index];
        return "";
    }

    std::string AudioPlayer::open_file(Options opts)
    {
        int ret = avformat_open_input(
            &this->audio_info.format_ctx,
            opts.filename.c_str(),
            nullptr,
            nullptr);
        if (ret < 0)
            return "Unable to open media file!";

        ret = avformat_find_stream_info(this->audio_info.format_ctx, nullptr);
        if (ret < 0)
            return "Unable to find stream info!";

        std::string res = this->decode_file(opts);
        if (res.length() > 0)
            return res;

        return "";
    }

    std::string AudioPlayer::decode_file(Options opts)
    {
        int ret;
        std::string err;

        const AVCodec *decoder;
        AVCodecContext *codec_ctx;
        SwrContext *swr_ctx = swr_alloc();
        AVPacket *packet = av_packet_alloc();
        AVFrame *frame = av_frame_alloc();

        err = get_audio_stream(opts.audio_language);
        if (err.length() > 0)
            return err;

        err = this->get_decoder(&decoder, &codec_ctx, &swr_ctx);
        if (err.length() > 0)
            return err;

        this->audio_info.buffer = av_audio_fifo_alloc(
            AV_SAMPLE_FMT_FLT,
            this->audio_info.stream->codecpar->ch_layout.nb_channels,
            1);

        while (!av_read_frame(this->audio_info.format_ctx, packet))
        {
            err = this->write_packet_to_buffer(codec_ctx, swr_ctx, packet, frame);
            if (err.length() > 0)
                return err;
        }

        av_packet_free(&packet);
        av_frame_free(&frame);
        swr_free(&swr_ctx);
        avcodec_free_context(&codec_ctx);

        return "";
    }

    std::string AudioPlayer::get_decoder(const AVCodec **decoder, AVCodecContext **codec_ctx, SwrContext **swr_ctx)
    {
        *decoder = avcodec_find_decoder(this->audio_info.stream->codecpar->codec_id);
        if (!decoder)
            return "No appropriate decoder found for file!";

        *codec_ctx = avcodec_alloc_context3(*decoder);
        avcodec_parameters_to_context(*codec_ctx, this->audio_info.stream->codecpar);

        int ret = avcodec_open2(*codec_ctx, *decoder, nullptr);
        if (ret < 0)
            return "Decoder could not be opened\n";

        ret = swr_alloc_set_opts2(
            swr_ctx,
            &this->audio_info.stream->codecpar->ch_layout,
            AV_SAMPLE_FMT_FLT,
            this->audio_info.stream->codecpar->sample_rate,
            &this->audio_info.stream->codecpar->ch_layout,
            (AVSampleFormat)this->audio_info.stream->codecpar->format,
            this->audio_info.stream->codecpar->sample_rate,
            0,
            nullptr);
        if (ret < 0)
            return "Error setting up resampler";

        return "";
    }

    std::string AudioPlayer::write_packet_to_buffer(AVCodecContext *codec_ctx, SwrContext *swr_ctx, AVPacket *packet, AVFrame *frame)
    {
        // If not reading audio stream, skip
        if (packet->stream_index != this->audio_info.stream->index)
            return "";

        int ret = avcodec_send_packet(codec_ctx, packet);
        if (ret < 0)
        {
            if (ret != AVERROR(EAGAIN))
                return "Error with decoding packet";
        }
        while (!avcodec_receive_frame(codec_ctx, frame))
        {
            // Resample frame
            AVFrame *resampled_frame = av_frame_alloc();
            resampled_frame->sample_rate = frame->sample_rate;
            resampled_frame->ch_layout = frame->ch_layout;
            resampled_frame->format = AV_SAMPLE_FMT_FLT;

            ret = swr_convert_frame(swr_ctx, resampled_frame, frame);
            av_frame_unref(frame);
            av_audio_fifo_write(this->audio_info.buffer,
                                (void **)resampled_frame->data,
                                resampled_frame->nb_samples);
            av_frame_unref(resampled_frame);
        }

        return "";
    }

    void AudioPlayer::play_file()
    {
        ma_device_config device_config;
        ma_device device;

        device_config = ma_device_config_init(ma_device_type_playback);
        device_config.playback.format = ma_format_f32;
        device_config.playback.channels = this->audio_info.stream->codecpar->ch_layout.nb_channels;
        device_config.sampleRate = this->audio_info.stream->codecpar->sample_rate;
        device_config.dataCallback = data_callback;
        device_config.pUserData = this->audio_info.buffer;

        avformat_close_input(&this->audio_info.format_ctx);

        if (ma_device_init(NULL, &device_config, &device) != MA_SUCCESS)
            return;

        if (ma_device_start(&device) != MA_SUCCESS)
        {
            ma_device_uninit(&device);
            return;
        }

        while (av_audio_fifo_size(this->audio_info.buffer))
        {
        }

        av_audio_fifo_free(this->audio_info.buffer);
        ma_device_uninit(&device);
    }
}