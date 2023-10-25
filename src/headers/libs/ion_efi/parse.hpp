#ifndef LIBS_IONEFI_PARSE_HPP
#define LIBS_IONEFI_PARSE_HPP

#include <ion_efi/ion_efi.hpp>
namespace ion {
/**
 * @brief Skips any whitespace characters at the start of a string
 *
 * @param input the string to start from, which has ->pos changed
 */
void skipWhitespace(Input input);
}

#endif // !LIBS_IONEFI_PARSE_HPP
