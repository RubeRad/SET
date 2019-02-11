// why is it so hard to get an OpenCL kernel to include an OpenCL include file?
//#include <CL/cl.h> 
typedef unsigned long  cl_ulong;
typedef unsigned short cl_ushort;

// 64 bits is long enough for C(81,12)=70724320184700
typedef cl_ulong BigInt; // cl_ulong is unsigned __int64 in cl_platform.h
// 8 bits is long enough for everything else
// but short prints as an int, so that's easier
typedef cl_ushort SmaInt;


__kernel void num_sets(
   __global const BigInt* DEALI, // which deal, in combinatorial order
   __global       SmaInt* num_sets_deal) // output
{
   int i=get_global_id(0);

   // TBD build up all the code needed here
//    //deal_type d;
//    //unrank_deal(DEALI[i], DEAL_SIZE, &(d.card[0]));
//    //num_sets_deal[i] = num_sets(d, DEAL_SIZE);
   num_sets_deal[i] = DEALI[i]%22;
}


