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



#if DEVICE_CODE
__kernel void num_sets_kernel(
   __global const BigInt* DEALI, // which deal, in combinatorial order
   __global       SmaInt* num_sets_deal) // output
{
   int i=get_global_id(0);

   BigInt test = CHOOSE[81][12];

   // TBD build up all the code needed here
//    //deal_type d;
//    //unrank_deal(DEALI[i], DEAL_SIZE, &(d.card[0]));
//    //num_sets_deal[i] = num_sets(d, DEAL_SIZE);
   num_sets_deal[i] = DEALI[i] % DEAL_SIZE;
}
#endif

