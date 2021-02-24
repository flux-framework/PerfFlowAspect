CXX := g++
CXXFLAGS := -I../../common/libtap -I../
LDFLAGS := -L../../common/libtap -L../

all: anno_validator_test1

anno_validator_test1: anno_validator_test1.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lperfflow_parser -ltap

anno_validator_test1.o: anno_validator_test1.cpp
	$(CXX) $(CXXFLAGS) $^ -c -o $@

.PHONY: clean

clean:
	rm anno_validator_test1.o anno_validator_test1
