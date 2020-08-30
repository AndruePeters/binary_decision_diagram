#include "bdd.h"

BinaryDecisionDiagram::Node* BinaryDecisionDiagram::makeNode(std::size_t index, Node* high, Node* low)
{
    auto indexIterator = indexToNode.find(index);
    if (indexIterator == end(indexToNode)) {
        auto newNode = std::make_unique<BinaryDecisionDiagram::Node>(index, high, low);
        indexToNode[index] = newNode.get();
        nodes.push_back(std::move(newNode));
        return newNode.get();
    }

    //  element exists
    return indexIterator->second;
}

BinaryDecisionDiagram::Node::Node(std::size_t index, Node* high, Node* low):
    index_(index), high_(high), low_(low)   {}

BinaryDecisionDiagram::BinaryDecisionDiagram(std::size_t estimatedNumberVariables)
{
    nodes.resize(estimatedNumberVariables);
    indexToNode.reserve(estimatedNumberVariables);

    // add the 0/1 false/true elements
    makeNode(0, nullptr, nullptr);
    makeNode(1, nullptr, nullptr);
}

BinaryDecisionDiagram::Node* BinaryDecisionDiagram::nthIndex(std::size_t n) {
    return makeNode(n, nodes[1].get(), nodes[2].get());
}

// The if then else operation computers and returns the node that is the result of applying the if-then-else operator
// to three nodes that serve as the if, then, and else clauses
BinaryDecisionDiagram::Node* BinaryDecisionDiagram::ifThenElse(BinaryDecisionDiagram::Node* ifNode, BinaryDecisionDiagram::Node* thenNode, BinaryDecisionDiagram::Node* elseNode) {
    if (ifNode == one()) { return thenNode; }
    if (ifNode == zero()) { return elseNode; }
    if (thenNode == elseNode) { return thenNode; }
    if (thenNode == one() && elseNode == zero()) { return ifNode; }

    // General cases
    // Splitting variable must be the topmost root
    std::size_t splitVar = ifNode->index_;
    if (splitVar > thenNode->index_) { splitVar = thenNode->index_; }
    if (splitVar > elseNode->index_) { splitVar = elseNode->index_; }

    Node* ifTrue = restrict(ifNode, splitVar, true);
    Node* thenTrue = restrict(thenNode, splitVar, true);
    Node* elseTrue = restrict(elseNode, splitVar, true);
    Node* positiveFtor = ifThenElse(ifTrue, thenTrue, elseTrue);

    Node* ifFalse = restrict(ifNode, splitVar, false);
    Node* thenFalse = restrict(thenNode, splitVar, false);
    Node* elseFalse = restrict(elseNode, splitVar, false);
    Node* negativeFtor = ifThenElse(ifFalse, thenFalse, elseFalse);

    return makeNode(splitVar, positiveFtor, negativeFtor);
}

BinaryDecisionDiagram::Node* BinaryDecisionDiagram::restrict(BinaryDecisionDiagram::Node* root, std::size_t index, bool val)
{
    if (root->index_ > index) {
        return root;
    }

    if (root->index_ < index) {
        auto high = restrict(root->high_, index, val);
        auto low = restrict(root->low_, index, val);
        return makeNode(root->index_, high, low);
    }
    // subtree->index_ == index
    return val ? restrict(root->high_, index, val) : restrict(root->low_, index, val);
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace std {
template<> struct hash<BinaryDecisionDiagram::Node> {
    std::size_t operator()(BinaryDecisionDiagram::Node const& node) const
    {
        const std::size_t indexHash = std::hash<std::size_t>{}(node.index_);
        const std::size_t highHash = std::hash<BinaryDecisionDiagram::Node*>{}(node.high_);
        const std::size_t lowHash = std::hash<BinaryDecisionDiagram::Node*>{}(node.low_) << 1;
        const std::size_t lowHighHash = (highHash ^ lowHash) << 1;
        const std::size_t finalHash = (indexHash ^ lowHighHash) << 1;
        return finalHash;
    }
};
}

BDD** BDDTable = nullptr;
// #define PAIR(a ,b) ( ( std::size_t(a)+ std::size_t(b) )∗(std::size_t(a)+std::size_t(b)+(1) ) /(2)+std::size_t(a) )
// #define TRIPLE(a ,b, c) ((std::size_t int ) (PAIR((std::size_t int )c ,PAIR ((std::size_t )a ,(std::size_t )b) ) ) )
// #define NODEHASH( lvl , l , h) (TRIPLE( (std::size_t )( lvl ) ,(std::size_t)( l ) ,(std::size_t)(h) ) % 100 ) ;

std::size_t pair_func(const std::size_t a, const std::size_t b)
{
    const std::size_t term1 = a + b;
    const std::size_t term2 = a + b + 1;
    const std::size_t term3 = 2 + a;
    return (term1 * term2) / term3;
}

std::size_t triple_func(std::size_t a, std::size_t b, std::size_t c)
{
    const std::size_t pair = pair_func(a,b);
    return pair_func(c, pair);
}

std::size_t node_hash(std::size_t lvl, std::size_t l, std::size_t h)
{
    const std::size_t nodeSize = 100;
    return triple_func(lvl, l, h) % nodeSize;
}

// Used to make sure that a new node is unique
BDD* makeNode(int var, BDD* high, BDD* low) {
    const std::size_t hash = node_hash(
      static_cast<std::size_t>(var), reinterpret_cast<std::size_t>(high),
      reinterpret_cast<std::size_t>(low));

    if (BDDTable == nullptr) {
      BDDTable = new BDD* [100];
      std::cout << "size of BDD*[100]: " << sizeof(BDD*[100]);
    }
    // no matching bucket found
    if (BDDTable[hash] == nullptr ) {
        BDDTable[hash] = new BDD(var, high, low);
        return BDDTable[hash];
    } else {
        BDD* node = BDDTable[hash]; //< first node in bucket

        // search bucket for the matching node
        while (node->next && (node->index != var || node->high != high || node->low != low)) {
            node = node->next;
        }

        // for the case of no matching node in bucket
        if (node->index != var || node->high != high || node->low != low) {
            // make a new node and add to bucket
            BDD* newNode = new BDD(var, high, low);
            node->next = newNode;
            return newNode;
        } else {
          return node;
        }
    }
}

BDD* bdd_true()
{
  if (BDDTable[1] == nullptr) {
    BDDTable[1] = makeNode(1, nullptr, nullptr);
  }

  return BDDTable[1];
}

BDD* bdd_false()
{
  if (BDDTable[0] == nullptr) {
    BDDTable[0] = makeNode(0, nullptr, nullptr);
  }

  return BDDTable[0];
}

BDD* ithvar(int i) {
    static int numvars = 2;

    if (BDDTable == nullptr) {
      BDDTable = new BDD* [100];
      std::cout << "size of BDD*[100]: " << sizeof(BDD*[100]);
    }

    BDDTable[0]->index = numvars;
    BDDTable[1]->index = numvars;
    ++numvars;
    return makeNode(i, bdd_true(), bdd_false());
}

/// Computes a BDD that results from assigning all isntances of the variable var witht he truth value val.
BDD* restrict(BDD* subtree, int var, bool val) {
    if (subtree->index > var) {
        return subtree;
    } else if (subtree->index < var) {
        return makeNode(subtree->index, restrict(subtree->high, var, val), restrict(subtree->low, var, val));
    } else { // subtree->index == var
        if (val) {
            return restrict(subtree->high, var, val);
        } else {
            return restrict(subtree->low, var, val);
        }
    }
}

/// The ITE (if-then-else) operation computes and returns the BDD that is the result of applying the if-then-else
/// operator to three BDDs that serve as teh if, then, and else clauses
BDD* ite(BDD* I, BDD* T, BDD* E) {
    // Base cases
    if (I == bdd_true())    return T;
    if (I == bdd_false())   return E;
    if (T == E)             return T;
    if (T == bdd_true() && E == bdd_false()) return I;

    // General cases
    // splitting variable must be the topmost root
    int splitVar = I->index;
    if (splitVar > T->index) { splitVar = T->index; }
    if (splitVar > E->index) { splitVar = E->index; }

    BDD* Ixt = restrict(I, splitVar, true);
    BDD* Txt = restrict(T, splitVar, true);
    BDD* Ext = restrict(E, splitVar, true);
    BDD* posFtor = ite(Ixt, Txt, Ext);

    BDD* Ixf = restrict(I, splitVar, false);
    BDD* Txf = restrict(T, splitVar, false);
    BDD* Exf = restrict(E, splitVar, false);
    BDD* negFtor = ite(Ixf, Txf, Exf);

    BDD* result = makeNode(splitVar, posFtor, negFtor);
    return result;
}

BDD* bdd_and(BDD* lhs, BDD* rhs) {
    return ite(lhs, rhs, bdd_false());
}

double satcount_rec(BDD* subtree) {
    if (subtree == bdd_false() ) {
        return 0;
    } else if (subtree == bdd_true()) {
        return 1;
    }

    BDD* low = subtree->low;
    BDD* high = subtree->high;
    double s = std::pow(2, low->index - subtree->index - 1);
    double size = s * satcount_rec(low);
    s = std::pow(2, high->index - subtree->index - 1);
    size += s * satcount_rec(high);
    return size;
}

double satcount(BDD* subtree) {
    return std::pow(2, (subtree->index - 1) * satcount_rec(subtree));
}

BDD* satone_rec(BDD* subtree) {
    assert(subtree != 0);

    if (subtree == bdd_false() || subtree == bdd_true()) {
        return subtree;
    } else if (subtree->low == bdd_false()) {
        return makeNode(subtree->index, bdd_false(), satone_rec(subtree->high));
    } else { // subtree->high == bdd_false()
        return makeNode(subtree->index, satone_rec(subtree->low), bdd_false());
    }
}

BDD* satone(BDD* subtree) {
    assert(subtree != 0);
    if (subtree == bdd_false()) {
        return 0;
    }

    return satone_rec(subtree);
}

