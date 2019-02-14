#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <sstream>
#include <vector>

// from Intel:
// intel_sdk_for_opencl_2017_7.0.0.2568_x64.tgz
// opencl_runtime_16.1.2_x64_rh_6.4.0.37.tgz
#include <CL/cl.h>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ostream;
using std::ostringstream;
using std::vector;

#define DEVICE_CODE 0
#define   HOST_CODE 1
#include "enumerate_deals.cl"


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

SmaInt
compute_number(SmaInt number,
               SmaInt color,
               SmaInt texture,
               SmaInt shape) {
   return (27 * (number%3) +
            9 * color      +
            3 * texture    +
           shape);
}

card_num compute_third(const card_num anum,
                       const card_num bnum)
{
   card_type a = create_card(anum);
   card_type b = create_card(bnum);
   card_type c;
   for (SmaInt i=0; i<4; ++i) {
      SmaInt complement = (a.attr[i]+b.attr[i]) % 3;
      c.attr[i] = (3 - complement) % 3;
   }
   return compute_number(c);
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
// original flavor (recursive)
void unrank_deal(BigInt N, SmaInt k,
                 card_num* card_ptr)
{
  if (k==1) {
    *card_ptr = N;
    // cout << "N " << N << " k " << k << " card " << *deal << endl;
    return;
  }

  SmaInt n = 0; // find the last n for which C(n,k) <= N
  while (CHOOSE[n][k] <= N)
    ++n;
  // now n>CHOOSE[n][k], so back up 1
  --n;
  *card_ptr = n;
  // cout << "N " << N << " k " << k << " card " << *deal << endl;

  // unrank smaller combination starting with next pointer
  unrank_deal(N - CHOOSE[n][k], k-1, card_ptr+1);
}


// this is slower, now that we switched to cards stored just as numbers 0..80,
// and is_a_set just checking the precomputed table of THIRDs
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

inline bool is_a_set(const card_type& a,
                     const card_type& b,
                     const card_type& c)
  { return is_a_set(&a, &b, &c); } 



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


// shortcuts a true after a first set is found
bool has_a_set(const card_type* cards, SmaInt kay) {
  for (SmaInt i=0;   i<kay; ++i)
  for (SmaInt j=i+1; j<kay; ++j)
  for (SmaInt k=j+1; k<kay; ++k)
    if (is_a_set(cards[i],
                 cards[j],
                 cards[k]))
      return true;

  // if it makes it all the way through the triple loop, then no sets
  return false;
}

// shortcuts a true after a first set is found
bool has_a_set(const deal_type& d, SmaInt kay) {
  for (SmaInt i=0;   i<kay; ++i)
  for (SmaInt j=i+1; j<kay; ++j)
  for (SmaInt k=j+1; k<kay; ++k)
    if (is_a_set(d.card[i], d.card[j], d.card[k]))
      return true;
  // if it makes it all the way through the triple loop, then no sets
  return false;
}




////////////////////////////////////////////////////////////////////////
// Unit tests. They're fast enough, might as well run them every time
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

  for (SmaInt a=0; a<NUM_CARDS; ++a)
  for (SmaInt b=0; b<NUM_CARDS; ++b)
     ASSERT(is_a_set(a,b, THIRD[a][b]), "THIRD card verification");

  deal_type recurse, serial;
  unrank_deal       (12345678, 12, &(recurse.card[0])); // trusted
  unrank_deal_serial(12345678, 12, &serial); // tested
  for (int i=0; i<12; ++i)   // for all 12 cards
        ASSERT_EQ(recurse.card[i],
                   serial.card[i], "serial unranking matches recursive");
  
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
  // Example from wiki page: the 72nd 5-combination is (8,6,3,1,0)
  unrank_deal(72, 5, &deal.card[0]);
  ASSERT_EQ(deal.card[0], 8, "8");
  ASSERT_EQ(deal.card[1], 6, "6");
  ASSERT_EQ(deal.card[2], 3, "3");
  ASSERT_EQ(deal.card[3], 1, "1");
  ASSERT_EQ(deal.card[4], 0, "0");

  // set up the plane in Joy of Set fig 1.21
  deal.card[0] = compute_number(1, GRN, STR, DMD);
  deal.card[1] = compute_number(3, PPL, SOL, SQG);
  deal.card[2] = compute_number(2, RED, STR, OVL);
  deal.card[3] = compute_number(2, RED, OPN, OVL);
  deal.card[4] = compute_number(3, PPL, STR, SQG);
  deal.card[5] = compute_number(1, GRN, SOL, DMD);
  deal.card[6] = compute_number(3, PPL, OPN, SQG);
  deal.card[7] = compute_number(2, RED, SOL, OVL);
  deal.card[8] = compute_number(1, GRN, OPN, DMD);
  ASSERT   (has_a_set(deal,9),     "Plane has set(s)");
  ASSERT_EQ(num_sets(&deal,9), 12, "Plane has 12 sets");

  // set up the hyperplane in Joy of Set fig 5.25
  card_type hp[27];
  SmaInt i=0;
  hp[i++]=create_card(3,GRN,SOL,OVL); hp[i++]=create_card(2,GRN,SOL,SQG); hp[i++]=create_card(1,GRN,SOL,DMD);
  hp[i++]=create_card(2,PPL,OPN,OVL); hp[i++]=create_card(1,PPL,OPN,SQG); hp[i++]=create_card(3,PPL,OPN,DMD);
  hp[i++]=create_card(1,RED,STR,OVL); hp[i++]=create_card(3,RED,STR,SQG); hp[i++]=create_card(2,RED,STR,DMD);

  hp[i++]=create_card(1,PPL,STR,OVL); hp[i++]=create_card(3,PPL,STR,SQG); hp[i++]=create_card(2,PPL,STR,DMD);
  hp[i++]=create_card(3,RED,SOL,OVL); hp[i++]=create_card(2,RED,SOL,SQG); hp[i++]=create_card(1,RED,SOL,DMD);
  hp[i++]=create_card(2,GRN,OPN,OVL); hp[i++]=create_card(1,GRN,OPN,SQG); hp[i++]=create_card(3,GRN,OPN,DMD);

  hp[i++]=create_card(2,RED,OPN,OVL); hp[i++]=create_card(1,RED,OPN,SQG); hp[i++]=create_card(3,RED,OPN,DMD);
  hp[i++]=create_card(1,GRN,STR,OVL); hp[i++]=create_card(3,GRN,STR,SQG); hp[i++]=create_card(2,GRN,STR,DMD);
  hp[i++]=create_card(3,PPL,SOL,OVL); hp[i++]=create_card(2,PPL,SOL,SQG); hp[i++]=create_card(1,PPL,SOL,DMD);
  ASSERT_EQ(num_sets(hp,27), 117, "Hyperplane has 117 sets");
  
  // set up the maximal cap in Joy of Set fig 5.23
  card_type mc[20];
  i=0;
  mc[i++]=create_card(2,GRN,SOL,SQG); mc[i++]=create_card(2,RED,STR,DMD); mc[i++]=create_card(3,RED,OPN,SQG);
  mc[i++]=create_card(1,GRN,STR,DMD); mc[i++]=create_card(3,RED,SOL,SQG); mc[i++]=create_card(2,RED,STR,OVL);
  mc[i++]=create_card(3,RED,STR,DMD); mc[i++]=create_card(1,GRN,SOL,SQG); mc[i++]=create_card(2,GRN,OPN,DMD);

  mc[i++]=create_card(1,GRN,OPN,DMD); mc[i++]=create_card(1,RED,SOL,SQG); mc[i++]=create_card(3,GRN,STR,DMD);
  mc[i++]=create_card(2,GRN,SOL,OVL); mc[i++]=create_card(2,GRN,OPN,SQG); mc[i++]=create_card(2,RED,OPN,DMD);
  mc[i++]=create_card(2,RED,OPN,SQG); mc[i++]=create_card(3,PPL,OPN,SQG); mc[i++]=create_card(1,PPL,OPN,DMD);

  mc[i++]=create_card(3,RED,SOL,DMD); mc[i++]=create_card(1,GRN,STR,SQG);
  ASSERT(!has_a_set(mc,20), "Maximal cap has no set");
  
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



////////////////////////////////////////////////////////


// These are the two main versions of the function


void enumerate_serial(SmaInt k,
                      BigInt BATCHSIZE)
{
   DEAL_SIZE=k;
   BigInt NUM_DEALS = CHOOSE[NUM_CARDS][k];
   cout << "Num deals for " << k << " cards is " << NUM_DEALS << endl;

   BigInt TOTAL_COUNTS[NUM_COUNTS];
   for (SmaInt i=0;  i<NUM_COUNTS;  ++i)
      TOTAL_COUNTS[i] = 0;

   deal_type d;
   double t0 = clock();
   for (BigInt N=0; N<NUM_DEALS; ++N) {
      unrank_deal_serial(N, k, &d);
      SmaInt nSETs = num_sets(&d, k);
      TOTAL_COUNTS[nSETs]++;

      // intermittent reporting
      BigInt DONE=N+1;
      if (DONE==NUM_DEALS || (DONE%BATCHSIZE==0)) {
         double seconds = (clock()-t0)/CLOCKS_PER_SEC;
         double frac = (DONE*1.0)/NUM_DEALS;
         if (DONE==NUM_DEALS) cout << "FINAL,"<<k<<","<<DONE<<","<<seconds;
         else                 cout << frac    <<k<<","<<DONE<<","<<seconds;
         for (SmaInt i=0; i<NUM_COUNTS; ++i) {
            cout << ",";
            if (TOTAL_COUNTS[i])
               cout << TOTAL_COUNTS[i];
         }
         cout << endl;
      }
   }
}


void enumerate(SmaInt k,         // deal size
               BigInt PARALLELS, // number of parallel units to task
               BigInt BATCHSIZE) // number of deals per task
{
   DEAL_SIZE=k;
   BigInt NUM_DEALS = CHOOSE[NUM_CARDS][k];
   cout << "Num deals for " << k << " cards is " << NUM_DEALS << endl;

   // starting/ending deal numbers for each unit
   BigInt* DEALI_VEC = new BigInt[PARALLELS+1];
   // kernel results; NUM_COUNTS per 
   BigInt* PARALLEL_COUNTS = new BigInt[PARALLELS*NUM_COUNTS];
   BigInt TOTAL_COUNTS[NUM_COUNTS];
   for (SmaInt i=0;  i<NUM_COUNTS;  ++i) {
      TOTAL_COUNTS[i] = 0;
      for (BigInt J=0; J<PARALLELS; ++J)
         PARALLEL_COUNTS[J*NUM_COUNTS + i] = 0;
   }

   // Get platform and device information
   cl_platform_id platform_id = NULL;
   cl_device_id device_id = NULL;   
   cl_uint ret_num_devices;
   cl_uint ret_num_platforms;
   cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
   cout << "clGetPlatformIDs returns " << ret << endl;
    
   ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, 
                        &device_id, &ret_num_devices);
   cout << "clGetDeviceIDs returns " << ret << endl;

   // Create an OpenCL context
   cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);
   cout << "clCreateContext returns " << ret << endl;
   
   // Create a command queue
#ifdef WINDOZE
   // not sure why the version of OpenCL I have for windoze prefers the deprecated function
   cl_command_queue command_queue = clCreateCommandQueue              (context, device_id, NULL, &ret);
#else
   // OpenCL for Linux compiles the old function but gives a warning
   cl_command_queue command_queue = clCreateCommandQueueWithProperties(context, device_id, NULL, &ret);
#endif
   cout << "clCreateCommandQueueWithProperties returns " << ret << endl;

   // Create memory buffers on the device for each vector 
   cl_mem inn_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, 
                  (PARALLELS+1) * sizeof(BigInt),          NULL, &ret);
   cout << "clClCreateBuffer(inn) returns " << ret << endl;
   
   cl_mem out_obj = clCreateBuffer(context, CL_MEM_READ_WRITE,
                  (PARALLELS*NUM_COUNTS) * sizeof(BigInt), NULL, &ret);
   cout << "clClCreateBuffer(out) returns " << ret << endl;

   // make sure device-side COUNTS buffer is initialized with zeroes
   ret = clEnqueueWriteBuffer(command_queue, out_obj, CL_TRUE, 0,
           (PARALLELS*NUM_COUNTS) * sizeof(BigInt), PARALLEL_COUNTS, 0, NULL, NULL);;
   cout << "clClEnqueueWriteBuffer returns " << ret << endl;
   

   FILE* fp;
   fp = fopen("enumerate_deals.cl", "r");
   if (!fp) {
      fprintf(stderr, "Failed to load kernel.\n");
      exit(1);
   }
   const int MAX_SOURCE_SIZE = 1000000;
   char* source_txt = new char[MAX_SOURCE_SIZE];
   size_t source_size = fread(source_txt, 1, MAX_SOURCE_SIZE, fp);
   fclose(fp);
   cout << "Read CL source length " << source_size << endl;

   string source_str = source_txt;
   size_t pos = source_str.find("//SNIP");
   
   

   ostringstream ostr;
   ostr << "#define DEVICE_CODE 1\n"
        << "#define   HOST_CODE 0\n"
        << source_str.substr(0,pos)
        << "__constant const SmaInt DEAL_SIZE="<<k<<";\n"
        << "__constant const BigInt CHOOSE[" << NUM_CARDS+1 << "]["
        << MAX_DEAL+1 << "] = {\n";

   for (int top=0; top<=NUM_CARDS; ++top) {
      ostr << "   {" << CHOOSE[top][0];
      for (int bot=1; bot<=MAX_DEAL; ++bot)
         ostr << ", " << CHOOSE[top][bot];
      ostr << (top < NUM_CARDS ? "},\n" : "}\n");
   }
   ostr << "};\n\n";

   ostr << "__constant const SmaInt THIRD[" << NUM_CARDS << "]["
        << NUM_CARDS << "] = {\n";
   for (int a=0; a<NUM_CARDS; ++a) {
      ostr << "   {" << THIRD[a][0];
      for (int b=1; b<NUM_CARDS; ++b)
         ostr << ", " << THIRD[a][b];
      ostr << (a < NUM_CARDS-1 ? "},\n" : "}\n");
   }
   ostr << "};\n\n";

   ostr << source_str.substr(pos);

   source_str = ostr.str();
   source_size = source_str.length();
   const char* source_ptr = source_str.c_str();

   cout << "Full source\n\n" << source_str << "\n\n";
   
   // Create and build the program
   cl_program program = clCreateProgramWithSource(context, 1, 
       &source_ptr, (const size_t *)&source_size, &ret);
   cout << "clCreateProgramWithSource returns " << ret << endl;
   
   ret = clBuildProgram(program, 1, &device_id,
                        "-I/usr/include/x86_64-linux-gnu",
                        NULL, NULL);
   cout << "clBuildProgram returns " << ret << endl;

   char log[2048];
   size_t logsize;
   ret = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG,
                               sizeof(log), log, &logsize);
   cout << "Build log:\n" << log << endl;
   if (ret != CL_SUCCESS)
      return;

   // Create the OpenCL kernel
   cl_kernel kernel = clCreateKernel(program, "num_sets_kernel", &ret);
   cout << "clCreateKernel returns " << ret << endl;
   
   // Set the arguments of the kernel
   ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&inn_obj);
   cout << "clSetKernelArg(inn) returns " << ret << endl;
   ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&out_obj);
   cout << "clSetKernelArg(out) returns " << ret << endl;
   
   BigInt OFF=0;

   //NUM_DEALS = 100000000; // only for test-running the first 'few'

   double t0 = clock();
   while (OFF < NUM_DEALS) {
      // OFF is number processed so far,

      BigInt NUM = PARALLELS*BATCHSIZE; // how many to do in a full loop
      if (OFF+NUM > NUM_DEALS)          // last one may not be full
         NUM = NUM_DEALS-OFF;
      // NUM is how many to process this loop

      BigInt PER = NUM / PARALLELS;
      if (PER*PARALLELS < NUM)
         PER +=1;

      // Populate vector of deal numbers to process
      // task i will work on DEALI_VEC[i]<=j<DEALI_VEC[i+1]
      for (BigInt I=0; I<PARALLELS; ++I) {
         DEALI_VEC[I] = OFF + I*PER;
         //cout << "task " << I << " starts with " << DEALI_VEC[I] << endl;
      }
      DEALI_VEC[PARALLELS] = OFF+NUM;
      //cout << "task " << PARALLELS-1 << " ends with " << DEALI_VEC[PARALLELS] << endl;
      
      // copy DEALI_VEC into device buffer
      ret = clEnqueueWriteBuffer(command_queue, inn_obj, CL_TRUE, 0,
              (PARALLELS+1) * sizeof(BigInt), DEALI_VEC, 0, NULL, NULL);;
      if (ret!=CL_SUCCESS)
         cout << "clClEnqueueWriteBuffer returns " << ret << endl;

      // Execute kernels
      size_t global_item_size=PARALLELS, local_item_size = PARALLELS;
      ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, 
            &global_item_size, &local_item_size, 0, NULL, NULL);
      if (ret!=CL_SUCCESS)
         cout << "clEnqueueNDRangeKernel returns " << ret << endl;

      // copy answers from device buffer
      ret = clEnqueueReadBuffer(command_queue, out_obj, CL_TRUE, 0, 
                (PARALLELS*NUM_COUNTS) * sizeof(BigInt), PARALLEL_COUNTS, 0, NULL, NULL);
      if (ret!=CL_SUCCESS)
         cout << "clClEnqueueReadBuffer returns " << ret << endl;

      // tabulate this batch of ansas
      for (SmaInt j=0; j<NUM_COUNTS; ++j) {
         TOTAL_COUNTS[j] = 0; // reset to 0, PARALLEL_COUNTS are cumulative
         for (BigInt I=0; I<PARALLELS; ++I) {
            TOTAL_COUNTS[j] += PARALLEL_COUNTS[I*NUM_COUNTS + j];
            //cout << I << " " << j << " " << PARALLEL_COUNTS[I*NUM_COUNTS + j] << endl;
         }
      }
            

      // Now that we are done with this batch, roll NUM into OFF
      OFF += NUM;
      // Once again OFF is the number of deals processed so far
      // if OFF==NUM_DEALS this is the last time through the while loop
      
      // intermittent reporting
      double seconds = (clock()-t0)/CLOCKS_PER_SEC;
      double frac = (OFF*1.0)/NUM_DEALS;
      if (OFF==NUM_DEALS) cout << "FINAL,"<<k<<","<<OFF<<","<<seconds;
      else                cout << frac    <<k<<","<<OFF<<","<<seconds;
      for (SmaInt i=0; i<NUM_COUNTS; ++i) {
         cout << ",";
         if (TOTAL_COUNTS[i])
            cout << TOTAL_COUNTS[i];
      }
      cout << endl;

      // BigInt TOTAL=0; // should add up to NUM_DEALS when we're done
      // for (SmaInt i=0; i<NUM_COUNTS; ++i) {
      //    double frac = (TOTAL_COUNTS[i]*1.0)/OFF;
      //    cout << i << " sets: " << counts_dev[i] << " " << frac << endl;
      //    TOTAL += counts_dev[i];
      // }
      
      //cout << "Total: " << TOTAL << " " << (TOTAL*1.0)/NUM_DEALS << endl << endl;
      
   } // loop through all batches

   delete[] DEALI_VEC;
   delete[] PARALLEL_COUNTS;
}


int main(int argc, char**argv) {
   // precompute table of combinations up to C(81,12)
   for   (SmaInt n=0; n<=NUM_CARDS; ++n)
      for (SmaInt k=0; k<=MAX_DEAL;  ++k)
         CHOOSE[n][k] = combinations(n,k);

   // precompute all the third cards for every pair that make SETs
   for (SmaInt a=0; a<NUM_CARDS; ++a)
   for (SmaInt b=0; b<NUM_CARDS; ++b)
      THIRD[a][b] = compute_third(a,b);

   self_test(); // run these every time. exit(1) on any test failing

  std::vector<int> args;
  for (int i=1; i<argc; ++i)
     args.push_back(atoi(argv[i]));
  if (args.empty())
     return 0; // just the tests
  
  SmaInt k = args.front();
  BigInt BATCHSIZE=1000, PARALLELS=0;
  if (argc > 2) BATCHSIZE = args.back();
  if (argc > 3) {
     PARALLELS = args[1];
     enumerate(k, PARALLELS, BATCHSIZE);
  } else {
     enumerate_serial(k, BATCHSIZE);
  }
}
