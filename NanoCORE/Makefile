include Makefile.arch

SOURCES=$(wildcard *.cc) $(wildcard Tools/*.cc) $(wildcard Tools/btagsf/*.cc)
ifdef CMSSW_BASE
SOURCES+=$(wildcard Tools/jetcorr/PhysicsTools/NanoAODTools/src/*.cc)
endif
OBJECTS=$(SOURCES:.cc=.o)
LIB=NANO_CORE.so

$(LIB): $(OBJECTS) 
	$(LD) $(LDFLAGS) $(SOFLAGS) $(OBJECTS) $(ROOTLIBS) -lTMVA -lEG -lGenVector -lXMLIO -lMLP -lTreePlayer -o $@
	ln -sf $(LIB) lib$(LIB)

%.o:	%.cc
	$(CXX) $(CXXFLAGS) -I${CMSSW_BASE}/../../../external/boost/1.67.0/include -I${CMSSW_BASE}/src -c $< -o $@ -fno-var-tracking

test: all
	python Tools/unit_tests/tests.py

testpy3: all
	python3 Tools/unit_tests/tests.py

all: $(LIB) 
clean:
	rm -f *.o \
	rm -f *.d \
	rm -f *.so \
	rm -f Tools/*.o \
	rm -f Tools/*.d \
	rm -f Tools/*.so
