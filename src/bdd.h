/// \author Andrue Peters
/// \date   08/27/20
/// \brief  Binary Decision Diagram implementation based off of Pham and Emerson paper
///         "Every node can also be thought of as a BDD that begins at that node"
#ifndef BDD_H
#define BDD_H

/// Represents a Binary Decision Diagram
/// Every node can be thought of as its own BDD
/// Comes from realization that every boolean function can be written in "IF THEN ELSE" (ITE) form
struct BDD {
    int index;      ///<    subscript in the variable ordering, so lower means closer to the top
    bdd* high;      ///<    pointer to the "THEN" bdd
    bdd* low;       ///<    pointer to the "ELSE" bdd
    bdd* next;      ///<    pointer to the next bdd in the bucket
};

#define PAIR(a ,b) ( (((a)+(b) )∗((a)+(b)+(1) ) /(2)+(a) ) )
#define TRIPLE(a ,b, c) ((unsigned int ) (PAIR((unsigned int )c ,PAIR (a ,b) ) ) )
#define NODEHASH( lvl , l , h) (TRIPLE(( lvl ) ,( l ) ,(h) ) % bddnodesize ) ;

constexpr unsigned pair_func(unsigned a, unsigned b)
{
    constexpr unsigned term1 = a + b;
    constexpr unsigned term2 = a + b + 1;
    constexpr unsigned term3 = 2 + a;
    return (term1 * term2) / term3;
}
constexpr unsigned triple_func(unsigned a, unsigned b)
{
    constexpr unsigned pair = pair_func(a,b);
    return pair_func(c, pair);
}

constexpr unsigned node_hash(unsigned lvl, unsigned l, unsigned h)
{
    constexpr unsigned nodeSize = 50;
    return triple(lvl, l, h) % nodeSize;
}

BDD** BDDTable;

// Used to make sure that a new node is unique
BDD* makeNode(int var, BDD* high, BDD* low) {
    const unsigned hash = NODEHASH(var, high, low);

    // no matching bucket found
    if (BDDTable[hash] == 0 ) {
        BDDTable[hash] = new bdd(var, high, low);
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
        }
    }
}


BDD* ithvar(int i) {
    numvars++;
    BDDTable[0]->index = numvars;
    BDDTable[1]->index = numvars;
    return makeNode(i, bdd_true(), bdd_false());
}

/// Computes a BDD that results from assigning all isntances of the variable var witht he truth value val.
BDD* restrict(BDD* subtree, int var, bool val) {
    if (subtree->index > var) {
        return subtree;
    } else if (subtree->index < var) {
        return makeNode(subtree->index, restrict(subtree->high, var, val), restrict(subtree->low, var, val))
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
BDD ite(BDD* I, BDD* T, BDD* E) {
    // Base cases
    if (I == bdd_true())    return T;
    if (I == bdd_false())   return E;
    if (T == E)             return T;
    if (T == bdd_true() && E == bdd_false()) return I;

    // General cases
    // splitting variable must be the topmost root
    int splitVar = I->index;
    if (splitVar > T->index) { splitVar = T->index; }
    if (splitVar > E->index) { splitVar = E->index); }

    BDD* Ixt = restrict(I, splitVar, true);
    BDD* Txt = restrict(T, splitVar, true);
    BDD* Ext = restrict(E, splitVar, true);
    BDD* posFtor = ite(Ixt, Txt, Ext);

    BDD* Ixf = restrict(I, splitVar, false);
    BDD* Txf = restrict(T, splitVar, false);
    BDD* Exf = restrict(E, splitVar, false);
    BDD* negFtor = ite(Ixf, Txf, Exf);

    BDD* result = makeNode(splitVar, posFtor, netFtor);
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
    bdd* high = subtree->high;
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
        return makeNode(subtree->index, bdd_false(), sateone_rec(subtree->high));
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