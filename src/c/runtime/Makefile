CXX := clang++
CXXFLAGS := -O0 -g -std=c++11

OBJS := perfflow_runtime.o \
	advice_dispatcher.o \
	advice_chrome_tracing.o
      
all: libperfflow_runtime.so

libperfflow_runtime.so: $(OBJS)
	$(CXX) -shared $^ -o $@ -ljansson

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -c -fPIC -o $@


.PHONY: clean

clean:
	rm -f $(OBJS) $(OBJS) libperfflow_runtime.so

