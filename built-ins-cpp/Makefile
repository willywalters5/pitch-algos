EXENAME = built-ins

CXX = clang++
CXXFLAGS = -std=c++0x -g -O0 -Wall -Wextra -I/usr/include/python2.7 -lpython2.7

all : $(EXENAME)

$(EXENAME): numpy_scipy_funcs.cpp tests.cpp main.cpp
	$(CXX) $(CXXFLAGS) numpy_scipy_funcs.cpp tests.cpp main.cpp -o $(EXENAME)

.PHONY: clean
clean:
	rm -f $(EXENAME)
