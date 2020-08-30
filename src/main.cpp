/// Test for bdd.h

#include "bdd.h"

#include <fstream>
#include <vector>



int main()
{
    BinaryDecisionDiagram bdd;
    auto root = bdd.addNthIndex(2);
    auto b1 = bdd.makeNode(3, bdd.one(), bdd.one());
    auto b2 = bdd.makeNode(3, bdd.one(), bdd.zero());
    root->low_ = b1;
    root->high_ = b2;
    const auto dotFormat = toDot(bdd, "bdd");


    std::ofstream dotOut;
    dotOut.open("bdd.gv");
    dotOut << dotFormat;
    return 0;
}