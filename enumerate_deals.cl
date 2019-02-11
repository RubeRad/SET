// why is it so hard to get an OpenCL kernel to include an OpenCL include file?
//#include <CL/cl.h>

#if DEVICE_CODE
typedef unsigned long  cl_ulong;
typedef unsigned short cl_ushort;
#endif

// 64 bits is long enough for C(81,12)=70724320184700
typedef cl_ulong BigInt; // cl_ulong is unsigned __int64 in cl_platform.h
// 8 bits is long enough for everything else
// but short prints as an int, so that's easier
typedef cl_ushort SmaInt;

#if DEVICE_CODE
#define DUAL_CONST __constant const
#else
#define DUAL_CONST            const
#endif

DUAL_CONST SmaInt NUM_CARDS=81; // 3^4 for 4 attributes of 3 values
DUAL_CONST SmaInt MAX_DEAL=12;  // if technology permits this might be increased

#if DEVICE_CODE
//SNIP
#else
SmaInt DEAL_SIZE;
BigInt CHOOSE[NUM_CARDS+1][MAX_DEAL+1]; // C(0,0) up to C(81,12)
#endif


#if HOST_CODE
#define DIE(msg) { cerr << "FATAL ERROR (" << __LINE__ << "): " << msg << endl; exit(1); }
#endif


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

// extra crispy (device code cannot recurse)
void unrank_deal_serial(BigInt origN, SmaInt origk,
                        deal_type* deal)
{
   BigInt N = origN;
   for (SmaInt k=origk; k>1; --k) {
      SmaInt n=0; // find last n for which C(n,k) <= N
      while (CHOOSE[n][k] <= N)
         ++n;
      // now n>CHOOSE[n][k], so back up 1
      --n;
      deal->card[origk-k] = create_card(n);
      N -= CHOOSE[n][k];
   }
   // now k==1, put the result in the last card of the deal
   deal->card[origk-1] = create_card(N);
}


// this function is kinda important
bool is_a_set(const card_type* a,
              const card_type* b,
              const card_type* c)
{
  if ( (a->attr[0] + b->attr[0] + c->attr[0]) % 3 )
    return false;
  if ( (a->attr[1] + b->attr[1] + c->attr[1]) % 3 )
    return false;
  if ( (a->attr[2] + b->attr[2] + c->attr[2]) % 3 )
    return false;
  if ( (a->attr[3] + b->attr[3] + c->attr[3]) % 3 )
    return false;

  // if we make it here, the card sums to 0mod3 in all three attributes
  return true;
}


// This has to iterate through all triples every time
SmaInt num_sets(const card_type* cards, SmaInt kay) {
  SmaInt count=0;
  for (SmaInt i=0;   i<kay; ++i)
  for (SmaInt j=i+1; j<kay; ++j)
  for (SmaInt k=j+1; k<kay; ++k)
    if (is_a_set(cards+i, cards+j, cards+k)) 
       ++count;
  return count;
}


#if DEVICE_CODE
__kernel void num_sets_kernel(
   __global const BigInt* DEALI, // which deal, in combinatorial order
   __global       SmaInt* num_sets_deal) // output
{
   int i=get_global_id(0);

   BigInt N = DEALI[i];
   SmaInt k = DEAL_SIZE;
   deal_type d;
   unrank_deal_serial(N, k, &d);
   num_sets_deal[i] = num_sets(&d.card[0], DEAL_SIZE);
}
#endif

