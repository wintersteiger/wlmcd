# Copyright (c) Christoph M. Wintersteiger
# Licensed under the MIT License.

WLMCD_ROOT=../..

# CXX=clang++-8
# CXXFLAGS+=-O1 -g -fsanitize=address -fno-omit-frame-pointer
# LDFLAGS+=-g -fsanitize=address

CXXFLAGS+=-g -MMD -MP -I ${WLMCD_ROOT} -Wno-psabi

%.o: %.cpp
	${CXX} ${CXXFLAGS} $< -c -o $@

all: dht22-test # dht22-monitor

LDFLAGS_STATIC=${LDFLAGS} ${WLMCD_ROOT}/libwlmcd-dev.a ${WLMCD_ROOT}/libwlmcd-ui.a -lwiringPi -lpthread -lrt -lncurses -lcrypto -lgpiod
dht22-monitor: dht22-monitor.o ${WLMCD_ROOT}/libwlmcd-dev.a ${WLMCD_ROOT}/libwlmcd-ui.a
	${CXX} -o $@ $< ${LDFLAGS_STATIC}

dht22-test: dht22-test.o ${WLMCD_ROOT}/dht22.o
	${CXX} -o $@ $^ -lgpiod

clean:
	rm -rf *.o dht22-monitor dht22-test

-include *.d