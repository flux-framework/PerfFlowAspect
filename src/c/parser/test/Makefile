CXX := g++
CXXFLAGS := -I../../../common/libtap -I../
LDFLAGS := -L../../../common/libtap -L../

all: anno_validate_test anno_parse_test

anno_validate_test: anno_validate_test.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lperfflow_parser -ltap
anno_parse_test: anno_parse_test.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lperfflow_parser -ltap

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $^ -c -o $@

.PHONY: clean

clean:
	rm -f *.o anno_validate_test anno_parse_test
