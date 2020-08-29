/// Test for bdd.h

#include "bdd.h"

#include <vector>

// Return a vector of BDDs
std::vector<BDD*> makeNBDD(unsigned n)
{
  std::vector<BDD*> bdds;
  bdds.reserve(n);
  for(auto i = 0u; i < n; ++i) {
    BDD* newBDD = ithvar(static_cast<int>(i));
    bdds.push_back(newBDD);
  }
  return bdds;
}

int main()
{
  BDDTable = new BDD*[100];
  auto bdds = makeNBDD(3);
  return 0;
}