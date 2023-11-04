#include "renderer.hpp"

/**
 * @brief Default Renderer constructor
 *
 */
TermVideo::Renderer::Renderer() {}

/**
 * @brief Construct a new Renderer:: Renderer object
 *
 * @param frames_to_skip Frames to skip for every 1 frame. Reduces flickering effect
 * @param width Width to render ASCII output
 * @param height Height to render ASCII output
 * @param print_colour Flag whether to print coloured characters
 * @param col_threshold Threshold to use the previous colour set
 * @param filename Video file to be converted into ASCII
 * @param char_set Character set to be used for ASCII conversion
 */
TermVideo::Renderer::Renderer(MediaInfo *info, Options opts)
{
    this->info = (VideoInfo *)info;
    this->info->v_clock_ms = 0;
    this->info->v_locked = false;
    this->info->seek_step_ms = opts.seek_step_ms;

#ifdef __USE_FFMPEG
    this->info->v_sws_ctx = nullptr;
#endif

    this->frames_to_skip = opts.frames_to_skip;
    this->print_colour = opts.print_colour;
    this->force_aspect = opts.force_aspect;
    this->force_avg_luminance = opts.force_avg_lumi;
    this->col_threshold = opts.col_threshold;
    this->filename = opts.filename;
    this->char_set = opts.char_set;
    this->padding_x = this->padding_y = 0;
    this->prev_r = this->prev_g = this->prev_b = 255;
    this->next_frame = std::chrono::steady_clock::now();
    this->optimiser = Optimiser(col_threshold);
    this->perf_checker = PerformanceChecker();
    this->ready = false;
    this->term_resized = false;
}

/**
 * @brief Converts a pixel's colour values to an ASCII character
 *
 * @param pixel_r Brightness of red pixel, ranges 0-255
 * @param pixel_g Brightness of green pixel, ranges 0-255
 * @param pixel_b Brightness of blue pixel, ranges 0-255
 * @return char Appropriate ASCII character for pixel
 */
char TermVideo::Renderer::pixel_to_ascii(uchar pixel_r, uchar pixel_g, uchar pixel_b)
{
    uchar luminance = get_luminance_approximate(pixel_r, pixel_g, pixel_b, this->force_avg_luminance);
    double normalised_luminance = (double)luminance / 255;

    // choose a character set to print out
    std::string ascii_char_set = this->char_set;
    size_t len = ascii_char_set.length();
    int ascii_index = (int)(normalised_luminance * ascii_char_set.length());

    if (ascii_index >= ascii_char_set.length())
        ascii_index = (int)ascii_char_set.length() - 1;

    return ascii_char_set[ascii_index];
}

/**
 * @brief Converts a full frame into a series of ASCII characters
 *
 * @param ascii_output Frame converted into ASCII string
 * @param frame_pixels Vector of pixels in RGB order
 * @param width Width of the frame
 * @param height Height of the frame
 * @param channels No of colour channels for each pixel
 * @return std::string
 */
void TermVideo::Renderer::frame_to_ascii(
    std::string &ascii_output,
    uchar *frame_pixels,
    const int width,
    const int height,
    const int channels)
{
    ascii_output = "";
    this->perf_checker.start_frame_time();

    // add top padding to fit aspect ratio
    for (int i = 0; i < this->padding_y; i++)
        ascii_output += std::string(this->width, ' ');

    for (int row = 0; row < height; row++)
    {
        // left padding to fit aspect ratio
        if (this->force_aspect)
            ascii_output += std::string(this->padding_x, ' ');

        for (int col = 0; col < width; col++)
        {
            ULONG index = channels * (row * width + col);
            uchar pixel_b = frame_pixels[index],
                  pixel_g = frame_pixels[index + 1],
                  pixel_r = frame_pixels[index + 2];

            char ascii = this->pixel_to_ascii(pixel_r, pixel_g, pixel_b);

            // only uses ANSI colours when necessary
            if (this->print_colour && this->optimiser.should_apply_ansi_col(pixel_r, pixel_g, pixel_b, ascii))
            {
                std::string ascii_col = get_char_ansi_col(pixel_r, pixel_g, pixel_b, ascii);
                ascii_output += ascii_col;

                // updates previous set of pixel colours
                this->optimiser.set_prev_colours(pixel_r, pixel_g, pixel_b);
            }
            else
            {
                ascii_output += ascii;
            }
        }

        // right padding to extend to next line
        if (this->force_aspect)
        {
            int rem_len = this->width - width - this->padding_x;
            ascii_output += std::string(rem_len, ' ');
        }
    }

    // add bottom padding to fit aspect ratio
    for (int i = 0; i < (this->height - this->padding_y - height); i++)
        ascii_output += std::string(this->width, ' ');

    this->perf_checker.end_frame_time();
}

#if defined(__USE_OPENCV)
/**
 * @brief Downscales a frame to better fix ASCII characters & size. Uses opencv4.
 * @param frame Frame to be downscaled.
 */
void TermVideo::Renderer::frame_downscale_opencv(cv::Mat &frame)
{
    const int max_width = this->width,
              max_height = this->height;

    int new_width = frame.cols,
        new_height = frame.rows;

    // if forcing aspect ration & resizing by height first
    if (this->force_aspect && frame.rows > this->height)
    {
        new_height = this->height;
        new_width = (int)std::min((double)frame.cols * ((double)this->height / frame.rows) * 2, (double)this->width);
        this->padding_x = (this->width - new_width);
        this->padding_x = (this->padding_x / 2) + (this->padding_x & 1);
    }

    else if (frame.cols > max_width)
    {
        new_width = this->width;
        new_height = this->height;
    }

    if (new_width != frame.cols && new_height != frame.rows)
    {
        cv::Mat resized_frame;
        cv::resize(frame, resized_frame, cv::Size(new_width, new_height));
        frame = resized_frame;
    }
}
#elif defined(__USE_FFMPEG)
/**
 * @brief Downscales a frame to better fix ASCII characters & size. Uses ffmpeg.
 * @param frame Frame to be downscaled
 */
void TermVideo::Renderer::frame_downscale_ffmpeg(AVFrame *frame)
{
    AVPixelFormat output_format = AV_PIX_FMT_BGR24;

    // create scaler if doesn't exist or terminal has been resized
    if (this->info->v_sws_ctx == nullptr || this->term_resized)
    {
        this->padding_x = 0;
        this->padding_y = 0;

        int new_width = frame->width,
            new_height = frame->height;

        double terminal_aspect = (double)this->width / ((double)this->height * 2);
        double video_aspect = (double)frame->width / (double)frame->height;

        // NOTE: For the aspect ratio resizing, video_aspect is multiplied
        // by 2 as the pixel sizes for a terminal character is 2:1 in height:width

        // if forcing video's aspect ratio and its ratio is greater than terminal's
        // i.e. it's "wider" than the terminal, use y padding
        if (this->force_aspect && video_aspect > terminal_aspect)
        {
            new_width = this->width;
            new_height = (int)std::min(
                (double)this->height,
                ((double)this->width / (video_aspect * 2)));
            this->padding_y = this->height - new_height;
            this->padding_y = (this->padding_y / 2);
        }
        // if forcing video's aspect ratio and its ratio is less than terminal's
        // i.e. it's "taller" than the terminal, use x padding
        else if (this->force_aspect && video_aspect < terminal_aspect)
        {
            new_height = this->height;
            new_width = (int)std::min(
                (double)this->width,
                (double)this->height * video_aspect * 2);
            this->padding_x = this->width - new_width;
            this->padding_x = (this->padding_x / 2);
        }
        // default case, fit to terminal size
        else
        {
            new_width = this->width;
            new_height = this->height;
        }

        this->info->colour_channels = 3;
        this->info->new_width = new_width;
        this->info->new_height = new_height;
        this->info->v_sws_ctx = sws_getContext(
            frame->width, frame->height, (AVPixelFormat)frame->format,
            new_width, new_height, output_format,
            SWS_BILINEAR,
            nullptr, nullptr, nullptr);

        this->term_resized = false;
    }

    if (this->info->new_width != frame->width && this->info->new_height != frame->height)
    {
        AVFrame *resized_frame = av_frame_alloc();
        resized_frame->format = output_format;
        resized_frame->width = this->info->new_width;
        resized_frame->height = this->info->new_height;

        if (av_frame_get_buffer(resized_frame, 1) < 0)
            return;

        sws_scale(this->info->v_sws_ctx,
                  frame->data, frame->linesize,
                  0, frame->height,
                  resized_frame->data, resized_frame->linesize);

        av_frame_unref(frame);
        *frame = *resized_frame;
    }
}
#endif

void TermVideo::Renderer::print(std::string ascii_frame)
{
#if defined(_WIN32)
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {0, 0});
#elif defined(__linux__)
    move(0, 0);
    refresh();
#endif

    // https://stackoverflow.com/questions/51149880/fprintf-fputs-vs-cout-performance-for-large-strings
    if (ascii_frame.length() <= 30000)
        fputs(ascii_frame.c_str(), stdout);
    else
        fwrite(ascii_frame.c_str(), ascii_frame.length(), 1, stdout);
}

/**
 * @brief Waits for frametime sync before resuming program
 */
void TermVideo::Renderer::wait_for_frame()
{
    this->next_frame += std::chrono::nanoseconds(this->info->frametime_ns);
    std::this_thread::sleep_until(this->next_frame);
}

#if defined(__USE_OPENCV)
/**
 * @brief Converts a video into ASCII frames. Uses opencv4
 */
void TermVideo::Renderer::process_video_opencv()
{
    double fps = this->cap->get(cv::CAP_PROP_FPS);
    this->info->frametime_ns = (int64)(1e9 / fps) * (1 + this->frames_to_skip);

    while (1)
    {
        while (this->info->v_locked)
            ;
        this->info->v_locked = true;

        cv::Mat frame;

        for (int i = 0; i <= this->frames_to_skip; i++)
            *this->cap >> frame;

        int frame_count = (int)this->cap->get(1);
        this->info->time_pt_ms = (int64_t)this->cap->get(cv::CAP_PROP_POS_MSEC);
        this->info->v_locked = false;

        // stop if EOF
        if (frame.empty())
            break;

        // reduces video resolution to fit the terminal
        this->frame_downscale_opencv(frame);

        // convert pixels and store to ascii_frame
        std::string ascii_frame;
        this->frame_to_ascii(ascii_frame, frame.data, frame.cols, frame.rows, frame.channels());

        // properly print output frame
        this->print(ascii_frame);

        // refetch terminal size every interval
        if (frame_count % FETCH_TERMINAL_INTERVAL == 0)
            get_terminal_size(this->width, this->height, this->term_resized);

        // wait for next interval before processing
        this->wait_for_frame();
    }
}
#elif defined(__USE_FFMPEG)
/**
 * @brief Converts a video into ASCII frames. Uses FFmpeg
 */
void TermVideo::Renderer::process_video_ffmpeg()
{
    AVFrame *frame = av_frame_alloc();
    AVPacket packet;

    int frame_count = 0;
    int skip_count = 0;
    this->info->v_clock_ms = 0;

    while (1)
    {
        while (this->info->v_locked)
            ;
        this->info->v_locked = true;

        int ret = av_read_frame(this->info->v_format_ctx, &packet);
        if (ret < 0)
            break;

        // skips if stream isn't the main video
        // or if there are errors with decoding the packet
        // or just a general frame skip for optimisation
        if (packet.stream_index != this->info->v_stream->index ||
            avcodec_send_packet(this->info->v_codec_ctx, &packet) ||
            avcodec_receive_frame(this->info->v_codec_ctx, frame) ||
            skip_count++ < this->frames_to_skip)
        {
            av_packet_unref(&packet);
            av_frame_unref(frame);
            this->info->v_locked = false;
            continue;
        }

        this->info->v_locked = false;
        skip_count = 0;

        // keep track of current video time
        auto time_unit = av_q2d(this->info->v_stream->time_base);
        this->info->v_clock_ms = frame->best_effort_timestamp * time_unit * 1000;

        // reduces video resolution to fit the terminal
        this->frame_downscale_ffmpeg(frame);

        // convert pixels and store to ascii_frame
        std::string ascii_frame;
        this->frame_to_ascii(
            ascii_frame,
            frame->data[0],
            frame->width, frame->height,
            this->info->colour_channels);

        // properly print output frame
        this->print(ascii_frame);

        av_frame_unref(frame);
        av_packet_unref(&packet);

        // refetch terminal size every interval
        if (++frame_count % FETCH_TERMINAL_INTERVAL == 0)
            get_terminal_size(this->width, this->height, this->term_resized);

        this->wait_for_frame();
    }
}
#endif

void TermVideo::Renderer::seek(int64_t time_pt_ms, int flags)
{
#if defined(__USE_OPENCV)
    this->cap->set(cv::CAP_PROP_POS_MSEC, time_pt_ms);
#elif defined(__USE_FFMPEG)

    int64_t time_u = this->info->v_stream->time_base.den;
    int64_t timestamp = (time_pt_ms * time_u) / 1000;

    if (timestamp < 0)
        timestamp = 0;

    int ret = av_seek_frame(this->info->v_format_ctx,
                            this->info->v_stream->index,
                            timestamp,
                            flags);
    avcodec_flush_buffers(this->info->v_codec_ctx);

    this->info->v_clock_ms = time_pt_ms;
#endif
}

/**
 * @brief Initialises values for the renderer
 */
void TermVideo::Renderer::init_renderer()
{
    set_terminal_title(this->filename);
    hide_terminal_cursor();
    get_terminal_size(this->width, this->height, this->term_resized);
    init_terminal_col(this->print_colour);

#ifdef __USE_OPENCV
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);
#endif

#if defined(__linux__)
    initscr();
#endif

    this->ready = true;
}

#ifdef __USE_FFMPEG
/**
 * @brief Retrieves FFmpeg video decoder
 * @return std::string Error string
 */
std::string TermVideo::Renderer::get_decoder()
{
    // Sets frametime to know how long to delay per frame
    double fps = av_q2d(this->info->v_stream->r_frame_rate);
    this->info->frametime_ns = (int64)(1e9 / fps) * (1 + this->frames_to_skip);

    this->info->v_codec_ctx = avcodec_alloc_context3(this->info->v_decoder);
    avcodec_parameters_to_context(this->info->v_codec_ctx, this->info->v_stream->codecpar);

    int ret = avcodec_open2(this->info->v_codec_ctx, this->info->v_decoder, nullptr);
    if (ret < 0)
        return "Decoder could not be opened";

    return "";
}

/**
 * @brief Opens input stream to video file set in constructor
 * @return std::string Error string
 */
std::string TermVideo::Renderer::open_file()
{
    this->info->v_format_ctx = nullptr;
    int ret = avformat_open_input(&this->info->v_format_ctx, this->filename.c_str(), nullptr, nullptr);
    if (ret < 0)
        return "Unable to open media file!";

    ret = avformat_find_stream_info(this->info->v_format_ctx, nullptr);
    if (ret < 0)
        return "Unable to find stream info!";

    int stream_index = av_find_best_stream(
        this->info->v_format_ctx,
        AVMEDIA_TYPE_VIDEO,
        -1,
        -1,
        &this->info->v_decoder,
        0);
    if (stream_index < 0)
        return "No audio streams found in file!";

    this->info->v_stream = this->info->v_format_ctx->streams[stream_index];
    return "";
}
#endif

/**
 * @brief Starts the video conversion and display
 *
 */
void TermVideo::Renderer::start_renderer()
{
    if (!this->ready)
        return;

#ifdef __USE_OPENCV
    this->cap = new cv::VideoCapture(this->filename);
    this->process_video_opencv();
    this->cap->release();
#elif defined(__USE_FFMPEG)
    this->process_video_ffmpeg();
#endif

    // prints performance after finishing video
    double avg_time = this->perf_checker.get_avg_frame_time();
    std::cout << "Average frame time: " << avg_time << "ms" << std::endl;
}