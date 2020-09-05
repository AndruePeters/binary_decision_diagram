/// \author Andrue Peters
/// \date   08/27/20
/// \brief  Binary Decision Diagram implementation based off of Pham and Emerson paper
///         "Every node can also be thought of as a BDD that begins at that node"
#ifndef BDD_H
#define BDD_H

#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <unordered_map>

struct BinaryDecisionDiagram
{
    BinaryDecisionDiagram(std::size_t index, BinaryDecisionDiagram* high, BinaryDecisionDiagram* low);
    std::size_t index_ = 0;
    BinaryDecisionDiagram* high_ = nullptr;
    BinaryDecisionDiagram* low_ = nullptr;
};

/// Binary Decision Diagram
/// Each node contains the index (subscript) and a pointer to the next node
class BinaryDecisionDiagramManager
{
  public:
    BinaryDecisionDiagram* makeBinaryDecisionDiagram(std::size_t index, BinaryDecisionDiagram* high, BinaryDecisionDiagram* low);

    explicit BinaryDecisionDiagramManager(std::size_t estimatedNumberVariables = 100);

    // Returns a constant references to the zero/one BinaryDecisionDiagram
    BinaryDecisionDiagram* one() const { return nodes[1].get(); }
    BinaryDecisionDiagram* zero() const { return nodes[0].get(); }

    BinaryDecisionDiagram* addNthIndex(std::size_t n);
    BinaryDecisionDiagram* ifThenElse(BinaryDecisionDiagram* ifNode, BinaryDecisionDiagram* thenNode, BinaryDecisionDiagram* elseNode);
    BinaryDecisionDiagram* restrict(BinaryDecisionDiagram* root, std::size_t var, bool val);
    std::vector<BinaryDecisionDiagram*> getNodes(std::size_t variableSubscript);

    BinaryDecisionDiagram* andOperation(BinaryDecisionDiagram* lhs, BinaryDecisionDiagram* rhs) { return ifThenElse(lhs, rhs, zero()); }
    BinaryDecisionDiagram* orOperation(BinaryDecisionDiagram* lhs, BinaryDecisionDiagram* rhs) { return ifThenElse(lhs, one(), rhs); }
    BinaryDecisionDiagram* xorOperation(BinaryDecisionDiagram* lhs, BinaryDecisionDiagram* rhs) { return ifThenElse(lhs, complement(rhs), rhs); }
    BinaryDecisionDiagram* complement(BinaryDecisionDiagram* node) { return ifThenElse(node, zero(), one()); }
    BinaryDecisionDiagram* norOperation(BinaryDecisionDiagram* lhs, BinaryDecisionDiagram* rhs) { return ifThenElse(lhs, zero(), complement(rhs)); }
    BinaryDecisionDiagram* equivalence(BinaryDecisionDiagram* lhs, BinaryDecisionDiagram* rhs) { return ifThenElse(lhs, rhs, complement(rhs)); }
    BinaryDecisionDiagram* nandOperation(BinaryDecisionDiagram* lhs, BinaryDecisionDiagram* rhs) { return ifThenElse(lhs, complement(rhs), zero()); }
    BinaryDecisionDiagram* inhibition(BinaryDecisionDiagram* lhs, BinaryDecisionDiagram* rhs) { return ifThenElse(lhs, complement(rhs), zero()); }

    auto begin() { return nodes.begin(); }
    auto end() { return nodes.end(); }

  private:
    std::vector<std::unique_ptr<BinaryDecisionDiagram>> nodes;
    std::unordered_multimap<std::size_t, BinaryDecisionDiagram*> indexToNode;
};

std::string toDot(BinaryDecisionDiagramManager& bdd, const std::string& name);
std::string toDot(BinaryDecisionDiagram* root, BinaryDecisionDiagram* one, BinaryDecisionDiagram* zero, const std::string& graphName);

#endif