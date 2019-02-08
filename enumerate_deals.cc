#include <stdlib.h>
#include <iostream>
#include <sstream>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ostream;
using std::ostringstream;

// 64 bits is long enough for C(81,12)=70724320184700
typedef unsigned long long BigInt;
// 8 bits is long enough for everything else
 // but short prints as an int, so that's easier
typedef unsigned short     SmaInt;

const SmaInt NUM_CARDS=81; // 3^4 for 4 attributes of 3 values
const SmaInt MAX_DEAL=12;  // if technology permits this might be increased
BigInt CHOOSE[NUM_CARDS+1][MAX_DEAL+1]; // C(0,0) up to C(81,12)

// Used to populate CHOOSE[][] up front, then hopefully never again
BigInt combinations(SmaInt n, SmaInt k) {
  if (n<k)  return 0; // base cases
  if (k==0) return 0;
  if (k==1) return n;
  BigInt C=1;
  for (SmaInt i=0; i<k; ++i) {
    C *= n-i; // careful order of multiplication/division
    C /= i+1; // ensures we always have an integer
  }
  return C;
}

// avoid dynamic stl containers for performance
typedef struct card_type_s { SmaInt attr[4]; } card_type;

// a deal is an array of card numbers 0...80, and also a corresponding
// array of card_type structs
typedef struct deal_type_s { card_type card[MAX_DEAL]; } deal_type;

// for card number n=0..80, use modular arithmetic to create a card
// with 4 attribtes valued 0,1,2
card_type
create_card(BigInt n) {
  card_type c;
  c.attr[0] = n/27;  n %= 27;         
  c.attr[1] = n/9 ;  n %= 9;          
  c.attr[2] = n/3 ;  n %= 3;          
  c.attr[3] = n;   
  return c;
}

SmaInt
compute_number(const card_type& c) {
  return c.attr[0]*27 +
         c.attr[1]*9  +
         c.attr[2]*3  +
         c.attr[3];
}

string tostr(const card_type& c) {
  ostringstream ostr;
  ostr << compute_number(c) << "("
       << (c.attr[0] ? c.attr[0] : 3);  // 1 or 2 as-is, 0=3(mod 3)
  switch (c.attr[1]) { case 0: ostr << 'r'; break;   // Red
                       case 1: ostr << 'g'; break;   // Green
                      default: ostr << 'p'; break; } // Purple
  switch (c.attr[2]) { case 0: ostr << 's'; break;   // Solid
                       case 1: ostr << 'o'; break;   // Open
                      default: ostr << 't'; break; } // sTriped
  switch (c.attr[3]) { case 0: ostr << 'c'; break;   // reCtangles
                       case 1: ostr << 'v'; break;   // oVals
                      default: ostr << 'q'; break; } // sQuiggles
  ostr << ")";
  return ostr.str();
}

ostream& operator<<(ostream& ostr, const card_type& c) {
  ostr << tostr(c);
  return ostr;
}
  


// Using the combinatorial number system, convert a number into the
// combination corresponding to that number
// See https://en.wikipedia.org/wiki/Combinatorial_number_system
void unrank_deal(BigInt N, SmaInt k,
		 card_type* card_ptr)
{
  if (k==1) {
    *card_ptr = create_card(N);
    // cout << "N " << N << " k " << k << " card " << *deal << endl;
    return;
  }

  SmaInt n = 0; // find the last n for which C(n,k) <= N
  while (CHOOSE[n][k] <= N)
    ++n;
  // now n>CHOOSE[n][k], so back up 1
  --n;
  *card_ptr = create_card(n);
  // cout << "N " << N << " k " << k << " card " << *deal << endl;

  // unrank smaller combination starting with next pointer
  unrank_deal(N - CHOOSE[n][k], k-1, card_ptr+1);
}





void enumerate(SmaInt k) {
  BigInt NUM_DEALS = CHOOSE[NUM_CARDS][k];
  cout << "Num deals for " << k << " cards is " << NUM_DEALS << endl;

  SmaInt count=0, sign=1;
  for (BigInt deali=0; deali<NUM_DEALS; ++deali) {
    ++count;
    sign *= -1;
  }
}


#define ASSERT_EQ(a,b,m)	                \
  if (a!=b) {                                   \
    cerr << "ERROR (line " << __LINE__ << "): " \
	 << a << "!=" << b << " " << m << endl;	\
    exit(1);                                    \
  }

void self_test() {
  ASSERT_EQ(combinations(81,12), 70724320184700, "C(81,12)");
  
  card_type c = create_card(0);
  for (SmaInt i=0; i<4; ++i)
    ASSERT_EQ(c.attr[i],0, "0");
  ASSERT_EQ(tostr(c), "0(3rsc)", "Card 0 = 3 Red Solid reCtangles");

  c = create_card(1*27 + 2*9 + 0*3 + 1);
  ASSERT_EQ(c.attr[0], 1, "1");
  ASSERT_EQ(c.attr[1], 2, "2");
  ASSERT_EQ(c.attr[2], 0, "0");
  ASSERT_EQ(c.attr[3], 1, "1");
  ASSERT_EQ(tostr(c), "46(1psv)", "Card 46 = 1 Purple Solid oVal");
  
  deal_type deal;
  // the 72nd 5-combination is (8,6,3,1,0)
  unrank_deal(72, 5, &deal.card[0]);
  ASSERT_EQ(compute_number(deal.card[0]), 8, "8");
  ASSERT_EQ(compute_number(deal.card[1]), 6, "6");
  ASSERT_EQ(compute_number(deal.card[2]), 3, "3");
  ASSERT_EQ(compute_number(deal.card[3]), 1, "1");
  ASSERT_EQ(compute_number(deal.card[4]), 0, "0");

#if 0
  SmaInt k=MAX_DEAL;
  for (BigInt N=0; N<5; ++N) {
    cout << k << "-combination " << N << ": ";
    unrank_deal(N, k, &deal.card[0]);
    for (SmaInt i=0; i<k; ++i) cout << deal.card[i] << ',';
    cout << endl;
  }

  for (BigInt N=0; N<5; ++N) {
    BigInt NN = CHOOSE[NUM_CARDS][k]-N-1;
    cout << k << "-combination " << NN << ": ";
    unrank_deal(NN, k, &deal.card[0]);
    for (SmaInt i=0; i<k; ++i) cout << deal.card[i] << ',';
    cout << endl;
  }
#endif
  cout << "Self-tests passed" << endl;
}

int main(int argc, char**argv) {

  for   (SmaInt n=0; n<=NUM_CARDS; ++n)
    for (SmaInt k=0; k<=MAX_DEAL;  ++k)
      CHOOSE[n][k] = combinations(n,k);

  if (1) 
    self_test();

  for (int i=1; i<argc; ++i) {
    SmaInt k = (SmaInt)atoi(argv[i]);

    enumerate(k);
  }
}
