CC          ?= gcc
CFLAGS      += -std=gnu99 -Wall -Wextra -DSINGLE -MMD -MF $@.d
LDFLAGS		+= -lrt

STUDENT_ID  := 0756020
EXECUTABLE  := npshell

OUT         ?= .build
SRCS        := $(wildcard *.c */*.c)
OBJS        := $(addprefix $(OUT)/, $(SRCS:.c=.o))
DEPS        := $(OBJS:.o=.o.d)

$(EXECUTABLE): $(OBJS) 
	$(CC) $(OBJS) -o $(EXECUTABLE) $(LDFLAGS)

$(OUT)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

run: deploy
	cd ./test/test_env && ../../$(EXECUTABLE)

deploy: $(EXECUTABLE)
	@rm -rf ./test/test_*
	@cp -r ./test/env ./test/test_env

test: $(EXECUTABLE)
	@cd ./test/np_project1_demo_sample/ && ./demo.sh ../../npshell

zip:
	@rm -rf $(STUDENT_ID) $(STUDENT_ID).zip ; mkdir $(STUDENT_ID)
	@git ls-files | xargs -i cp {} $(STUDENT_ID)
	@zip -r $(STUDENT_ID).zip $(STUDENT_ID) ; rm -rf $(STUDENT_ID)

clean:
	rm -rf ./test/test_* $(OUT) $(STUDENT_ID) $(STUDENT_ID).zip $(EXECUTABLE)

-include $(DEPS) 
