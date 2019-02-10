#CLINC = /opt/AMDAPPSDK-3.0/include
#CLLIB = /opt/AMDAPPSDK-3.0/lib/x86_64
#CLINC = /opt/OpenCL-Headers
#CLLIB = /usr/lib/x86_64-linux-gnu
CXXFLAGS =             #-I$(CLINC) -L$(CLLIB)

all: enumerate_deals


enumerate_deals: enumerate_deals.cc
	$(CXX) $(CXXFLAGS) -o enumerate_deals $^ -lOpenCL

cltest: cltest.cc
	$(CXX) $(CXXFLAGS) -o cltest $^ -lOpenCL

clean:
	rm -rf enumerate_deals cltest
