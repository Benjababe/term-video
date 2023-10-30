#include "audio_player.hpp"
#include "media.hpp"

namespace TermVideo
{
    AudioPlayer::AudioPlayer()
    {
        this->audio_info.format_ctx = nullptr;
    }

    AudioPlayer::~AudioPlayer()
    {
        ao_shutdown();
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

    std::string AudioPlayer::init_player(Options opts)
    {
        this->use_audio = opts.use_audio;
        if (!this->use_audio)
            return "";

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
        this->audio_info.swr_ctx = swr_alloc();

        err = get_audio_stream(opts.audio_language);
        if (err.length() > 0)
            return err;

        err = this->get_decoder(&decoder);
        if (err.length() > 0)
            return err;

        this->init_output_device();

        return "";
    }

    void AudioPlayer::init_output_device()
    {
        ao_initialize();
        int driver_id = ao_default_driver_id();

        this->ao_s_format.bits = SAMPLE_BITS;
        this->ao_s_format.byte_format = AO_FMT_NATIVE;
        this->ao_s_format.matrix = nullptr;
        this->ao_s_format.channels = this->audio_info.codec_ctx->ch_layout.nb_channels;
        this->ao_s_format.rate = this->audio_info.codec_ctx->sample_rate;

        this->a_device = ao_open_live(driver_id, &this->ao_s_format, NULL);
    }

    std::string AudioPlayer::get_decoder(const AVCodec **decoder)
    {
        *decoder = avcodec_find_decoder(this->audio_info.stream->codecpar->codec_id);
        if (!decoder)
            return "No appropriate decoder found for file!";

        this->audio_info.codec_ctx = avcodec_alloc_context3(*decoder);
        avcodec_parameters_to_context(this->audio_info.codec_ctx, this->audio_info.stream->codecpar);

        int ret = avcodec_open2(this->audio_info.codec_ctx, *decoder, nullptr);
        if (ret < 0)
            return "Decoder could not be opened\n";

        ret = swr_alloc_set_opts2(
            &this->audio_info.swr_ctx,
            &this->audio_info.stream->codecpar->ch_layout,
            SAMPLE_FORMAT,
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

    void AudioPlayer::play_file()
    {
        if (!this->use_audio)
            return;

        AVPacket *packet = av_packet_alloc();
        AVFrame *frame = av_frame_alloc();

        while (!av_read_frame(this->audio_info.format_ctx, packet))
        {
            if (packet->stream_index != this->audio_info.stream->index)
            {
                av_packet_unref(packet);
                continue;
            }

            int ret = avcodec_send_packet(this->audio_info.codec_ctx, packet);
            if (ret < 0 && ret != AVERROR(EAGAIN))
            {
                av_packet_unref(packet);
                continue;
            }

            while (!avcodec_receive_frame(this->audio_info.codec_ctx, frame))
            {
                // Resample frame
                AVFrame *resampled_frame = av_frame_alloc();
                resampled_frame->sample_rate = frame->sample_rate;
                resampled_frame->ch_layout = frame->ch_layout;
                resampled_frame->format = SAMPLE_FORMAT;

                ret = swr_convert_frame(this->audio_info.swr_ctx, resampled_frame, frame);

                int buf_size = av_samples_get_buffer_size(nullptr,
                                                          this->audio_info.codec_ctx->ch_layout.nb_channels,
                                                          resampled_frame->nb_samples,
                                                          this->audio_info.codec_ctx->sample_fmt,
                                                          1);

                ao_play(this->a_device,
                        (char *)resampled_frame->extended_data[0],
                        buf_size);

                av_frame_unref(resampled_frame);
                av_frame_unref(frame);
            }

            av_packet_unref(packet);
        }

        av_packet_free(&packet);
    }
}