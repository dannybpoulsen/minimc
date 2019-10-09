#include <memory>
#include <graphviz/cgraph.h>

#include "support/graph.hpp"

namespace MiniMC {
  namespace Support {
  class DOTGraph;
  
  class DOTNode : public Node{
  public:	
	DOTNode (Agraph_t* graph,
			 Agnode_t* node,std::string n,
			 size_t& edges
			 ) : graph(graph),
				 node(node),
				 name(n),
				 edges(edges)
	{
	  agset (node,const_cast<char*> ("shape"),const_cast<char*>("rectangle"));
	  agset (node,const_cast<char*> ("style"),const_cast<char*>("rounded,filled"));
	}
	
	void setLabel (std::string str) {
	  agset (node,const_cast<char*> ("label"),const_cast<char*>(str.c_str()));
	}

	void connect (Node& nn,std::string label) {
	  auto& n = dynamic_cast<DOTNode&> (nn);
	  auto e = agedge(graph,node,n.node,const_cast<char*>(std::to_string (edges).c_str()),1);
	  edges++;
	  agset (e,const_cast<char*>("label"),const_cast<char*>(label.c_str()));
	}

	void color (Color& c ) {
	  agset (node,const_cast<char*>("fillcolor"),const_cast<char*> (c.toString().c_str()));
	}

	
	void color ( ) {
	  HSLColor col (0.0,1.0,1.0);
	  color (col);
	}
	
  protected:
	Agraph_t* graph;
	Agnode_t* node;
	std::string name;
	size_t& edges;
  };


  
  class DOTGraph : public Graph  {
  public:
	DOTGraph (std::string name) {
	  graph = agopen (const_cast<char*>(name.c_str()),Agdirected,0);
	  agattr(graph,AGEDGE,const_cast<char*>("label"),const_cast<char*> (""));
	  agattr(graph,AGNODE,const_cast<char*>("label"),const_cast<char*>(""));
	  agattr(graph,AGNODE,const_cast<char*>("shape"),const_cast<char*>(""));
	  agattr(graph,AGNODE,const_cast<char*>("color"),const_cast<char*>("black"));
	  agattr(graph,AGNODE,const_cast<char*>("fillcolor"),const_cast<char*>("white"));
	  agattr(graph,AGNODE,const_cast<char*>("style"),const_cast<char*>("filled"));
	}

	~DOTGraph () {
	  agclose (graph);
	}
	
	std::unique_ptr<Node> getNode (const std::string& name) override {
	  return std::make_unique<DOTNode> (graph,agnode (graph,const_cast<char*> (name.c_str ()),1),name,edges);
	}

	void write (std::string f) override {
	  auto out = fopen ((f+".dot").c_str(),"w");
	  agwrite (graph,out);
	  fclose (out);
	  
	}

	void write (std::ostream& os) override {
	  char* buf;
	  size_t len;
	  auto out = open_memstream(&buf,&len);
	  agwrite (graph,out);
	  os << buf;
	  fclose (out);
	  free (buf);
	}
	
  protected:
	Agraph_t* graph;
	size_t edges;
  };
    

  
    
    template<>
    std::unique_ptr<Graph> CreateGraph<GraphType::DOT> (const std::string& name) {
      return std::make_unique<DOTGraph> (name);
    }
    
  }
}
