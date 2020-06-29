#ifndef _SUPP_GRAPH__
#define _SUPP_GRAPH__

#include <memory>
#include <vector>
#include <

namespace MiniMC {
  namespace util {
	template<class T>
	struct Node {
	  Node (const T& tt) : t(tt) {}
	  
	  T val;
	};

	template<class T>
	using Node_ptr = std::shared_ptr<Node<T> >;

	template<class T>
	struct Edge {
	  Edge (const Node_ptr<T>& f, const Node_ptr<T>& t) : from (f),to(t) {}
	  Node_ptr<T> from;
	  Node_ptr<T> to;
	};

	template<class T>
	using Edge_ptr = std::shared_ptr<Edge<T> >;

	
	template<class T>
	class Graph {
	public:
	  auto makeNode (const T& t) {
		nodes.push_back (std::make_shared<Node<T> > (T));
		return nodes.back();
	  }
	  
	  auto makeEdge (Node_ptr<T> from, Node_ptr<T> to) {
		edges.push_back (std::make_shared<Edge<T>> (from,to));
		return edges.back();
	  }

	  auto ebegin () const {return edges.begin();}
	  auto eend () const {return edges.end();}

	  auto nbegin () const {return nodes.begin();}
	  auto nend () const {return nodes.end();}
	  
	  
	private:
	  std::vector<Node_ptr<T> > nodes;
	  std::vector<Edge_ptr> edges;
	};

	void outputDot (const Graph& g, const std::string& f); 
	
  }
}

#endif
