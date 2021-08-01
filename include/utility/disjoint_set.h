
#pragma once

#include <vector>

struct DisjointSet {
  struct Set {
    int parent;
    unsigned int rank;
  };
  std::vector<Set> sets;

  // Creates a disjoint set for all integers 0 to n-1.
  DisjointSet(unsigned int n);

  // Joins the sets that `a` and `b` are a part of.
  void Union(unsigned int a, unsigned int b);

  // Finds the set that `index` belongs to.
  int Find(unsigned int index);
};
