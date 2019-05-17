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

#define NUM_CARDS  81 // 3^4 for 4 attributes of 3 values
#define MAX_DEAL   12 // if technology permits this might be increased
#define NUM_COUNTS 23 // internet sez max SETs in 12 cards is 22

#if DEVICE_CODE
//SNIP
#else
SmaInt DEAL_SIZE;
BigInt CHOOSE[NUM_CARDS+1][MAX_DEAL+1]; // C(0,0) up to C(81,12)
BigInt  THIRD[NUM_CARDS][NUM_CARDS];    // the third card that makes a set with two
#endif


#if HOST_CODE
#define DIE(msg) { cerr << "FATAL ERROR (" << __LINE__ << "): " << msg << endl; exit(1); }
#endif


// avoid dynamic stl containers for performance
typedef struct card_type_s { SmaInt attr[4]; } card_type;

typedef SmaInt card_num; // 0...80

// a deal is an array of card numbers 0...80, and also a corresponding
// array of card_type structs
typedef struct deal_type_s { card_num card[MAX_DEAL]; } deal_type;

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
      deal->card[origk-k] = n;
      N -= CHOOSE[n][k];
   }
   // now k==1, put the result in the last card of the deal
   deal->card[origk-1] = N;
}


// kernel code becomes very inefficient if it has branching and simultaneous
// kernels take different branches. Write increment_deal with no branching
void increment_deal(SmaInt num_cards, deal_type* deal) {
   SmaInt increment_index=MAX_DEAL;      // illegal value
   bool   found_increment_index = false; // haven't found index to increment yet

   // process all but the first card (can't compare first to previous)
   for (SmaInt pos=num_cards-1; pos>0; --pos) {
      bool this_pos_can_increment = (deal->card[pos] < deal->card[pos-1]-1);
      // if (this_pos_can_increment) pos_or_max = pos
      // else                        pos_or_max = MAX_DEAL
      SmaInt pos_or_max = (this_pos_can_increment)*pos +
                         (!this_pos_can_increment)*MAX_DEAL;
      // if (found_increment_index) increment_index = increment_index;
      // else                       increment_index = pos_or_max;
      increment_index = (found_increment_index) * increment_index
                     + (!found_increment_index) * pos_or_max;
      found_increment_index = (increment_index<MAX_DEAL);
   }

   // process first card
   // if (found_increment_index) increment_index = increment_index;
   // else                       increment_index = 0;
   increment_index = (found_increment_index)*increment_index;
   //             + (!found_increment_index*0;

   for (SmaInt pos=0; pos<num_cards; ++pos) {
      // if      (pos< increment_index) card[pos] = card[pos]
      // else if (pos==increment_index) card[pos]+= 1
      // else                           card[pos] = num_cards-1-pos
      deal->card[pos] = (pos< increment_index) * (deal->card[pos])
                      + (pos==increment_index) * (deal->card[pos]+1)
                      + (pos> increment_index) * (num_cards-1-pos);
   }
}


// this function is kinda important
inline bool is_a_set(const card_num a,
                     const card_num b,
                     const card_num c)
{
   return (c==THIRD[a][b]);
}


// This has to iterate through all triples every time
SmaInt num_sets(const deal_type* d, SmaInt kay) {
  SmaInt count=0;
  for (SmaInt i=0;   i<kay; ++i)
  for (SmaInt j=i+1; j<kay; ++j)
  for (SmaInt k=j+1; k<kay; ++k)
     count += (d->card[k] == THIRD[d->card[i]][d->card[j]]);
  return count;
}


#if HOST_CODE
// Compile the same kernel code within the serial program for testing
void num_sets_kernel(
   BigInt taski, // replaces get_global_id(0);
   const BigInt* DEALI,
   BigInt* COUNTS)
{
#else

// The real OpenCL kernel
__kernel void num_sets_kernel(
   __global const BigInt* DEALI,  // Bounds for deal numbers to process
   __global       BigInt* COUNTS) // output, organized as 23 per kernel
{
   int taski=get_global_id(0);
#endif
   BigInt local_counts[NUM_COUNTS];
   for (SmaInt i=0; i< NUM_COUNTS; ++i)
      local_counts[i]=0;

   BigInt NBEG = DEALI[taski];
   BigInt NEND = DEALI[taski+1];
   deal_type d;

   // this is branchy, but maybe unavoidable first time
   unrank_deal_serial(NBEG, DEAL_SIZE, &d);

   // is it necessary/helpful to put a gate right here?
   // or would simultaneous kernels wait for each other anyways?

   // in this main loop there is no branching
   for (BigInt N=NBEG; N<NEND; ++N) {
      SmaInt nSETs = num_sets(&d, DEAL_SIZE);
      local_counts[nSETs]++;
      increment_deal(DEAL_SIZE, &d);
   }

   // After this work item has counted up all its work,
   // write to global memory
   BigInt OFF = taski*NUM_COUNTS;
   for (SmaInt i=0; i<NUM_COUNTS; ++i)
      COUNTS[OFF + i] += local_counts[i];
   
}


