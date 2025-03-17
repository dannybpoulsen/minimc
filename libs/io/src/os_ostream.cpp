

#include "minimc/io/ostream.hpp"

#include <ostream>
#include <iostream>
#include <unordered_map>
#include <utility>

namespace MiniMC{
  namespace IO {

    ostream& os_ostream::out () {
      static os_ostream os{std::cout};
      return os;
    }

    ostream& os_ostream::err () {
      static os_ostream os{std::cerr};
      return os;
    }

    ostream& os_ostream::log () {
      static os_ostream os{std::clog};
      return os;
    }
    
    const std::unordered_map<ColorMod::Color,int> foregrounds = {
      {ColorMod::Color::RED,31},
      {ColorMod::Color::GREEN,32},
      {ColorMod::Color::BLUE,34},
      {ColorMod::Color::DEFAULT,39}
    };
    
    inline std::ostream& operator<< (std::ostream& os, const ColorMod& m ) {
#ifdef __linux__
      os << "\033[";
      if (m.isForeground ())
	return os << foregrounds.at(m.theColor ()) << "m";
      return os;
#endif
    }
    
    ostream& os_ostream::operator<< (const ColorMod& m){ os << m; return *this;}
    ostream& os_ostream::operator<< (std::uint8_t v){os << v;return *this;}
    ostream& os_ostream::operator<< (std::int8_t v){ os << v;return *this;}
    ostream& os_ostream::operator<< (std::uint16_t v){ os << v;return *this;}
    ostream& os_ostream::operator<< (std::int16_t v){ os << v;return *this;}
    ostream& os_ostream::operator<< (std::uint32_t v){ os << v;return *this;}
    ostream& os_ostream::operator<< (std::int32_t v){os << v;return *this;}
    ostream& os_ostream::operator<< (std::uint64_t v){ os << v;return *this;}
    ostream& os_ostream::operator<< (std::int64_t v){ os << v;return *this;}
    ostream& os_ostream::operator<< (const std::string& str){ os << str;return *this;}
    ostream& os_ostream::operator<< (const char* v){  os << v;return *this;}
    ostream& os_ostream::operator<< (const void* p ){  os << p;return *this;}
    void os_ostream::flush () {os.flush();} 

    ostream& os_ostream::operator<< (manipulator m) {
      switch(m) {
      case manipulator::endl:
	os << std::endl;
	break;
      case manipulator::clearline:
#ifdef __linux__
	os << "\33[2K\r";
#endif
	break;
      case manipulator::flush:
	os << std::flush;
	break;
      default:
	std::unreachable();
	
      }

      return *this;
    }


    
    ostream& str_ostream::operator<< (manipulator m ) {
      switch(m) {
      case manipulator::endl:
	stream << std::endl;
	break;
      case manipulator::clearline:
	break;
      case manipulator::flush:
	stream << std::flush;
	break;
      default:
	std::unreachable();
	
      }
      return *this;
    }
    ostream& str_ostream::operator<< (const ColorMod& m) {return *this;}
    ostream& str_ostream::operator<< (std::uint8_t m) {stream << m; return *this;}
    ostream& str_ostream::operator<< (std::int8_t m) {stream << m; return *this;}
    ostream& str_ostream::operator<< (std::uint16_t m) {stream << m; return *this;}
    ostream& str_ostream::operator<< (std::int16_t m) {stream << m; return *this;}
    ostream& str_ostream::operator<< (std::uint32_t m) {stream << m; return *this;}
    ostream& str_ostream::operator<< (std::int32_t m)  {stream << m; return *this;}
    ostream& str_ostream::operator<< (std::uint64_t m) {stream << m; return *this;}
    ostream& str_ostream::operator<< (std::int64_t m) {stream << m; return *this;}
    ostream& str_ostream::operator<< (const std::string& s) {stream << s; return *this;}
    ostream& str_ostream::operator<< (const char* m) {stream << m; return *this;}
    ostream& str_ostream::operator<< (const void*  m) {stream << m; return *this;}
      
    
  }
}
