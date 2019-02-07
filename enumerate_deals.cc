#include <stdlib.h>
#include <iostream>

using std::cout;
using std::endl;

const int NUM_CARDS=81; // 3^4 for 4 attributes of 3 values
const int MAX_DEAL=12;  // if technology permits this might be increased

// 64 bits is long enough for C(81,12)=70724320184700
typedef unsigned long long uint64;
uint64 CHOOSE[NUM_CARDS+1][MAX_DEAL+1]; // C(0,0) up to C(81,12)

// avoid dynamic stl containers for performance
struct  deal_type_struct { int card[MAX_DEAL]; };
typedef deal_type_struct deal_type;


// Used to populate CHOOSE[][] up front, then hopefully never again
uint64 combinations(int n, int k) {
  if (n<k)  return 0; // base cases
  if (k==0) return 0;
  if (k==1) return n;
  uint64 C=1;
  for (int i=0; i<k; ++i) {
    C *= n-i; // careful order of multiplication/division
    C /= i+1; // ensures we always have an integer
  }
  return C;
}


// Using the combinatorial number system, convert a number into the
// combination corresponding to that number
// See https://en.wikipedia.org/wiki/Combinatorial_number_system
void unrank_deal(uint64 N, int k,
		 int* deal)
{
  if (k==1) {
    *deal=N;
    // cout << "N " << N << " k " << k << " card " << *deal << endl;
    return;
  }

  int n = 0; // find the last n for which C(n,k) <= N
  while (CHOOSE[n][k] <= N)
    ++n;
  // now n>CHOOSE[n][k], so back up 1
  --n;
  *deal = n; // set this value
  // cout << "N " << N << " k " << k << " card " << *deal << endl;

  // unrank smaller combination starting with next pointer
  unrank_deal(N - CHOOSE[n][k], k-1, deal+1);
}





void enumerate(int k) {
  uint64 NUM_DEALS = CHOOSE[NUM_CARDS][k];
  cout << "Num deals for " << k << " cards is " << NUM_DEALS << endl;

  int count=0, sign=1;
  for (uint64 deali=0; deali<NUM_DEALS; ++deali) {
    ++count;
    sign *= -1;
  }
}


int main(int argc, char**argv) {

  for   (int n=0; n<=NUM_CARDS; ++n)
    for (int k=0; k<=MAX_DEAL;  ++k)
      CHOOSE[n][k] = combinations(n,k);

  deal_type deal;
  for (int k=1; k<=5; ++k) {
    for (uint64 N=0; N<120; ++N) {
      cout << k << "-combination " << N << ": ";
      unrank_deal(N, k, &deal.card[0]);
      for (int i=0; i<k; ++i) cout << deal.card[i] << ',';
      cout << endl;
    }
  }

  int k=MAX_DEAL;
  for (uint64 N=0; N<50; ++N) {
    cout << k << "-combination " << N << ": ";
    unrank_deal(N, k, &deal.card[0]);
    for (int i=0; i<k; ++i) cout << deal.card[i] << ',';
    cout << endl;
  }

  for (uint64 N=0; N<50; ++N) {
    uint64 NN = CHOOSE[NUM_CARDS][k]-N-1;
    cout << k << "-combination " << NN << ": ";
    unrank_deal(NN, k, &deal.card[0]);
    for (int i=0; i<k; ++i) cout << deal.card[i] << ',';
    cout << endl;
  }
    
  

  for (int i=1; i<argc; ++i) {
    int k = atoi(argv[i]);

    enumerate(k);
  }
}
