# Copyright (c) Christoph M. Wintersteiger
# Licensed under the MIT License.

WLMCD_ROOT=../..

# CXX=clang++-8
# CXXFLAGS+=-O1 -g -fsanitize=address -fno-omit-frame-pointer -D_GLIBCXX_DEBUG
# LDFLAGS+=-g -fsanitize=address

CXXFLAGS+=-g -MMD -MP -I ${WLMCD_ROOT} -Wno-psabi

%.o: %.cpp
	${CXX} ${CXXFLAGS} $< -c -o $@

all: enocean-monitor enocean-test

SRC = enocean_frame.cpp enocean_codec.cpp enocean_telegrams.cpp enocean_ui.cpp enocean_gateway.cpp enocean-monitor.cpp
OBJ = $(subst .cpp,.o,$(SRC))

LDFLAGS_STATIC=${LDFLAGS} ${WLMCD_ROOT}/libwlmcd-ui.a ${WLMCD_ROOT}/libwlmcd-dev.a  -lpthread -lrt -lncurses -lcrypto -lgpiod
enocean-monitor: ${OBJ}  ${WLMCD_ROOT}/libwlmcd-ui.a ${WLMCD_ROOT}/libwlmcd-dev.a
	${CXX} -o $@ ${OBJ} ${LDFLAGS_STATIC}

usb300: usb300.o
	${CXX} -o $@ $<

enocean-test: enocean_tests.o enocean_frame.o enocean_codec.o
	${CXX} -o $@ $^ ${LDFLAGS_STATIC}

clean:
	rm -rf *.o enocean-monitor

-include *.d
