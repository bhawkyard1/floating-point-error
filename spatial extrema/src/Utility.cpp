#include <ciso646>
#include <fstream>
#include <random>
#include <sstream>

#include <SDL.h>

#include "Utility.hpp"

namespace Utility
{
std::default_random_engine randomDevice = std::default_random_engine ((std::random_device())());

float radians(const float _degrees)
{
	return _degrees / 180.0f * pi;
}

float degrees(const float _radians)
{
	return _radians / pi * 180.0f;
}

float randFlt(const float _min, const float _max)
{
	//if there is not range to find a number between
	if( _min == _max )
		return _min;

	//random number generator
	std::uniform_real_distribution< float > uni( _min, _max );
	return uni( randomDevice );
}

int randInt(const int _min, const int _max)
{
	//if there is not range to find a number between
	if( _min == _max )
		return _min;

	//random number generator
	std::uniform_int_distribution<int> uni( _min, _max );
	return uni( randomDevice );
}

float average(std::vector<float> _values)
{
	float total = std::accumulate( _values.begin(), _values.end(), 0.0f );
	return total / static_cast<float>(_values.size());
}

float clamp(float _in, float _lo, float _hi)
{
	if(_in < _lo)
		return _lo;
	else if(_in > _hi)
		return _hi;
	return _in;
}

float sqrDistance(ngl::Vec2 _a, ngl::Vec2 _b)
{
	//compute pythagoras
	return ((_a.m_x -_b.m_x) * (_a.m_x -_b.m_x)) + ((_a.m_y - _b.m_y) * (_a.m_y - _b.m_y));
}

float remap01(float _in_value, float _max_in_range, float _min_in_range)
{
	//remap number
	float num = (_in_value - _min_in_range) / (_max_in_range - _min_in_range);
	//return absolute value (sometimes 0 was being returned as -0)
	return std::abs(num);
}

int findSetElement(std::set<int> _set, int index)
{
	auto iter = _set.begin();
	std::advance(iter, index);
	if (iter != _set.end())
	{
		return *iter;
	}
	else
	{
		std::cerr << "ERROR, element not found" << std::endl;
		return -1;
	}
}

bool pointInBox(std::pair<ngl::Vec3, ngl::Vec3> _box, ngl::Vec3 _pt)
{
	return _pt.m_x > _box.first.m_x and
			_pt.m_y > _box.first.m_y and
			_pt.m_z > _box.first.m_z and
			_pt.m_x < _box.second.m_x and
			_pt.m_y < _box.second.m_y and
			_pt.m_z < _box.second.m_z;
}

bool boxIntersectBox(std::pair<ngl::Vec3, ngl::Vec3> _a, std::pair<ngl::Vec3, ngl::Vec3> _b)
{
	return !(_b.first.m_x > _a.second.m_x or
					 _b.second.m_x < _a.first.m_x or
					 _b.first.m_y > _a.second.m_y or
					 _b.second.m_y < _a.first.m_y or
					 _b.first.m_z > _a.second.m_z or
					 _b.second.m_z < _a.first.m_z
					 );
}

ngl::Vec2 getMousePos()
{
	int x = 0;
	int y = 0;
	SDL_GetMouseState(&x, &y);

	return ngl::Vec2(x, y);
}

void errorExit(const std::string &_msg)
{
	std::cout << _msg << '\n';
	exit( EXIT_FAILURE );
}

std::istream& getlineSafe(std::istream& is, std::string& t)
{
	t.clear();

	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.

	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();

	for(;;) {
		int c = sb->sbumpc();
		switch (c) {
		case '\n':
			return is;
		case '\r':
			if(sb->sgetc() == '\n')
				sb->sbumpc();
			return is;
		case EOF:
			// Also handle the case when the last line has no line ending
			if(t.empty())
				is.setstate(std::ios::eofbit);
			return is;
		default:
			t += (char)c;
		}
	}
}

std::pair<std::string, std::string> splitFirst(std::string _str, char _delim)
{
		std::pair<std::string, std::string> ret;
		bool section = false;

		for(auto &ch : _str)
		{
				if(ch == _delim)
						section = true;

				if(!section)
						ret.first += ch;
				else
						ret.second += ch;
		}

		return ret;
}

std::vector<std::string> split(std::string _str, char _delim)
{
		std::vector<std::string> ret;

		std::stringstream ss(_str);
		std::string sub;

		while( getline( ss, sub, _delim ) )
		{
				if(sub != "")
						ret.push_back(sub);
		}

		return ret;
}

float rad(const float _deg)
{
		return _deg * (M_PI / 180.0f);
}


std::vector<std::string> split(std::string _str, std::string _delims)
{
		std::vector<std::string> ret;

		for(auto &character : _delims)
		{
				std::replace(_str.begin(), _str.end(), character, _delims[0]);
		}

		std::stringstream ss(_str);
		std::string sub;

		while( getline( ss, sub, _delims[0] ) )
		{
				if(sub != "")
						ret.push_back(sub);
		}

		return ret;
}

}
