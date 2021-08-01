
#include "utility/disjoint_set.h"

DisjointSet::DisjointSet(unsigned int n) {
  sets.resize(n);
  for (int i = 0; i < n; i++) {
    sets[i].parent = i;
    sets[i].rank = 0;
  }
}

void DisjointSet::Union(unsigned int a, unsigned int b) {
  unsigned int set_a = Find(a), set_b = Find(b);
  if (set_a == set_b) {
    return;
  }
  if (sets[set_a].rank < sets[set_b].rank) {
    sets[set_a].parent = set_b;
  } else if (sets[set_a].rank > sets[set_b].rank) {
    sets[set_b].parent = set_a;
  } else {
    sets[set_a].parent = set_b;
    sets[set_b].rank += 1;
  }
}

int DisjointSet::Find(unsigned int index) {
  int value = sets[index].parent;
  if (value == index) {
    return value;
  } else {
    sets[index].parent = Find(value);
    return sets[index].parent;
  }
}
