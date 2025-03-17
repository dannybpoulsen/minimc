#ifndef _MINIC_OSTREAM__
#define _MINIC_OSTREAM__
#include <cstdint>
#include <string>
#include <iosfwd>
#include <iomanip>
#include <sstream>

namespace MiniMC {
  namespace IO {

    class ColorMod {
    public:
      enum class Color {
	RED,
	GREEN,
	BLUE,
	DEFAULT
      };
      ColorMod (Color  color= Color::RED, bool foreground = true) : foreground(foreground),color(color) {}
      auto& isForeground () const {return foreground;}
      auto& theColor () const {return color;}
      
    private:
      bool foreground;
      Color color;
    };

    enum class manipulator {
      endl,
      clearline,
      flush
    };
    
    
    class ostream {
    public:

      virtual ostream& operator<< (manipulator) = 0;
      
      virtual ostream& operator<< (const ColorMod&) = 0 ;
      virtual ostream& operator<< (std::uint8_t) = 0;
      virtual ostream& operator<< (std::int8_t) = 0;
      virtual ostream& operator<< (std::uint16_t) = 0;
      virtual ostream& operator<< (std::int16_t) = 0;
      virtual ostream& operator<< (std::uint32_t) = 0;
      virtual ostream& operator<< (std::int32_t) = 0;
      virtual ostream& operator<< (std::uint64_t) = 0;
      virtual ostream& operator<< (std::int64_t) = 0;
      virtual ostream& operator<< (const std::string& str) = 0;
      virtual ostream& operator<< (const char* ) = 0;
      virtual ostream& operator<< (const void* p ) = 0;
      virtual void flush () = 0; 
      
    };

    template<typename T>
    class HexFormat {
    public:
      ostream& output (ostream& os) const  {
	std::stringstream str;
	str << std::hex << t;
	os << str.str();
	return os;
      }
    private:
      T t;
    };

    
    template<class M>
    concept Outputtable = requires (const M& m, MiniMC::IO::ostream& os) {
      {m.output(os)};
    };
    
    
    template<Outputtable O>
    inline ostream& operator<< (ostream& os, O& o) {
      return o.output(os);
    }
    
    
    class os_ostream  : public ostream{
    public:
      static ostream& out ();
      static ostream& err ();
      static ostream& log ();
      
      
      virtual ostream& operator<< (manipulator);
      virtual ostream& operator<< (const ColorMod&);
      virtual ostream& operator<< (std::uint8_t);
      virtual ostream& operator<< (std::int8_t);
      virtual ostream& operator<< (std::uint16_t);
      virtual ostream& operator<< (std::int16_t);
      virtual ostream& operator<< (std::uint32_t);
      virtual ostream& operator<< (std::int32_t);
      virtual ostream& operator<< (std::uint64_t);
      virtual ostream& operator<< (std::int64_t);
      virtual ostream& operator<< (const std::string& str);
      virtual ostream& operator<< (const char* );
      virtual ostream& operator<< (const void*  );
      void flush (); 
      
    private:
      os_ostream (std::ostream& os) : os(os) {}
      std::ostream& os;
    };

    class str_ostream  : public ostream{
    public:
      ostream& operator<< (manipulator);
      ostream& operator<< (const ColorMod&);
      ostream& operator<< (std::uint8_t);
      ostream& operator<< (std::int8_t);
      ostream& operator<< (std::uint16_t);
      ostream& operator<< (std::int16_t);
      ostream& operator<< (std::uint32_t);
      ostream& operator<< (std::int32_t);
      ostream& operator<< (std::uint64_t);
      ostream& operator<< (std::int64_t);
      ostream& operator<< (const std::string& str);
      ostream& operator<< (const char* );
      ostream& operator<< (const void*  );
      void flush () {} 

      std::string str () const {return stream.str();}
      void clear ()  {stream.str(std::string{}) ;}
      
    private:
      std::stringstream stream;
    };
    
        
    
  }
}

#endif
