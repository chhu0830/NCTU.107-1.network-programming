CXX					?= g++
CXXFLAGS			+= -I/usr/local/include -std=c++11 -Wall -Wextra -pedantic -MMD -MF $@.d
LDFLAGS				+= -L/usr/local/lib -lboost_system -lboost_filesystem -pthread

STUDENT_ID			:= 0756020
EXECUTABLE			:= http_server console.cgi

OUT					?= .build
SRCS				:= $(wildcard *.cpp */*.cpp)
OBJS				:= $(addprefix $(OUT)/, $(SRCS:.cpp=.o))
DEPS				:= $(OBJS:.o=.o.d)

containing			= $(foreach v,$(2),$(if $(findstring $(1),$(v)),$(v),))

all: $(EXECUTABLE)

http_server: $(call containing,http,$(OBJS))
	$(CXX) $^ -o $@ $(LDFLAGS)

console.cgi: $(call containing,console,$(OBJS))
	$(CXX) $^ -o $@ $(LDFLAGS)

$(OUT)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $< -o $@ -c $(CXXFLAGS)

run: $(EXECUTABLE)
	cd ./test/ && ./http_server 8000

zip:
	@rm -rf $(STUDENT_ID) $(STUDENT_ID).zip ; mkdir $(STUDENT_ID)
	@git ls-files | xargs -i cp {} $(STUDENT_ID)
	@zip -r $(STUDENT_ID).zip $(STUDENT_ID) ; rm -rf $(STUDENT_ID)

clean:
	rm -rf $(OUT) $(STUDENT_ID) $(STUDENT_ID).zip $(EXECUTABLE)

-include $(DEPS) 
