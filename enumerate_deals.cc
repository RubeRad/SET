#include <stdlib.h>
#include <iostream>
#include <sstream>

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

#define DIE(msg) { cerr << "FATAL ERROR (" << __LINE__ << "): " << msg << endl; exit(1); }

// 64 bits is long enough for C(81,12)=70724320184700
typedef cl_ulong BigInt; // cl_ulong is unsigned __int64 in cl_platform.h
// 8 bits is long enough for everything else
 // but short prints as an int, so that's easier
typedef cl_ushort SmaInt;

const SmaInt NUM_CARDS=81; // 3^4 for 4 attributes of 3 values
const SmaInt MAX_DEAL=12;  // if technology permits this might be increased
const SmaInt DEAL_SIZE=5;  // used in num_sets_kernel
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

inline bool has_a_set(const deal_type& deal, SmaInt k) 
  { return has_a_set(&(deal.card[0]), k); }

// Unfortunately this has to iterate through all triples every time
SmaInt num_sets(const card_type* cards, SmaInt kay) {
  SmaInt count=0;
  for (SmaInt i=0;   i<kay; ++i)
  for (SmaInt j=i+1; j<kay; ++j)
  for (SmaInt k=j+1; k<kay; ++k)
    if (is_a_set(cards[i],
		 cards[j],
		 cards[k])) 
      ++count;

  return count;
}

inline SmaInt num_sets(const deal_type& deal, SmaInt k) 
  { return num_sets(&(deal.card[0]), k); }


int get_global_id(int dum) { return dum; }

// assuming DEAL_SIZE is constant/defined
// assuming CHOOSE[][] is populated
void
num_sets_kernel(const BigInt* DEALI, // which deal, in combinatorial order
                SmaInt* num_sets_deal) // output
{
   int i=get_global_id(0);

   deal_type d;
   unrank_deal(DEALI[i], DEAL_SIZE, &(d.card[0]));
   num_sets_deal[i] = num_sets(d, DEAL_SIZE);
}


void enumerate(SmaInt k,
               BigInt BATCHSIZE) {
   BigInt NUM_DEALS = CHOOSE[NUM_CARDS][k];
   cout << "Num deals for " << k << " cards is " << NUM_DEALS << endl;

   BigInt counts1[22]; // internet sez max sets for 12 cards is 22
   BigInt counts2[22];
   for (int i=0; i<22; ++i)
      counts1[i]=0;

   BigInt* DEALI_VEC = new BigInt[BATCHSIZE];
   SmaInt* ANSA1_VEC = new SmaInt[BATCHSIZE]; // serial verification
   SmaInt* ANSA2_VEC = new SmaInt[BATCHSIZE]; // kernel results

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
   cl_command_queue command_queue = clCreateCommandQueueWithProperties(context, device_id, NULL, &ret);
   cout << "clCreateCommandQueueWithProperties returns " << ret << endl;

   // Create memory buffers on the device for each vector 
   cl_mem inn_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, 
                                   BATCHSIZE * sizeof(BigInt), NULL, &ret);
   cout << "clClCreateBuffer(inn) returns " << ret << endl;
   
   cl_mem out_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 
                                   BATCHSIZE * sizeof(SmaInt), NULL, &ret);
   cout << "clClCreateBuffer(out) returns " << ret << endl;
   

   FILE* fp;
   fp = fopen("enumerate_deals.cl", "r");
   if (!fp) {
      fprintf(stderr, "Failed to load kernel.\n");
      exit(1);
   }
   const int MAX_SOURCE_SIZE = 1000000;
   char* source_str = new char[MAX_SOURCE_SIZE];
   size_t source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
   fclose(fp);
   cout << "Read CL source length " << source_size << endl;
   
   // Create and build the program
   cl_program program = clCreateProgramWithSource(context, 1, 
       (const char **)&source_str, (const size_t *)&source_size, &ret);
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

   // Create the OpenCL kernel
   cl_kernel kernel = clCreateKernel(program, "num_sets", &ret);
   cout << "clCreateKernel returns " << ret << endl;
   
   // Set the arguments of the kernel
   ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&inn_obj);
   cout << "clSetKernelArg(inn) returns " << ret << endl;
   ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&out_obj);
   cout << "clSetKernelArg(out) returns " << ret << endl;
   
   BigInt OFF=0;
   while (OFF < NUM_DEALS) {
      // Most batches are full, last one may be less
      BigInt NUM = BATCHSIZE;
      if (OFF+NUM > NUM_DEALS)
         NUM = NUM_DEALS-OFF;
      
      // Now OFF is number processed so far,
      // NUM is how many to process this batch
      
      // Populate vector of deal numbers to process
      for (BigInt I=0; I<NUM; ++I)
         DEALI_VEC[I] = OFF+I;

      // copy DEALI_VEC into device buffer
      ret = clEnqueueWriteBuffer(command_queue, inn_obj, CL_TRUE, 0,
            NUM * sizeof(BigInt), DEALI_VEC, 0, NULL, NULL);
      cout << "clClEnqueueWriteBuffer returns " << ret << endl;

      // Serially process
      for (BigInt I=0; I<NUM; ++I)
         num_sets_kernel(DEALI_VEC+I,
                         ANSA1_VEC+I);

      // Execute kernels
      size_t global_item_size=NUM, local_item_size = 1;
      ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, 
            &global_item_size, &local_item_size, 0, NULL, NULL);
      cout << "clEnqueueNDRangeKernel returns " << ret << endl;

      // copy ANSA2_VEC from device buffer
      ret = clEnqueueReadBuffer(command_queue, out_obj, CL_TRUE, 0, 
            NUM * sizeof(SmaInt), ANSA2_VEC, 0, NULL, NULL);
      cout << "clClEnqueueReadBuffer returns " << ret << endl;

      // tabulate this batch of ansas
      for (int ii=0; ii<22; ++ii) counts2[ii] = 0;
      for (BigInt I=0; I<NUM; ++I) {
         SmaInt ansa1 = ANSA1_VEC[I];
         counts1[ansa1]++;
         SmaInt ansa2 = ANSA2_VEC[I];
         counts2[ansa2]++;
         if (ansa2 != ansa1)
            cout << "Wrong answer for " << OFF+I
                 << ": correct = " << ansa1
                 << ": wrong = "   << ansa2 << endl;
      }
      for (int ii=0; ii<22; ++ii)
         cout << "Counts2 " << counts2[ii] << endl;

      // Now that we are done with this batch, roll NUM into OFF
      OFF += NUM;
      // Now again OFF is the number of deals processed so far
      // if OFF==NUM_DEALS this is the last time through the while loop
      
      // intermittent reporting
      BigInt TOTAL=0; // should add up to NUM_DEALS when we're done
      for (SmaInt i=0; i<22; ++i) {
         if (counts1[i]==0)
            continue;
         double frac = (counts1[i]*1.0)/OFF;
         cout << i << " sets: " << counts1[i] << " " << frac << endl;
         TOTAL += counts1[i];
      }
      cout << "Total: " << TOTAL << endl << endl;
      
   } // loop through all batches

   delete[] DEALI_VEC;
   delete[] ANSA1_VEC;
   delete[] ANSA2_VEC;
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

int main(int argc, char**argv) {

  cl_uint n_plats;
  clGetPlatformIDs(0, 0, &n_plats);
  cout << "platforms: " << n_plats << endl;
  

  for   (SmaInt n=0; n<=NUM_CARDS; ++n)
    for (SmaInt k=0; k<=MAX_DEAL;  ++k)
      CHOOSE[n][k] = combinations(n,k);

  if (1) 
    self_test();

  
  enumerate(DEAL_SIZE, 1024);
}
