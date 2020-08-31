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

/// Binary Decision Diagram
/// Each node contains the index (subscript) and a pointer to the next node
class BinaryDecisionDiagram {
public:
    struct Node {
        Node(std::size_t index, Node* high, Node* low);
        std::size_t index_ = 0;
        Node* high_ = nullptr;
        Node* low_ = nullptr;
    };

    Node* makeNode(std::size_t index, Node* high, Node* low);

    explicit BinaryDecisionDiagram(std::size_t estimatedNumberVariables = 100);

    // Returns a constant references to the zero/one node
    Node* one() const { return  nodes[1].get(); }
    Node* zero() const { return nodes[0].get(); }

    Node* addNthIndex(std::size_t n);
    Node* ifThenElse(Node* ifNode, Node* thenNode, Node* elseNode);
    Node* restrict(Node* root, std::size_t var, bool val);
    std::vector<Node*> getNodes(std::size_t variableSubscript);

    Node* andOperation(Node* lhs, Node* rhs) { return ifThenElse(lhs, rhs, zero()); }
    Node* orOperation(Node* lhs, Node* rhs) { return ifThenElse(lhs, one(), rhs);}
    Node* xorOperation(Node* lhs, Node* rhs) { return ifThenElse(lhs, ifThenElse(rhs, zero(), one()), rhs); }
    std::vector<std::unique_ptr<Node>>& getNodes() { return nodes; }

    auto begin() { return nodes.begin(); }
    auto end() { return nodes.end(); }
private:
    std::vector<std::unique_ptr<Node>> nodes;
    std::unordered_multimap<std::size_t, Node*> indexToNode;
};

std::string toDot(BinaryDecisionDiagram& bdd, const std::string& name);
std::string toDot(BinaryDecisionDiagram::Node* root, BinaryDecisionDiagram::Node* one, BinaryDecisionDiagram::Node* zero, const std::string& graphName);

#endif