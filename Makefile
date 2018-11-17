CC          ?= gcc
CFLAGS      += -std=gnu99 -Wall -Wextra -MMD -MF -g $@.d
LDFLAGS		+= -lrt

STUDENT_ID  := 0756020
EXECUTABLE  := server-simple server-single server-multi

OUT         ?= .build
SRCS        := $(wildcard *.c */*.c)
OBJS        := $(addprefix $(OUT)/, $(SRCS:.c=-simple.o)) \
			   $(addprefix $(OUT)/, $(SRCS:.c=-single.o)) \
			   $(addprefix $(OUT)/, $(SRCS:.c=-multi.o))
DEPS        := $(OBJS:.o=.o.d)

all: $(EXECUTABLE) deploy

server-%: $(OBJS)
	$(CC) $(filter %$(subst server-,,$@).o,$(OBJS)) -o $@ $(LDFLAGS)

$(OUT)/%-simple.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -DSIMPLE $< -o $@

$(OUT)/%-single.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -DSINGLE $< -o $@

$(OUT)/%-multi.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -DMULTI $< -o $@

run-%: deploy
	cd ./test/test_env && ../../server-$(subst run-,,$@)

deploy: $(EXECUTABLE)
	@rm -rf ./test/test_*
	@cp -r ./test/env ./test/test_env

zip:
	@rm -rf $(STUDENT_ID) $(STUDENT_ID).zip ; mkdir $(STUDENT_ID)
	@git ls-files | xargs -i cp {} $(STUDENT_ID)
	@zip -r $(STUDENT_ID).zip $(STUDENT_ID) ; rm -rf $(STUDENT_ID)

clean:
	rm -rf ./test/test_* $(OUT) $(STUDENT_ID) $(STUDENT_ID).zip $(EXECUTABLE)

-include $(DEPS) 
