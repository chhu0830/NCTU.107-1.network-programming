CC          ?= gcc
CFLAGS      += -std=gnu99 -Wall -Wextra -MMD -MF $@.d

OUT         := .build
EXECUTABLE  := npshell

SRCS        := $(wildcard *.c)
OBJS        := $(addprefix $(OUT)/, $(SRCS:.c=.o))
DEPS        := $(OBJS:.o=.o.d)

$(EXECUTABLE): $(OBJS) 
	$(CC) $(OBJS) -o $(EXECUTABLE)

$(OUT)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

run: $(EXECUTABLE)
	./$(EXECUTABLE)

clean:
	rm -rf $(OUT)
	rm -f $(EXECUTABLE)


-include $(DEPS) 
