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
    std::vector<Node*> getNodes(std::size_t variableSubscript = 0);

    Node* andOperation(Node* lhs, Node* rhs) { return ifThenElse(lhs, rhs, zero()); }

    auto begin() { return nodes.begin(); }
    auto end() { return nodes.end(); }
private:
    std::vector<std::unique_ptr<Node>> nodes;
    std::unordered_multimap<std::size_t, Node*> indexToNode;

    Node* ifThenElse();
};

std::string toDot(BinaryDecisionDiagram& bdd, const std::string& name);

/// Represents a Binary Decision Diagram
/// Every node can be thought of as its own BDD
/// Comes from realization that every boolean function can be written in "IF THEN ELSE" (ITE) form
struct BDD {
  BDD(int index_, BDD* high_, BDD* low_) : index(index_), high(high_), low(low_) {}
  int index = 0;      ///<    subscript in the variable ordering, so lower means closer to the top
  BDD* high = nullptr;      ///<    pointer to the "THEN" bdd
  BDD* low = nullptr;       ///<    pointer to the "ELSE" bdd
  BDD* next = nullptr;      ///<    pointer to the next bdd in the bucket
};

// #define PAIR(a ,b) ( ( std::size_t(a)+ std::size_t(b) )∗(std::size_t(a)+std::size_t(b)+(1) ) /(2)+std::size_t(a) )
// #define TRIPLE(a ,b, c) ((std::size_t int ) (PAIR((std::size_t int )c ,PAIR ((std::size_t )a ,(std::size_t )b) ) ) )
// #define NODEHASH( lvl , l , h) (TRIPLE( (std::size_t )( lvl ) ,(std::size_t)( l ) ,(std::size_t)(h) ) % 100 ) ;

std::size_t pair_func(const std::size_t a, const std::size_t b);
std::size_t triple_func(std::size_t a, std::size_t b, std::size_t c);

std::size_t node_hash(std::size_t lvl, std::size_t l, std::size_t h);



// Used to make sure that a new node is unique
BDD* makeNode(int var, BDD* high, BDD* low);

BDD* bdd_true();

BDD* bdd_false();

BDD* ithvar(int i);

/// Computes a BDD that results from assigning all isntances of the variable var witht he truth value val.
BDD* restrict(BDD* subtree, int var, bool val);

/// The ITE (if-then-else) operation computes and returns the BDD that is the result of applying the if-then-else
/// operator to three BDDs that serve as teh if, then, and else clauses
BDD* ite(BDD* I, BDD* T, BDD* E);

BDD* bdd_and(BDD* lhs, BDD* rhs);
double satcount_rec(BDD* subtree);

double satcount(BDD* subtree);

BDD* satone_rec(BDD* subtree);

BDD* satone(BDD* subtree);
#endif