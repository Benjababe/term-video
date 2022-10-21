#ifndef INCL_EXPORT_HEADER
#define INCL_EXPORT_HEADER
#include <export.hpp>
#endif

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