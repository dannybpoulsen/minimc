#ifndef _GRAPH__
#define _GRAPH__

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "support/color.hpp"

namespace MiniMC {
  namespace Support {
    class Graph;

    class Node {
    public:
      virtual ~Node() {}
      virtual void setLabel(std::string str) = 0;
      virtual void connect(Node& n, std::string label) = 0;
      virtual void color(const Color& c) = 0;
      virtual void color() = 0;
    };

    class Graph {
    public:
      virtual ~Graph() {}
      virtual std::unique_ptr<Node> getNode(const std::string& name) = 0;
      virtual void write(std::string f) = 0;
      virtual void write(std::ostream& os) = 0;
    };

    using Graph_ptr = std::unique_ptr<Graph>;

    enum class GraphType {
      DOT,
      JSON
    };

    template <GraphType>
    std::unique_ptr<Graph> CreateGraph(const std::string& name);
  } // namespace Support
} // namespace MiniMC

#endif
