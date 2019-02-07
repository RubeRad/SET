#include <stdlib.h>
#include <iostream>

using std::cout;
using std::endl;

typedef unsigned long long uint64;

uint64 CHOOSE[82][13];

uint64 choose(int n, int k) {
  if (k==0) return 0; // base cases
  if (k==1) return n;
  uint64 c=1;
  for (int i=0; i<k; ++i) {
    c *= n-i; // careful order of multiplication/division
    c /= i+1; // ensures we always have an integer
  }
  return c;
}

void enumerate(int k) {
  uint64 NUM_DEALS = CHOOSE[81][k];
  cout << "Num deals for " << k << " cards is " << NUM_DEALS << endl;

  int count=0, sign=1;
  for (uint64 deali=0; deali<NUM_DEALS; ++deali) {
    ++count;
    sign *= -1;
  }
}


int main(int argc, char**argv) {

  for   (int n=0; n<=81; ++n)
    for (int k=0; k<=12; ++k)
      CHOOSE[n][k] = choose(n,k);

  for (int i=1; i<argc; ++i) {
    int k = atoi(argv[i]);

    enumerate(k);
  }
}
