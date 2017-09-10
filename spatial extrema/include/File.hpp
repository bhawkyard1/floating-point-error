//----------------------------------------------------------------------------------------------------------------------
/// \file fft.hpp
/// \brief This file contains functions related to the file operations.
/// \author Ben Hawkyard
/// \version 1.0
/// \date 19/01/17
/// Revision History :
/// This is an initial version used for the program.
//----------------------------------------------------------------------------------------------------------------------

#ifndef FILE_HPP
#define FILE_HPP

#include <fstream>
#include <string>
#include <sstream>
#include <vector>

namespace File {
std::vector<std::string> getLinesFromFile(const std::string &_path);
}

#endif
