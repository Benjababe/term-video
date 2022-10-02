#ifndef INCL_STD_HEADERS
#define INCL_STD_HEADERS
#include <algorithm>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#endif

#include <optimiser.hpp>
#include <terminal.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

using namespace cv;

void int_to_string_leading_zeroes(std::string, int, int);
char pixel_to_ascii(uchar, uchar, uchar);
void frame_to_ascii(std::string &, std::vector<uchar>, int, int, int);
void frame_downscale(Mat &);
void video_to_ascii(VideoCapture);
void save_ascii_to_file(const std::string, const std::string);

typedef unsigned char uchar;

// no. of frames to skip per 1 frame. will decrease 'film' like effect for coloured output because of frequent printing
int skip_frames = 0;
int terminal_width, terminal_height;
bool print_colour = true;
uchar prev_r = 255, prev_g = 255, prev_b = 255;
std::string ascii_grayscale_chars = " `.:+=xsoeaS@", ascii_colour_chars = " .*es@";
std::chrono::steady_clock::time_point next_frame = std::chrono::steady_clock::now();

/**
 * @brief Converts a pixel's colour values to an ASCII character
 *
 * @param pixel_r Brightness of red pixel, ranges 0-255
 * @param pixel_g Brightness of green pixel, ranges 0-255
 * @param pixel_b Brightness of blue pixel, ranges 0-255
 * @return char Appropriate ASCII character for pixel
 */
char pixel_to_ascii(uchar pixel_r, uchar pixel_g, uchar pixel_b)
{
    // https://en.wikipedia.org/wiki/Relative_luminance
    double rgb_sum = (0.2126 * pixel_r) + (0.7152 * pixel_g) + (0.0722 * pixel_b);
    double normalised_rgb_sum = rgb_sum / 255;

    // choose a character set to print out
    std::string ascii_char_set = (print_colour) ? ascii_colour_chars : ascii_grayscale_chars;

    size_t len = ascii_char_set.length();
    int ascii_index = (int)(normalised_rgb_sum * ascii_char_set.length());

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
 * @param channels No of colour channels in the frame
 * @return std::string
 */
void frame_to_ascii(std::string &ascii_output, std::vector<uchar> frame_pixels, int width, int height, int channels)
{
    ascii_output = "";

    Optimiser optimiser;
    optimiser.set_colour_threshold(4);
    optimiser.set_prev_colours(0, 0, 0);

    for (int pixel_y = 0; pixel_y < height; pixel_y++)
    {
        for (int pixel_x = 0; pixel_x < width; pixel_x++)
        {
            ULONG index = channels * (pixel_y * width + pixel_x);
            uchar pixel_b = frame_pixels[index],
                  pixel_g = frame_pixels[index + 1],
                  pixel_r = frame_pixels[index + 2];

            if (print_colour)
            {
                char ascii = pixel_to_ascii(pixel_r, pixel_g, pixel_b);

                // only include ANSI colour codes if needed, for optimisation
                if (optimiser.apply_ansi_col(pixel_r, pixel_g, pixel_b))
                {
                    std::string ascii_col = get_char_col(pixel_r, pixel_g, pixel_b, ascii);
                    ascii_output += ascii_col;
                }
                else
                {
                    ascii_output += std::string(1, ascii);
                }
            }
            else
            {
                char ascii = pixel_to_ascii(pixel_r, pixel_g, pixel_b);
                ascii_output += ascii;
            }

            // updates previous set of pixel colours
            optimiser.set_prev_colours(pixel_r, pixel_g, pixel_b);
        }
        ascii_output += "\n";
    }
}

/**
 * @brief Downscales a frame to better fix ASCII characters & size
 *
 * @param frame Frame to be downscaled & converted into ASCII
 */
void frame_downscale(Mat &frame)
{
    const int max_width = terminal_width,
              max_height = terminal_height;

    int new_width = frame.cols,
        new_height = frame.rows;

    if (frame.cols > max_width)
    {
        new_width = terminal_width;
        new_height = terminal_height;
    }

    if (new_width != frame.cols && new_height != frame.rows)
    {
        Mat resized_frame;
        resize(frame, resized_frame, Size(max_width, new_height));
        frame = resized_frame;
    }
}

/**
 * @brief Converts a video into ASCII frames
 *
 * @param cap Video file to be converted
 */
void video_to_ascii(VideoCapture cap)
{
    double fps = cap.get(CAP_PROP_FPS);
    while (1)
    {
        Mat frame;

        for (int i = 0; i < (skip_frames + 1); i++)
            cap >> frame;

        int frame_count = (int)cap.get(1);

        if (frame.empty())
            break;

        // reduces video resolution to fit the terminal
        frame_downscale(frame);

        // collect rgb data for each pixel
        std::vector<uchar> frame_pixels(frame.rows * frame.cols * frame.channels());
        frame_pixels.assign(frame.data, frame.data + frame.total() * frame.channels());

        // convert pixels and store to ascii_frame
        std::string ascii_frame;
        frame_to_ascii(ascii_frame, frame_pixels, frame.cols, frame.rows, frame.channels());

        // save ASCII if it is to be displayed elsewhere
        // std::string ascii_txt_path = "./output/" + std::to_string(frame_count) + ".txt";
        // save_ascii_to_file(ascii_txt_path, ascii_frame);

        // wait for frametime interval before displaying
        int64 time_per_frame = (int64)(1000 / fps) * (1 + skip_frames);
        next_frame += std::chrono::milliseconds(time_per_frame);
        std::this_thread::sleep_until(next_frame);

        std::cout << "\r" << ascii_frame;
    }
}

/**
 * @brief Save ASCII string into a file
 *
 * @param filepath Relative path to save string
 * @param ascii ASCII string to be saved
 */
void save_ascii_to_file(const std::string filepath, const std::string ascii)
{
    std::ofstream file;
    file.open(filepath);
    file << ascii;
    file.close();
}

int main(int argc, char **argv)
{
    std::vector<std::string> args(argv + 1, argv + argc);

    set_terminal_title("Video to ASCII");
    get_terminal_size(terminal_width, terminal_height);

    // hide annoying opencv info logs
    utils::logging::setLogLevel(utils::logging::LogLevel::LOG_LEVEL_ERROR);

    std::string filename = "./data/ouran.mkv";
    VideoCapture cap(filename);

    video_to_ascii(cap);

    cap.release();
    destroyAllWindows();

    return 0;
}