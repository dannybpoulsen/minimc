#include <map>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/writer.h>

#include "support/graph.hpp"

namespace rj = rapidjson;

namespace MiniMC {
  namespace Support {
    struct NodeDetails {
      NodeDetails(size_t id, const std::string& str) : id(id), label(str) {}
      NodeDetails() : id(0), label("") {}
      size_t id;
      std::string label;
    };

    template <typename G>
    class JSONNode : public Node {
    public:
      JSONNode(G& g, NodeDetails& v) : graph(g), details(v) {
      }

      void setLabel(std::string str) {
        details.label = str;
        //v.AddMember ("Label",str,graph.getAllocator ());
      }

      void connect(Node& nn, std::string label) {
        auto& n = dynamic_cast<JSONNode<G>&>(nn);
        graph.addEdge(this->getID(), n.getID(), label);
      }

      void color(Color&) {
      }

      void color() {
      }

      auto getID() { return details.id; }

    private:
      G& graph;
      NodeDetails& details;
    };

    class JSONGraph : public Graph {
    public:
      JSONGraph(std::string) {
        doc.SetObject();
        edges.SetArray();
      }

      ~JSONGraph() {
      }

      std::unique_ptr<Node> getNode(const std::string& name) override {
        auto it = nodes.find(name);
        if (it != nodes.end()) {
          return std::make_unique<JSONNode<JSONGraph>>(*this, it->second);
        } else {
          NodeDetails details(nodes.size(), "");
          nodes.insert(std::pair<std::string, NodeDetails>(name, details));
          return std::make_unique<JSONNode<JSONGraph>>(*this, nodes[name]);
        }
      }

      void write(std::string) override {
      }

      void write(std::ostream& os) override {
        finalise();
        rj::OStreamWrapper osw(os);
        rj::Writer<rj::OStreamWrapper> writer(osw);
        doc.Accept(writer);
      }

      auto& getAllocator() { return doc.GetAllocator(); }

      void addEdge(int from, int to, const std::string& lab) {
        rj::Value value;
        value.SetObject();
        value.AddMember("source", from, getAllocator());
        value.AddMember("target", to, getAllocator());
        value.AddMember("label", createString(lab), getAllocator());
        edges.PushBack(value, getAllocator());
        //edges.push_back (value);
      }

    private:
      rj::Value createString(const std::string s) {
        rj::Value val;
        val.SetString(s.c_str(), s.size(), getAllocator());
        return val;
      }

      void finalise() {
        rj::Value rjnodes;
        rjnodes.SetArray();
        for (auto it = nodes.begin(); it != nodes.end(); ++it) {
          rj::Value node;
          node.SetObject();
          node.AddMember("id", it->second.id, getAllocator());
          node.AddMember("label", createString(it->second.label), getAllocator());
          rjnodes.PushBack(node, getAllocator());
        }

        doc.AddMember("nodes", rjnodes, getAllocator());
        doc.AddMember("edges", edges, getAllocator());
      }

      rj::Document doc;
      std::map<std::string, NodeDetails> nodes;
      rj::Value edges;
    };

    template <>
    std::unique_ptr<Graph> CreateGraph<GraphType::JSON>(const std::string& name) {
      return std::make_unique<JSONGraph>(name);
    }

  } // namespace Support
} // namespace MiniMC
