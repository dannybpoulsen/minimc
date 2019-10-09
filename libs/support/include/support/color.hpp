#ifndef _COLOR__
#define _COLOR__

namespace MiniMC {
  namespace Support {
  class Color {
	public:
	  virtual std::string toString () = 0;
	};

	class HSLColor : public Color {
	public:
	  HSLColor (double h,double s, double l) : h(h),s(s),l(l) {}
	  virtual std::string toString () {
		std::stringstream str;
		str << h <<" " << s <<" "<< l;
		return str.str ();
	  }
	private:
	  double h;
	  double s;
	  double l;
	};
  }
  
}

#endif

