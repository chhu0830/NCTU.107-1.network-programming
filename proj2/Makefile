CC          ?= gcc
CFLAGS      += -std=gnu99 -Wall -Wextra -MMD -MF $@.d
LDFLAGS		+= -lrt

STUDENT_ID  := 0756020
EXECUTABLE  := np_simple np_single_proc np_multi_proc

OUT         ?= .build
SRCS        := $(wildcard *.c */*.c)
OBJS        := $(addprefix $(OUT)/, $(SRCS:.c=_simple.o)) \
			   $(addprefix $(OUT)/, $(SRCS:.c=_single_proc.o)) \
			   $(addprefix $(OUT)/, $(SRCS:.c=_multi_proc.o))
DEPS        := $(OBJS:.o=.o.d)

all: $(EXECUTABLE)

np_%: $(OBJS)
	$(CC) $(filter %$(subst np_,,$@).o,$(OBJS)) -o $@ $(LDFLAGS)

$(OUT)/%_simple.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -DSIMPLE $< -o $@

$(OUT)/%_single_proc.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -DSINGLE $< -o $@

$(OUT)/%_multi_proc.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -DMULTI $< -o $@

run-%: deploy
	cd ./test/test_env && ../../np_$(subst run-,,$@)

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
