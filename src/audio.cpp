#include <audio.hpp>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

Vid2ASCII::AudioPlayer::AudioInfo audioInfo;

void data_callback(ma_device *p_device, void *p_output, const void *p_input, ma_uint32 frame_count)
{
    AVAudioFifo *fifo = (AVAudioFifo *)p_device->pUserData;
    av_audio_fifo_read(fifo, &p_output, frame_count);
}

std::string Vid2ASCII::AudioPlayer::open_file(std::string file_path)
{
    audioInfo.packet = av_packet_alloc();
    audioInfo.frame = av_frame_alloc();
    audioInfo.buffer = NULL;

    int ret;

    ret = avformat_open_input(&audioInfo.format_ctx, file_path.c_str(), nullptr, nullptr);
    if (ret < 0)
        return "Unable to open media file!";

    ret = avformat_find_stream_info(audioInfo.format_ctx, nullptr);
    if (ret < 0)
        return "Unable to find stream info!";

    audioInfo.stream_index = av_find_best_stream(audioInfo.format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (audioInfo.stream_index < 0)
        return "No audio streams found in file!";

    audioInfo.stream = audioInfo.format_ctx->streams[audioInfo.stream_index];

    audioInfo.decoder = avcodec_find_decoder(audioInfo.stream->codecpar->codec_id);
    if (!audioInfo.decoder)
        return "No appropriate decoder found for file!";

    AVCodecContext *codec_ctx{avcodec_alloc_context3(audioInfo.decoder)};
    audioInfo.codec_ctx = codec_ctx;
    avcodec_parameters_to_context(codec_ctx, audioInfo.stream->codecpar);

    ret = avcodec_open2(codec_ctx, audioInfo.decoder, nullptr);
    if (ret < 0)
        return "Decoder could not be opened\n";

    std::string err = decode_file();
    if (err.length() > 0)
        return err;

    return "";
}

std::string Vid2ASCII::AudioPlayer::decode_file()
{
    int ret = swr_alloc_set_opts2(
        &audioInfo.swr,
        &audioInfo.stream->codecpar->ch_layout,
        AV_SAMPLE_FMT_FLT,
        audioInfo.stream->codecpar->sample_rate,
        &audioInfo.stream->codecpar->ch_layout,
        (AVSampleFormat)audioInfo.stream->codecpar->format,
        audioInfo.stream->codecpar->sample_rate,
        0,
        nullptr);
    if (ret < 0)
        return "Error setting up resampler";

    audioInfo.buffer = av_audio_fifo_alloc(
        AV_SAMPLE_FMT_FLT,
        audioInfo.stream->codecpar->ch_layout.nb_channels,
        1);

    while (!av_read_frame(audioInfo.format_ctx, audioInfo.packet))
    {
        // If not reading audio stream, skip
        if (audioInfo.packet->stream_index != audioInfo.stream->index)
            continue;

        ret = avcodec_send_packet(audioInfo.codec_ctx, audioInfo.packet);
        if (ret < 0)
        {
            if (ret != AVERROR(EAGAIN))
                return "Error with decoding packet";
        }
        while (!avcodec_receive_frame(audioInfo.codec_ctx, audioInfo.frame))
        {
            // Resample frame
            AVFrame *resampled_frame = av_frame_alloc();
            resampled_frame->sample_rate = audioInfo.frame->sample_rate;
            resampled_frame->ch_layout = audioInfo.frame->ch_layout;
            resampled_frame->format = AV_SAMPLE_FMT_FLT;

            ret = swr_convert_frame(audioInfo.swr, resampled_frame, audioInfo.frame);
            av_frame_unref(audioInfo.frame);
            av_audio_fifo_write(audioInfo.buffer, (void **)resampled_frame->data, resampled_frame->nb_samples);
            av_frame_unref(resampled_frame);
        }
    }

    av_packet_free(&audioInfo.packet);
    av_frame_free(&audioInfo.frame);
    avcodec_free_context(&audioInfo.codec_ctx);
    swr_free(&audioInfo.swr);

    return "";
}

void Vid2ASCII::AudioPlayer::play_file()
{
    ma_device_config device_config;
    ma_device device;

    device_config = ma_device_config_init(ma_device_type_playback);
    device_config.playback.format = ma_format_f32;
    device_config.playback.channels = audioInfo.stream->codecpar->ch_layout.nb_channels;
    device_config.sampleRate = audioInfo.stream->codecpar->sample_rate;
    device_config.dataCallback = data_callback;
    device_config.pUserData = audioInfo.buffer;

    avformat_close_input(&audioInfo.format_ctx);

    if (ma_device_init(NULL, &device_config, &device) != MA_SUCCESS)
        return;

    if (ma_device_start(&device) != MA_SUCCESS)
    {
        ma_device_uninit(&device);
        return;
    }

    while (av_audio_fifo_size(audioInfo.buffer))
    {
    }

    av_audio_fifo_free(audioInfo.buffer);
    ma_device_uninit(&device);
}