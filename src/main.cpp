/// Test for bdd.h

#include "bdd.h"

#include <fstream>
#include <vector>



int main()
{
    /*BinaryDecisionDiagram bdd;
    auto root = bdd.addNthIndex(2);
    auto b1 = bdd.makeNode(3, bdd.one(), bdd.one());
    auto b2 = bdd.makeNode(3, bdd.one(), bdd.zero());
    root->low_ = b1;
    root->high_ = b2;
    const auto dotFormat = toDot(bdd, "bdd");
*/



    BinaryDecisionDiagram otherBdd;
    auto& nodes = otherBdd.getNodes();
    auto *n2 = otherBdd.addNthIndex(2);
    auto *n3 = otherBdd.addNthIndex(3);
    //auto *n4 = otherBdd.addNthIndex(4);
    auto *orOp = otherBdd.xorOperation(n2, n3);
    //auto *idk = otherBdd.andOperation(orOp, n4);
    std::ofstream otherDotOut ("other_bdd.gv");
    otherDotOut << toDot(orOp, nodes[1].get(), nodes[0].get(), "otherBDD");

    std::ofstream dotOut;
    dotOut.open("bdd.gv");
    dotOut << toDot(otherBdd, "bdd");
    return 0;
}