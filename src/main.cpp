/// Test for bdd.h

#include "bdd.h"

#include <fstream>
#include <vector>


int main()
{
    BinaryDecisionDiagramManager otherBdd;
    auto* n2 = otherBdd.addNthIndex(2);
    auto* n3 = otherBdd.addNthIndex(3);
    //auto *n4 = otherBdd.addNthIndex(4);
    auto* orOp = otherBdd.xorOperation(n2, n3);
    //auto *idk = otherBdd.andOperation(orOp, n4);
    std::ofstream otherDotOut("other_bdd.gv");
    otherDotOut << toDot(orOp, otherBdd.one(), otherBdd.zero(), "otherBDD");

    std::ofstream dotOut;
    dotOut.open("bdd.gv");
    dotOut << toDot(otherBdd, "bdd");
    return 0;
}