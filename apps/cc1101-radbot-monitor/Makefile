# Copyright (c) Christoph M. Wintersteiger
# Licensed under the MIT License.

WLMCD_ROOT=../..

# CXX=clang++-8
# CXXFLAGS+=-O1 -g -fsanitize=address -fno-omit-frame-pointer -D_GLIBCXX_DEBUG
# LDFLAGS+=-g -fsanitize=address

CXXFLAGS+=-g -I ${WLMCD_ROOT} -Wno-psabi

%.o: %.cpp
	${CXX} ${CXXFLAGS} $< -c -o $@

all: cc1101-radbot-monitor cc1101-radbot-monitor-shared

# Static libwlmcd
LDFLAGS_STATIC=${LDFLAGS} ${WLMCD_ROOT}/libwlmcd-dev.a ${WLMCD_ROOT}/libwlmcd-ui.a -lpthread -lrt -lncurses -lcrypto -lgpiod
cc1101-radbot-monitor: cc1101-radbot-monitor.o ${WLMCD_ROOT}/libwlmcd-dev.a ${WLMCD_ROOT}/libwlmcd-ui.a
	${CXX} -o $@ $< ${LDFLAGS_STATIC}

# Dynamic libwlmcd
LDFLAGS_SHARED=${LDFLAGS} -L ${WLMCD_ROOT} -lwlmcd-dev -lwlmcd-ui -lpthread -lrt -lncurses -lcrypto -lgpiod
cc1101-radbot-monitor-shared: cc1101-radbot-monitor.o ${WLMCD_ROOT}/libwlmcd-dev.so ${WLMCD_ROOT}/libwlmcd-ui.so
	${CXX} -o $@ $< ${LDFLAGS_SHARED}

clean:
	rm -rf *.o cc1101-radbot-monitor cc1101-radbot-monitor-shared