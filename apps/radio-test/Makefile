# Copyright (c) Christoph M. Wintersteiger
# Licensed under the MIT License.

WLMCD_ROOT=../..

# CXX=clang++-8
# CXXFLAGS+=-O1 -g -fsanitize=address -fno-omit-frame-pointer -D_GLIBCXX_DEBUG
# LDFLAGS+=-g -fsanitize=address

CXXFLAGS+=-g -MMD -MP -I ${WLMCD_ROOT} -Wno-psabi

%.o: %.cpp
	${CXX} ${CXXFLAGS} $< -c -o $@

all: radio-test

SRC = radio_test.cpp radio_test_tracker.cpp radio_test_ui.cpp
OBJ = $(subst .cpp,.o,$(SRC))

LDFLAGS_STATIC=${LDFLAGS} ${WLMCD_ROOT}/libwlmcd-dev.a ${WLMCD_ROOT}/libwlmcd-ui.a -lpthread -lrt -lncurses -lcrypto -lgpiod
radio-test: ${OBJ} ${WLMCD_ROOT}/libwlmcd-dev.a ${WLMCD_ROOT}/libwlmcd-ui.a
	${CXX} -o $@ ${OBJ} ${LDFLAGS_STATIC}

clean:
	rm -rf *.o cc1101-enocean-monitor

-include *.d
