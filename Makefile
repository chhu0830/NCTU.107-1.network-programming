CXX					?= g++
CXXFLAGS			+= -std=c++11 -Wall -Wextra -lboost_system -pthread -pedantic
CXX_INCLUDE_DIRS	:= /usr/local/include
CXX_LIB_DIRS		:= /usr/local/lib

STUDENT_ID			:= 0756020
EXECUTALBE			:= http_server

all: $(EXECUTALBE)
	cd test/env/ && ../../$(EXECUTALBE) 8000

%: %.cpp
	$(CXX) $< -o $@ -I $(CXX_INCLUDE_DIRS) -L $(CXX_LIB_DIRS) $(CXXFLAGS)

run: $(EXECUTALBE)
	./$(EXECUTALBE)

clean:
	rm -rf $(EXECUTALBE)
