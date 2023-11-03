#include "buffer_renderer.hpp"

/**
 * @brief Construct a new BufferRenderer:: BufferRenderer object
 *
 * @param frames_to_skip Frames to skip for every 1 frame. Reduces flickering effect
 * @param width Width to render ASCII output
 * @param height Height to render ASCII output
 * @param print_colour Flag whether to print coloured characters
 * @param col_threshold Threshold to use the previous colour set
 * @param filename Video file to be converted into ASCII
 * @param char_set Character set to be used for ASCII conversion
 */
TermVideo::BufferRenderer::BufferRenderer(Options opts)
{
#ifdef __USE_FFMPEG
    this->video_info.sws_ctx = nullptr;
#endif

    this->video_info.time_pt_ms = 0;
    this->video_info.locked = false;
    this->video_info.seek_step_ms = opts.seek_step_ms;
    this->video_info.seek_step_ms = opts.seek_step_ms;
    this->frames_to_skip = opts.frames_to_skip;
    this->print_colour = opts.print_colour;
    this->force_aspect = opts.force_aspect;
    this->filename = opts.filename;
    this->char_set = opts.char_set;
    this->padding_x = this->padding_y = 0;
    this->prev_r = this->prev_g = this->prev_b = 255;
    this->next_frame = std::chrono::steady_clock::now();
    this->perf_checker = PerformanceChecker();
    this->ready = false;
#if defined(__linux__)
    this->color_step_no = 1;
#endif
}

#if defined(__linux__)
/**
 * @brief Sets optimised colours based on total number of colour supported by the terminal
 */
void TermVideo::BufferRenderer::set_curses_colors()
{
    if (COLORS == 8 || !can_change_color())
        return;

    // subtract 1 because rgb=0 is included
    // eg. if cbrt(COLORS)=6, colour values=(0, 200, 400, 600, 800, 1000)
    // which is 5 steps instead of 6
    this->color_step_no = floor(std::cbrt(COLORS)) - 1;
    double col_step_amt = 1000 / this->color_step_no;

    for (int r = 0; r < this->color_step_no; r++)
    {
        for (int g = 0; g < this->color_step_no; g++)
        {
            for (int b = 0; b < this->color_step_no; b++)
            {
                short index = (r * this->color_step_no * this->color_step_no) + (g * this->color_step_no) + b;
                short r_val = col_step_amt * r,
                      g_val = col_step_amt * g,
                      b_val = col_step_amt * b;
                init_color(index, r_val, g_val, b_val);

                // for colour pairs, foreground will change while background will always be black
                init_pair(index, index, 0);
            }
        }
    }
}
#endif

/**
 * @brief Converts a full frame into a series of ASCII characters and displays them
 *
 * @param frame_pixels Vector of pixels in RGB order
 * @param width Width of the frame
 * @param height Height of the frame
 * @param channels No of colour channels for each pixel
 */
void TermVideo::BufferRenderer::frame_to_ascii(uchar *frame_pixels, const int width, const int height, const int channels)
{
    this->perf_checker.start_frame_time();

    for (int row = 0; row < height; row++)
    {
#if defined(_WIN32)
        for (int col = 0; col < width; col++)
        {
            ULONG index = channels * (row * width + col);
            uchar pixel_b = frame_pixels[index],
                  pixel_g = frame_pixels[index + 1],
                  pixel_r = frame_pixels[index + 2];

            char ascii = this->pixel_to_ascii(pixel_r, pixel_g, pixel_b);

            if (this->print_colour)
            {
                WORD attr = TermVideo::get_win32_col(pixel_r, pixel_g, pixel_b);
                this->write_to_buffer(row + this->padding_y, col + this->padding_x, ascii, attr);
            }
            else
            {
                // forces text to be white
                WORD white_attr = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
                this->write_to_buffer(row + this->padding_y, col + this->padding_x, ascii, white_attr);
            }
        }
#elif defined(__linux__)
        for (int col = 0; col < width; col++)
        {
            ULONG index = channels * (row * width + col);
            uchar pixel_b = frame_pixels[index],
                  pixel_g = frame_pixels[index + 1],
                  pixel_r = frame_pixels[index + 2];

            char ascii = this->pixel_to_ascii(pixel_r, pixel_g, pixel_b);

            if (this->print_colour)
            {
                int col_index = TermVideo::get_ncurses_col_index(pixel_r, pixel_g, pixel_b, this->color_step_no);
                attron(COLOR_PAIR(col_index));
                mvprintw(row, col + this->padding_x, "%c", ascii);
                attroff(COLOR_PAIR(col_index));
            }
        }
        refresh();
#endif
    }

    this->perf_checker.end_frame_time();

#if defined(_WIN32)
    WriteConsoleOutputA(this->write_handle, this->buffer, this->buffer_size, {0, 0}, &this->console_write_area);
#endif
}

#if defined(__USE_OPENCV)
/**
 * @brief Converts a video into ASCII frames
 */
void TermVideo::BufferRenderer::process_video_opencv()
{
    double fps = this->cap->get(cv::CAP_PROP_FPS);
    this->video_info.frametime_ns = (int64)(1e9 / fps) * (1 + this->frames_to_skip);

    while (1)
    {
        while (this->video_info.locked)
            ;
        this->video_info.locked = true;

        cv::Mat frame;

        for (int i = 0; i < (this->frames_to_skip + 1); i++)
            *this->cap >> frame;

        int frame_count = (int)this->cap->get(1);
        this->video_info.time_pt_ms = (int64_t)this->cap->get(cv::CAP_PROP_POS_MSEC);
        this->video_info.locked = false;

        // stop if EOF
        if (frame.empty())
            break;

        // reduces video resolution to fit the terminal
        this->frame_downscale_opencv(frame);

        // converts frame into ascii output & prints it out
        this->frame_to_ascii(frame.data, frame.cols, frame.rows, frame.channels());

        // refetch terminal size every interval
        if (frame_count % FETCH_TERMINAL_INTERVAL == 0)
            this->check_resize();

        // wait for next interval before processing
        this->wait_for_frame();
    }
}
#elif defined(__USE_FFMPEG)
/**
 * @brief Converts a video into ASCII frames. Uses FFmpeg
 */
void TermVideo::BufferRenderer::process_video_ffmpeg()
{
    AVFrame *frame = av_frame_alloc();
    AVPacket packet;

    int frame_count = 0;
    int skip_count = 0;

    while (!av_read_frame(this->video_info.format_ctx, &packet))
    {
        // skips if stream isn't the main video
        if (packet.stream_index != this->video_info.stream->index)
        {
            av_packet_unref(&packet);
            continue;
        }
        // skip if there are issues feeding packet into decoder
        if (avcodec_send_packet(this->video_info.codec_ctx, &packet))
        {
            av_packet_unref(&packet);
            continue;
        }
        // skip if there are issues decoding the packet
        if (avcodec_receive_frame(this->video_info.codec_ctx, frame))
        {
            av_frame_unref(frame);
            continue;
        }

        // skip frames by user request
        if (skip_count++ < this->frames_to_skip)
            continue;
        skip_count = 0;

        // reduces video resolution to fit the terminal
        this->frame_downscale_ffmpeg(frame);

        // convert pixels and store to ascii_frame
        std::string ascii_frame;
        this->frame_to_ascii(
            frame->data[0],
            frame->width, frame->height,
            this->video_info.colour_channels);

        av_frame_unref(frame);
        av_packet_unref(&packet);

        // refetch terminal size every interval
        if (frame_count % FETCH_TERMINAL_INTERVAL == 0)
            this->check_resize();

        this->wait_for_frame();
    }
}
#endif

void TermVideo::BufferRenderer::check_resize()
{
    int new_width, new_height;
    get_terminal_size(new_width, new_height, this->term_resized);

    if (this->width != new_width || this->height != new_height)
    {
        init_renderer();
    }
}

/**
 * @brief Initialises values for the renderer
 *
 */
void TermVideo::BufferRenderer::init_renderer()
{
    set_terminal_title(this->filename);
    hide_terminal_cursor();
    get_terminal_size(this->width, this->height, this->term_resized);
    init_terminal_col(this->print_colour);

#ifdef __USE_OPENCV
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);
#endif

#if defined(_WIN32)
    short width_s = (short)this->width - 1,
          height_s = (short)this->height - 1;

    this->write_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    this->buffer = new CHAR_INFO[this->width * this->height];
    this->console_write_area = {0, 0, width_s, height_s};

    this->buffer_size = {(short)this->width, (short)this->height};
    SetConsoleScreenBufferSize(this->write_handle, this->buffer_size);

    for (int i = 0; i < (this->width * this->height); i++)
    {
        this->buffer[i].Char.AsciiChar = ' ';
        this->buffer[i].Attributes = 0;
    }
#elif defined(__linux__)
    initscr();

    if (this->print_colour)
    {
        if (!has_colors())
        {
            std::cerr << "Terminal does not support colour output!" << std::endl;
            this->ready = false;
            return;
        }

        start_color();
        this->set_curses_colors();
    }
#endif

    this->ready = true;
}

/**
 * @brief Starts the video conversion and display
 *
 */
void TermVideo::BufferRenderer::start_renderer()
{
    if (!this->ready)
        return;

#if defined(__USE_OPENCV)
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

#if defined(_WIN32)
void TermVideo::BufferRenderer::write_to_buffer(const int row, const int col, uchar ascii, WORD attr)
{
    this->buffer[row * this->width + col].Char.AsciiChar = ascii;
    this->buffer[row * this->width + col].Attributes = attr;
}
#elif defined(__linux__)
#endif