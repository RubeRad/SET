#CLINC = /opt/AMDAPPSDK-3.0/include
#CLLIB = /opt/AMDAPPSDK-3.0/lib/x86_64
#CLINC = /opt/OpenCL-Headers
#CLLIB = /usr/lib/x86_64-linux-gnu
CXXFLAGS = -std=c++17            #-I$(CLINC) -L$(CLLIB)

all: enumerate_deals


enumerate_deals: enumerate_deals.cc enumerate_deals.cl
	$(CXX) $(CXXFLAGS) -o enumerate_deals enumerate_deals.cc -lOpenCL -lpthread

enumerate_deals.pdf: enumerate_deals.tex
	pdflatex enumerate_deals.tex

cltest: cltest.cc
	$(CXX) $(CXXFLAGS) -o cltest $^ -lOpenCL

clean:
	rm -rf enumerate_deals cltest
