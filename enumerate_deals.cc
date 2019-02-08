#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <map>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ostream;
using std::ostringstream;
using std::map;

#define DIE(msg) { cerr << "FATAL ERROR (" << __LINE__ << "): " << msg << endl; exit(1); }

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

card_type
create_card(SmaInt number,
	    SmaInt color,
	    SmaInt texture,
	    SmaInt shape)
{
  return create_card(27*(number%3) +
		      9*color      +
		      3*texture    +
		        shape);
}

SmaInt
compute_number(const card_type& c) {
  return c.attr[0]*27 +
         c.attr[1]*9  +
         c.attr[2]*3  +
         c.attr[3];
}


const SmaInt GRN = 0;  // alphabetical colors
const SmaInt PPL = 1;
const SmaInt RED = 2;
const SmaInt OPN = 0; // because 0 looks like O, and open is the least filled
const SmaInt STR = 1; // because striped is intermediate-filled
const SmaInt SOL = 2; // because solid is most filled
const SmaInt OVL = 0; // because 0 looks like O
const SmaInt REC = 1; // because a 1 is pretty straight-lined
const SmaInt SQG = 2; // because a 2 is pretty squiggly
const SmaInt DMD = 1; // Joy of Set has Diamonds, not Rectangles!

string tostr(const card_type& c) {
  ostringstream ostr;
  ostr << compute_number(c) << "("
       << (c.attr[0] ? c.attr[0] : 3);  // 1 or 2 as-is, 0=3(mod 3)
  switch (c.attr[1]) { case GRN: ostr << 'g'; break;
                       case PPL: ostr << 'p'; break;  
                       case RED: ostr << 'r'; break;
                        default: DIE("Unknown color");
  }
  switch (c.attr[2]) { case OPN: ostr << 'o'; break;  
                       case STR: ostr << 't'; break;
                       case SOL: ostr << 's'; break;  
                        default: DIE("Unknown color");
  }
  switch (c.attr[3]) { case OVL: ostr << 'v'; break;  
                       case REC: ostr << 'c'; break;  
                       case SQG: ostr << 'q'; break;  
                        default: DIE("Unknown shape");
  }
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



// this function is kinda important
bool is_a_set(const card_type& a,
	      const card_type& b,
	      const card_type& c)
{
  if ( (a.attr[0] + b.attr[0] + c.attr[0]) % 3 )
    return false;
  if ( (a.attr[1] + b.attr[1] + c.attr[1]) % 3 )
    return false;
  if ( (a.attr[2] + b.attr[2] + c.attr[2]) % 3 )
    return false;
  if ( (a.attr[3] + b.attr[3] + c.attr[3]) % 3 )
    return false;

  // if we make it here, the card sums to 0mod3 in all three attributes
  return true;
}


// shortcuts a true after a first set is found
bool has_a_set(const deal_type& d, SmaInt kay) {
  for (SmaInt i=0;   i<kay; ++i)
  for (SmaInt j=i+1; j<kay; ++j)
  for (SmaInt k=j+1; k<kay; ++k)
    if (is_a_set(d.card[i],
		 d.card[j],
		 d.card[k]))
      return true;

  // if it makes it all the way through the triple loop, then no sets
  return false;
}

// Unfortunately this has to iterate through all triples every time
SmaInt num_sets(const deal_type& d, SmaInt kay) {
  SmaInt count=0;
  for (SmaInt i=0;   i<kay; ++i)
  for (SmaInt j=i+1; j<kay; ++j)
  for (SmaInt k=j+1; k<kay; ++k)
    if (is_a_set(d.card[i],
		 d.card[j],
		 d.card[k])) 
      ++count;

  return count;
}

void enumerate(SmaInt k) {
  BigInt NUM_DEALS = CHOOSE[NUM_CARDS][k];
  cout << "Num deals for " << k << " cards is " << NUM_DEALS << endl;

  map<SmaInt, BigInt> counts;
  deal_type d;
  for (BigInt deali=0; deali<NUM_DEALS; ++deali) {
    unrank_deal(deali, k, &(d.card[0]));
    SmaInt ns = num_sets(d, k);

    auto iter = counts.find(ns);
    if (iter == counts.end()) counts[ns]  = 1;
    else                    iter->second += 1;
  }

  BigInt total=0;
  for (const auto& ns_n : counts) {
    cout << ns_n.first << " sets: " << ns_n.second << endl;
    total += ns_n.second;
  }
  cout << "Total: " << total << endl << endl;
}


#define ASSERT_EQ(a,b,m)	                \
  if (a!=b) {                                   \
    cerr << "ERROR (line " << __LINE__ << "): " \
	 << a << "!=" << b << " " << m << endl;	\
    exit(1);                                    \
  }

#define ASSERT(b,m)                             \
  if (!b) {                                     \
    cerr << "ERROR (line " << __LINE__ << "): " \
	 << "false! " << m << endl;             \
    exit(1);                                    \
  }

void self_test() {
  ASSERT_EQ(combinations(81,12), 70724320184700, "C(81,12)");
  
  card_type c = create_card(0);
  for (SmaInt i=0; i<4; ++i)
    ASSERT_EQ(c.attr[i],0, "0");
  ASSERT_EQ(tostr(c), "0(3gov)", "Card 0 = 3 Green Open Ovals");

  c = create_card(80);
  for (SmaInt i=0; i<4; ++i)
    ASSERT_EQ(c.attr[i],2, "2");
  ASSERT_EQ(tostr(c), "80(2rsq)", "Card 80 = 2 Red Solid Squiggles");

  card_type c1psv = create_card(1, PPL, SOL, OVL);
  ASSERT_EQ(c1psv.attr[0], 1, "1");
  ASSERT_EQ(c1psv.attr[1], 1, "1");
  ASSERT_EQ(c1psv.attr[2], 2, "2");
  ASSERT_EQ(c1psv.attr[3], 0, "0");
  ASSERT_EQ(tostr(c1psv), "42(1psv)", "Card 42 = 1 Purple Solid oVal");
  
  // build all-different set with c1psv
  card_type c2roc = create_card(2, RED, OPN, REC);
  card_type c3gtq = create_card(3, GRN, STR, SQG);
  ASSERT(is_a_set(c1psv, c2roc, c3gtq), "all-different attribute set");

  // 3-different set with c1psv, all ovals
  card_type c2rov = create_card(2, RED, OPN, OVL);
  card_type c3gtv = create_card(3, GRN, STR, OVL);
  ASSERT(is_a_set(c1psv, c2rov, c3gtv), "3-different attribute set");

  // 2-different set with c1psv, all one solids
  card_type c1rsq = create_card(1, RED, SOL, SQG);
  card_type c1gsc = create_card(1, GRN, SOL, REC);
  ASSERT(is_a_set(c1psv, c1rsq, c1gsc), "2-different attribute set");

  // 1-different set with c1psv, all one purple oval
  card_type c1pov = create_card(1, PPL, OPN, OVL);
  card_type c1ptv = create_card(1, PPL, STR, OVL);
  ASSERT(is_a_set(c1psv, c1pov, c1ptv), "1-different attribute set");
  
  // not a set
  ASSERT(!is_a_set(c1psv, c2roc, c3gtv), "Not a set");
  

  deal_type deal;
  // the 72nd 5-combination is (8,6,3,1,0)
  unrank_deal(72, 5, &deal.card[0]);
  ASSERT_EQ(compute_number(deal.card[0]), 8, "8");
  ASSERT_EQ(compute_number(deal.card[1]), 6, "6");
  ASSERT_EQ(compute_number(deal.card[2]), 3, "3");
  ASSERT_EQ(compute_number(deal.card[3]), 1, "1");
  ASSERT_EQ(compute_number(deal.card[4]), 0, "0");


  // set up the plane in Joy of Set fig 1.21
  deal.card[0] = create_card(1, GRN, STR, DMD);
  deal.card[1] = create_card(3, PPL, SOL, SQG);
  deal.card[2] = create_card(2, RED, STR, OVL);
  deal.card[3] = create_card(2, RED, OPN, OVL);
  deal.card[4] = create_card(3, PPL, STR, SQG);
  deal.card[5] = create_card(1, GRN, SOL, DMD);
  deal.card[6] = create_card(3, PPL, OPN, SQG);
  deal.card[7] = create_card(2, RED, SOL, OVL);
  deal.card[8] = create_card(1, GRN, OPN, DMD);
  ASSERT  (has_a_set(deal,9),     "Plane has set(s)");
  ASSERT_EQ(num_sets(deal,9), 12, "Plane has 12 sets");

  
  
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
