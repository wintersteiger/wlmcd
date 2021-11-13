# Copyright (c) Christoph M. Wintersteiger
# Licensed under the MIT License.

WLMCD_ROOT=../..

# CXX=clang++-8
# CXXFLAGS+=-O1 -g -fsanitize=address -fno-omit-frame-pointer -D_GLIBCXX_DEBUG
# LDFLAGS+=-g -fsanitize=address

CXXFLAGS+=-g -I ${WLMCD_ROOT} -Wno-psabi

%.o: %.cpp
	${CXX} ${CXXFLAGS} $< -c -o $@

all: rfm69-evohome-monitor rfm69-evohome-monitor-shared

# Static libwlmcd
LDFLAGS_STATIC=${LDFLAGS} ${WLMCD_ROOT}/libwlmcd-dev.a ${WLMCD_ROOT}/libwlmcd-ui.a -lpthread -lrt -lncurses -lcrypto -lgpiod
rfm69-evohome-monitor: rfm69-evohome-monitor.o ${WLMCD_ROOT}/libwlmcd-dev.a ${WLMCD_ROOT}/libwlmcd-ui.a
	${CXX} -o $@ $< ${LDFLAGS_STATIC}

# Dynamic libwlmcd
LDFLAGS_SHARED=${LDFLAGS} -L ${WLMCD_ROOT} -lwlmcd-dev -lwlmcd-ui -lpthread -lrt -lncurses -lcrypto -lgpiod
rfm69-evohome-monitor-shared: rfm69-evohome-monitor.o
	${CXX} -o $@ $^ ${LDFLAGS_SHARED}

clean:
	rm -rf *.o rfm69-evohome-monitor rfm69-evohome-monitor-shared
