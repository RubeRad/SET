#include <stdlib.h>
#include <iostream>

using std::cout;
using std::endl;

typedef long long uint64;

void enumerate(int n) {
  uint64 NUM_DEALS = 1;
  for (int i=1; i<=n; ++i) {
    int topno = 81-(i-1);
    NUM_DEALS *= topno;
    NUM_DEALS /= i;
  }

  cout << "Num deals for " << n << " cards is " << NUM_DEALS << endl;
  int count=0, sign=1;
  for (uint64 deali=0; deali<NUM_DEALS; ++deali) {
    ++count;
    sign *= -1;
  }
}


int main(int argc, char**argv) {
  for (int i=1; i<argc; ++i) {
    int n = atoi(argv[i]);

    enumerate(n);

  }
}
