#include "renderer.hpp"

/**
 * @brief Default Renderer constructor
 *
 */
TermVideo::Renderer::Renderer() {}

/**
 * @brief Construct a new Renderer:: Renderer object
 *
 * @param media_info File information extracted from FFmpeg
 * @param frames_to_skip Frames to skip for every 1 frame. Reduces flickering effect
 * @param width Width to render ASCII output
 * @param height Height to render ASCII output
 * @param print_colour Flag whether to print coloured characters
 * @param col_threshold Threshold to use the previous colour set
 * @param filename Video file to be converted into ASCII
 * @param char_set Character set to be used for ASCII conversion
 */
TermVideo::Renderer::Renderer(MediaInfo media_info, Options opts)
{
    this->video_info.format_ctx = media_info.format_ctx;

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

    for (int row = 0; row < height; row++)
    {
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

        if (this->force_aspect)
            ascii_output += std::string(this->padding_x, ' ');
    }

    this->perf_checker.end_frame_time();
}

/**
 * @brief Downscales a frame to better fix ASCII characters & size
 *
 * @param frame Frame to be downscaled & converted into ASCII
 */
void TermVideo::Renderer::frame_downscale(cv::Mat &frame)
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

/**
 * @brief Waits for frametime sync before resuming program
 *
 * @param frametime_ns Time given for each frame in nanoseconds
 */
void TermVideo::Renderer::wait_for_frame(int64 frametime_ns)
{
    this->next_frame += std::chrono::nanoseconds(frametime_ns);
    std::this_thread::sleep_until(this->next_frame);
}

/**
 * @brief Converts a video into ASCII frames
 *
 * @param cap Video file to be converted
 */
void TermVideo::Renderer::video_to_ascii(cv::VideoCapture cap)
{
    double fps = cap.get(cv::CAP_PROP_FPS);
    int64 frametime_ns = (int64)(1e9 / fps) * (1 + this->frames_to_skip);

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

        // convert pixels and store to ascii_frame
        std::string ascii_frame;
        this->frame_to_ascii(ascii_frame, frame.data, frame.cols, frame.rows, frame.channels());
        std::cout << "\r" << ascii_frame;

        // refetch terminal size every 4 frames
        if (frame_count % 4 == 0)
            get_terminal_size(this->width, this->height);

        // wait for next interval before processing
        this->wait_for_frame(frametime_ns);
    }
}

/**
 * @brief Initialises values for the renderer
 *
 */
void TermVideo::Renderer::init_renderer()
{
    set_terminal_title("Video to ASCII");
    hide_terminal_cursor();
    get_terminal_size(this->width, this->height);
    init_terminal_col(this->print_colour);
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);

    this->ready = true;
}

/**
 * @brief Starts the video conversion and display
 *
 */
void TermVideo::Renderer::start_renderer()
{
    if (!this->ready)
        return;

    cv::VideoCapture cap(this->filename);
    this->video_to_ascii(cap);
    cap.release();

    // prints performance after finishing video
    double avg_time = this->perf_checker.get_avg_frame_time();
    std::cout << "Average frame time: " << avg_time << "ms" << std::endl;
}