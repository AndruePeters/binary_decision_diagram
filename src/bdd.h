/// \author Andrue Peters
/// \date   08/27/20
/// \brief  Binary Decision Diagram implementation based off of Pham and Emerson paper
///         "Every node can also be thought of as a BDD that begins at that node"
#ifndef BDD_H
#define BDD_H

#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>

/// Represents a Binary Decision Diagram
/// Every node can be thought of as its own BDD
/// Comes from realization that every boolean function can be written in "IF THEN ELSE" (ITE) form
struct BDD {
  BDD(int index, BDD* high, BDD* low) : index(index), high(high), low(low) {}
  int index = 0;      ///<    subscript in the variable ordering, so lower means closer to the top
  BDD* high = nullptr;      ///<    pointer to the "THEN" bdd
  BDD* low = nullptr;       ///<    pointer to the "ELSE" bdd
  BDD* next = nullptr;      ///<    pointer to the next bdd in the bucket
};

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

BDD** BDDTable = nullptr;

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
#endif