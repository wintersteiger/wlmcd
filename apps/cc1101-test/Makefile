# Copyright (c) Christoph M. Wintersteiger
# Licensed under the MIT License.

WLMCD_ROOT=../..

# CXX=clang++-8
# CXXFLAGS+=-O1 -g -fsanitize=address -fno-omit-frame-pointer -D_GLIBCXX_DEBUG
# LDFLAGS+=-g -fsanitize=address

CXXFLAGS+=-g -I ${WLMCD_ROOT} -Wno-psabi

%.o: %.cpp
	${CXX} ${CXXFLAGS} $< -c -o $@

all: cc1101-test

OBJ=cc1101-test.o

LDFLAGS_STATIC=${LDFLAGS} ${WLMCD_ROOT}/libwlmcd-dev.a ${WLMCD_ROOT}/libwlmcd-ui.a -lwiringPi -lpthread -lrt -lncurses -lcrypto -lgpiod
cc1101-test: ${OBJ} ${WLMCD_ROOT}/libwlmcd-dev.a ${WLMCD_ROOT}/libwlmcd-ui.a
	${CXX} -o $@ ${OBJ} ${LDFLAGS_STATIC}

clean:
	rm -rf *.o cc1101-test