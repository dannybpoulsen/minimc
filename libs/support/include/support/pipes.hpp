#include <memory>
#include <vector>

namespace MiniMC {
  namespace Support {
    template<class P>
    class Transformer {
    public:
      ~Transformer (){}
      virtual P operator() (P&& ) = 0;
    };

    template<class P>
    class TransformManager : public Transformer<P> {
    public:
      template<class T, class... Args>
      auto& add (Args... args) {
	transforms.push_back (std::make_unique<T> (args...));
	return *this;
      }
      
      virtual P operator() (P&& p)  {
	for (auto& t : transforms) {
	  p = (*t) (std::move(p));
	}
	return p;
      }
      
      
    private:
      std::vector<std::unique_ptr<Transformer<P>> > transforms;
    };
    
  }
}
