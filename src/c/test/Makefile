CXX := clang++
CXXFLAGS := -std=c++11 -Xclang -load -Xclang ../weaver/build/weave/libWeavePass.so
LDFLAGS := -L../runtime -lperfflow_runtime -lcrypto

all: smoketest smoketest2

smoketest: smoketest.o
	$(CXX) $< -o $@ $(LDFLAGS)

smoketest2: smoketest2.o
	$(CXX) $< -o $@ $(LDFLAGS)

%.o: %.c
	$(CXX) $(CXXFLAGS) $< -c -fPIC -o $@

.PHONY: clean

clean:
	rm -f *.o smoketest smoketest2

