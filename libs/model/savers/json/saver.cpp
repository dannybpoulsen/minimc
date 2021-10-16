#include <sstream>
#include <string>
#ifdef RAPIDJSON
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#endif

#include "savers/savers.hpp"
#include "support/binary_encode.hpp"

namespace MiniMC {
  namespace Savers {
#ifdef RAPIDJSON
    template <class T>
    std::string toString(const T& t) {
      std::stringstream str;
      str << t;
      return str.str();
    }

    template <class Obj, class Allocator>
    void writePreamble(Obj& obj, Allocator& alloc) {
      obj.AddMember("MiniMC JSON Version", rapidjson::Value().SetString("0.1"), alloc);
    }

    template <class Obj, class Allocator>
    void writeVariableStack(MiniMC::Model::VariableStackDescr& stack, Obj& obj, Allocator& alloc) {
      rapidjson::Value rvalue;
      rvalue.SetArray();
      for (auto& var : stack.getVariables()) {
        rapidjson::Value tempvalue;
        tempvalue.SetObject();
        tempvalue.AddMember("name", rapidjson::Value().SetString(var->getName().c_str(), alloc), alloc);
        tempvalue.AddMember("type", rapidjson::Value().SetString(toString(*var->getType()).c_str(), alloc), alloc);
        tempvalue.AddMember("id", rapidjson::Value().SetInt(var->getId()), alloc);
        rvalue.PushBack(tempvalue, alloc);
      }
      obj.AddMember("VariableStack", rvalue, alloc);
    }

    template <class Obj, class Allocator>
    void writeValue(const MiniMC::Model::Value_ptr& val, Obj& obj, Allocator& alloc) {
      rapidjson::Value rvalue;
      rvalue.SetObject();
      if (val->isRegister()) {
        auto var = std::static_pointer_cast<MiniMC::Model::Register>(val);
        rvalue.AddMember("type", rapidjson::Value().SetString(var->isGlobal() ? "global" : "local", alloc), alloc);
        rvalue.AddMember("value", rapidjson::Value().SetInt(var->getId()), alloc);

      }

      else if (val->isConstant()) {
        auto constant = std::static_pointer_cast<MiniMC::Model::Constant>(val);
        if (constant->isNonCompileConstant()) {
          auto ncconstant = std::static_pointer_cast<MiniMC::Model::NonCompileConstant>(constant);
          if (ncconstant->isAggregate()) {
            auto ancconsant = std::static_pointer_cast<MiniMC::Model::AggregateNonCompileConstant>(constant);
            rvalue.AddMember("type", rapidjson::Value().SetString("AggregateNonCompileConstant", alloc), alloc);
            rapidjson::Value innervals;
            innervals.SetArray();
            for (auto val : *ancconsant) {
              writeValue(val, innervals, alloc);
            }
            rvalue.AddMember("value", innervals, alloc);
          }
        }

        else {
          MiniMC::Support::Base64Encode encoder;
          rvalue.AddMember("type", rapidjson::Value().SetString(constant->isInteger() ? "integer" : "blob", alloc), alloc);
          rvalue.AddMember("size", rapidjson::Value().SetInt(constant->getSize()), alloc);

          std::stringstream str;
          str << encoder.encode(reinterpret_cast<const char*>(constant->getData()), constant->getSize());
          rvalue.AddMember("value", rapidjson::Value().SetString(str.str().c_str(), alloc), alloc);
        }
      }
      obj.PushBack(rvalue, alloc);
    }

    template <class Obj, class Allocator>
    void writeInstruction(const MiniMC::Model::Instruction& instr, Obj& obj, Allocator& alloc) {
      rapidjson::Value rvalue;
      rvalue.SetObject();
      std::stringstream opstr;
      opstr << instr.getOpcode();
      rvalue.AddMember("opcode", rapidjson::Value().SetString(opstr.str().c_str(), alloc), alloc);
      rapidjson::Value params;
      params.SetArray();
      for (auto& val : instr) {
        writeValue(val, params, alloc);
      }
      rvalue.AddMember("operands", params, alloc);
      obj.PushBack(rvalue, alloc);
    }

    template <class Obj, class Allocator>
    void writeInstructionStream(const MiniMC::Model::InstructionStream& stream, Obj& obj, Allocator& alloc) {
      rapidjson::Value rvalue;
      rvalue.SetArray();
      for (auto& instr : stream) {
        writeInstruction(instr, rvalue, alloc);
      }
      obj.AddMember("instructions", rvalue, alloc);
    }

    template <class Obj, class Allocator>
    void writeLocation(const MiniMC::Model::Location_ptr& loc, Obj& obj, Allocator& alloc) {
      rapidjson::Value rvalue;
      rvalue.SetObject();
      rvalue.AddMember("id", rapidjson::Value().SetInt(loc->getID()), alloc);
      rvalue.AddMember("callplace", rapidjson::Value().SetBool(loc->getInfo().is<MiniMC::Model::Attributes::CallPlace>()), alloc);
      rvalue.AddMember("assumptionplace", rapidjson::Value().SetBool(loc->getInfo().is<MiniMC::Model::Attributes::AssumptionPlace>()), alloc);
      obj.PushBack(rvalue, alloc);
    }

    template <class Obj, class Allocator>
    void writeEdge(const MiniMC::Model::Edge_ptr& edge, Obj& obj, Allocator& alloc) {
      rapidjson::Value rvalue;
      rvalue.SetObject();
      rvalue.AddMember("from", rapidjson::Value().SetInt(edge->getFrom()->getID()), alloc);
      rvalue.AddMember("to", rapidjson::Value().SetInt(edge->getTo()->getID()), alloc);
      if (edge->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
        writeInstructionStream(edge->getAttribute<MiniMC::Model::AttributeType::Instructions>(), rvalue, alloc);
      }
      obj.PushBack(rvalue, alloc);
    }

    template <class Obj, class Allocator>
    void writeCFG(const MiniMC::Model::CFG_ptr cfg, Obj& obj, Allocator& alloc) {
      rapidjson::Value rvalue;
      rvalue.SetObject();

      rapidjson::Value locations;
      locations.SetArray();
      for (auto& loc : cfg->getLocations()) {
        writeLocation(loc, locations, alloc);
      }

      rapidjson::Value edges;
      edges.SetArray();
      for (auto& edge : cfg->getEdges()) {
        writeEdge(edge, edges, alloc);
      }

      rvalue.AddMember("locations", locations, alloc);
      rvalue.AddMember("initiallocation", rapidjson::Value().SetInt(cfg->getInitialLocation()->getID()), alloc);
      rvalue.AddMember("edges", edges, alloc);

      obj.AddMember("CFG", rvalue, alloc);
    }

    template <class Obj, class Allocator>
    void writeFunction(const MiniMC::Model::Function_ptr& func, Obj& obj, Allocator& alloc) {
      rapidjson::Value rvalue;
      rvalue.SetObject();
      rvalue.AddMember("name", rapidjson::Value().SetString(func->getName().c_str(), alloc), alloc);
      rvalue.AddMember("id", rapidjson::Value().SetInt(func->getID()), alloc);
      rvalue.AddMember("returns", rapidjson::Value().SetString(toString(*func->getReturnType()).c_str(), alloc), alloc);
      writeVariableStack(*func->getVariableStackDescr(), rvalue, alloc);
      rapidjson::Value params;
      params.SetArray();
      for (auto& v : func->getParameters()) {
        writeValue(v.get(), params, alloc);
      }
      rvalue.AddMember("parameters", params, alloc);
      writeCFG(func->getCFG(), rvalue, alloc);

      obj.PushBack(rvalue, alloc);
    }

    template <class Obj, class Allocator>
    void writeGlobalDefinitions(const MiniMC::Model::Program_ptr& prgm, Obj& obj, Allocator& alloc) {
      rapidjson::Value rvalue;
      rvalue.SetObject();
      writeVariableStack(*prgm->getGlobals().get(), rvalue, alloc);
      writeInstructionStream(prgm->getInitialisation(), rvalue, alloc);
      obj.AddMember("globals", rvalue, alloc);

      rapidjson::Value functions;
      functions.SetArray();
      for (auto func : prgm->getFunctions()) {
        writeFunction(func, functions, alloc);
      }
      obj.AddMember("functions", functions, alloc);
    }

    template <>
    void saveModel<Type::JSON, BaseSaveOptions>(const MiniMC::Model::Program_ptr& prgm, BaseSaveOptions saveOptions) {
      rapidjson::Document doc;
      doc.SetObject();

      writePreamble(doc, doc.GetAllocator());
      writeGlobalDefinitions(prgm, doc, doc.GetAllocator());

      rapidjson::StringBuffer buffer;
      rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
      doc.Accept(writer);
      *saveOptions.writeTo << buffer.GetString() << std::endl;
    }
#else

    template <>
    void saveModel<Type::JSON, BaseSaveOptions>(const MiniMC::Model::Program_ptr& prgm, BaseSaveOptions saveOptions) {
    }
#endif
  } // namespace Savers
} // namespace MiniMC
