#include <iostream>
#include <sstream>

#include "File.hpp"
#include "Utility.hpp"

#ifdef _WIN32
#include <ciso646>
#endif

namespace File {

std::vector<std::string> getLinesFromFile(const std::string &_path)
{
	std::vector<std::string> ret;
	std::ifstream src( _path );

	std::string cur;

	while(Utility::getlineSafe(src, cur))
		ret.push_back(cur);

	src.close();

	return ret;
}

}
