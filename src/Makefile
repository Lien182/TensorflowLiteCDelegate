CC = gcc

CFLAGS += -g -Wno-unused-variable -Wall -I ~/git/tensorflow/

LDFLAGS+= lib/libtensorflowlite_c.so -lm

MKDIRP = mkdir -p

OBJS = delegated_fpga.o helper.o main.o 

BUILD_OBJS = $(OBJS:%=_build/%)

all: $(BUILD_OBJS)
	$(CC) $(BUILD_OBJS) $(LDFLAGS) -o tflite_c_example

install: all
	cp sortdemo $(PREFIX)

clean:
	rm -rf _build tflite_c_example

_build/%.o: %.c
	$(MKDIRP) $(@D)
	$(CC) $(CFLAGS) -x c -c $< -o $@

_build/%.o: %.cpp
	$(MKDIRP) $(@D)
	$(CC) $(CFLAGS) -x c -c $< -o $@
