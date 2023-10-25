#include <audio.hpp>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

void data_callback(ma_device *p_device, void *p_output, const void *p_input, ma_uint32 frame_count)
{
    AVAudioFifo *fifo = (AVAudioFifo *)p_device->pUserData;
    av_audio_fifo_read(fifo, &p_output, frame_count);
}

std::string Vid2ASCII::AudioPlayer::play_file(std::string file_path)
{
    // 1. Open the audio file
    AVFormatContext *format_ctx{nullptr};
    int ret = avformat_open_input(&format_ctx, file_path.c_str(), nullptr, nullptr);
    if (ret < 0)
        return "Unable to open media file!";

    ret = avformat_find_stream_info(format_ctx, nullptr);
    if (ret < 0)
        return "Unable to find stream info!";

    int stream_index = av_find_best_stream(format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (stream_index < 0)
        return "No audio streams found in file!";

    AVStream *stream = format_ctx->streams[stream_index];

    const AVCodec *decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!decoder)
        return "No appropriate decoder found for file!";

    AVCodecContext *codec_ctx{avcodec_alloc_context3(decoder)};
    avcodec_parameters_to_context(codec_ctx, stream->codecpar);

    ret = avcodec_open2(codec_ctx, decoder, nullptr);
    if (ret < 0)
        return "Decoder could not be opened\n";

    // 2. Decode the audio
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    SwrContext *resampler{swr_alloc_set_opts(
        nullptr,
        stream->codecpar->channel_layout,
        AV_SAMPLE_FMT_FLT,
        stream->codecpar->sample_rate,
        stream->codecpar->channel_layout,
        (AVSampleFormat)stream->codecpar->format,
        stream->codecpar->sample_rate,
        0,
        nullptr)};

    AVAudioFifo *fifo = av_audio_fifo_alloc(AV_SAMPLE_FMT_FLT, stream->codecpar->channels, 1);

    while (!av_read_frame(format_ctx, packet))
    {
        // If not reading audio stream, skip
        if (packet->stream_index != stream->index)
            continue;

        ret = avcodec_send_packet(codec_ctx, packet);
        if (ret < 0)
        {
            if (ret != AVERROR(EAGAIN))
                return "Error with decoding packet";
        }
        while ((ret = avcodec_receive_frame(codec_ctx, frame)) == 0)
        {
            // Resample frame
            AVFrame *resampled_frame = av_frame_alloc();
            resampled_frame->sample_rate = frame->sample_rate;
            resampled_frame->channel_layout = frame->channel_layout;
            resampled_frame->channels = frame->channels;
            resampled_frame->format = AV_SAMPLE_FMT_FLT;

            ret = swr_convert_frame(resampler, resampled_frame, frame);
            av_frame_unref(frame);
            av_audio_fifo_write(fifo, (void **)resampled_frame->data, resampled_frame->nb_samples);
            av_frame_unref(resampled_frame);
        }
    }

    // 3. Playback the audio
    ma_device_config device_config;
    ma_device device;

    device_config = ma_device_config_init(ma_device_type_playback);
    device_config.playback.format = ma_format_f32;
    device_config.playback.channels = stream->codecpar->channels;
    device_config.sampleRate = stream->codecpar->sample_rate;
    device_config.dataCallback = data_callback;
    device_config.pUserData = fifo;

    avformat_close_input(&format_ctx);
    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_free_context(&codec_ctx);
    swr_free(&resampler);

    if (ma_device_init(NULL, &device_config, &device) != MA_SUCCESS)
        return "Failed to open playback device";

    if (ma_device_start(&device) != MA_SUCCESS)
    {
        ma_device_uninit(&device);
        return "Failed to start playback device";
    }

    getchar();

    av_audio_fifo_free(fifo);
    ma_device_uninit(&device);

    return "";
}