#include <buffer_renderer.hpp>

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
Vid2ASCII::BufferRenderer::BufferRenderer(Options opts)
{
    this->frames_to_skip = opts.frames_to_skip;
    this->print_colour = opts.print_colour;
    this->force_aspect = opts.force_aspect;
    this->filename = opts.filename;
    this->char_set = opts.char_set;
    this->padding_x = this->padding_y = 0;
    this->prev_r = this->prev_g = this->prev_b = 255;
    this->next_frame = std::chrono::steady_clock::now();
    this->perfChecker = PerformanceChecker();
    this->ready = false;
#if defined(__linux__)
    this->color_step_no = 1;
#endif
}

#if defined(__linux__)
/**
 * @brief Sets optimised colours based on total number of colour supported by the terminal
 */
void Vid2ASCII::BufferRenderer::set_curses_colors()
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
void Vid2ASCII::BufferRenderer::frame_to_ascii(uchar *frame_pixels, const int width, const int height, const int channels)
{
    this->perfChecker.start_frame_time();

    for (int row = 0; row < height; row++)
    {
#if defined(_WIN32)
        for (int col = 0; col < this->padding_x; col++)
            this->write_to_buffer(row, col, ' ', 0);

        for (int col = 0; col < width; col++)
        {
            ULONG index = channels * (row * width + col);
            uchar pixel_b = frame_pixels[index],
                  pixel_g = frame_pixels[index + 1],
                  pixel_r = frame_pixels[index + 2];

            char ascii = this->pixel_to_ascii(pixel_r, pixel_g, pixel_b);

            if (this->print_colour)
            {
                WORD attr = Vid2ASCII::get_win32_col(pixel_r, pixel_g, pixel_b);
                this->write_to_buffer(row, col + this->padding_x, ascii, attr);
            }
            else
            {
                // forces text to be white
                WORD white_attr = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
                this->write_to_buffer(row, col + this->padding_x, ascii, white_attr);
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
                int col_index = Vid2ASCII::get_ncurses_col_index(pixel_r, pixel_g, pixel_b, this->color_step_no);
                attron(COLOR_PAIR(col_index));
                mvprintw(row, col + this->padding_x, "%c", ascii);
                attroff(COLOR_PAIR(col_index));
            }
        }
        refresh();
#endif
    }

    this->perfChecker.end_frame_time();

#if defined(_WIN32)
    WriteConsoleOutputA(this->writeHandle, this->buffer, this->buffer_size, {0, 0}, &this->console_write_area);
#endif
}

/**
 * @brief Converts a video into ASCII frames
 *
 * @param cap Video file to be converted
 */
void Vid2ASCII::BufferRenderer::video_to_ascii(cv::VideoCapture cap)
{
    double fps = cap.get(cv::CAP_PROP_FPS);
    int64 frametime_ms = (int64)(1000 / fps) * (1 + this->frames_to_skip);

    while (1)
    {
        cv::Mat frame;

        for (int i = 0; i < (this->frames_to_skip + 1); i++)
            cap >> frame;

        int frame_count = (int)cap.get(1);

        // stop if EOF
        if (frame.empty())
            break;

        // reduces video resolution to fit the terminal
        this->frame_downscale(frame);

        // converts frame into ascii output & prints it out
        this->frame_to_ascii(frame.data, frame.cols, frame.rows, frame.channels());

        // wait for next interval before processing
        this->wait_for_frame(frametime_ms);
    }
}

/**
 * @brief Initialises values for the renderer
 *
 */
void Vid2ASCII::BufferRenderer::init_renderer()
{
    set_terminal_title("Video to ASCII (Buffer)");
    hide_terminal_cursor();
    get_terminal_size(this->width, this->height);
    init_terminal_col(this->print_colour);
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);

#if defined(_WIN32)
    short width_s = (short)this->width - 1,
          height_s = (short)this->height - 1;

    this->writeHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    this->buffer = new CHAR_INFO[this->width * this->height];
    this->console_write_area = {0, 0, width_s, height_s};

    this->buffer_size = {(short)this->width, (short)this->height};
    SetConsoleScreenBufferSize(this->writeHandle, this->buffer_size);

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
            std::cout << "Terminal does not support colour output!" << std::endl;
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
void Vid2ASCII::BufferRenderer::start_renderer()
{
    if (!this->ready)
        return;

    cv::VideoCapture cap(this->filename);
    this->video_to_ascii(cap);
    cap.release();

    // prints performance after finishing video
    double avg_time = this->perfChecker.get_avg_frame_time();
    std::cout << "Average frame time: " << avg_time << "ms" << std::endl;
}

#if defined(_WIN32)
void Vid2ASCII::BufferRenderer::write_to_buffer(const int row, const int col, uchar ascii, WORD attr)
{
    this->buffer[row * this->width + col].Char.AsciiChar = ascii;
    this->buffer[row * this->width + col].Attributes = attr;
}
#elif defined(__linux__)
#endif