#ifndef LOADER_FILE_HPP
#define LOADER_FILE_HPP

/**
 * @brief Reads a file from the same volume as the loaded EFI image. Prints an error if it's not there, and returns nullptr
 *
 * @param name The null-terminated uint8_t-list of characters of the name
 * @return A pointer to malloc'd data of the file contents, or nullptr
 */
char* readFile(const char* name);

#endif
