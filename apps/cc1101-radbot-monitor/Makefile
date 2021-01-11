# Copyright (c) Christoph M. Wintersteiger
# Licensed under the MIT License.

WLMCD_ROOT=../..

CXXFLAGS+=-I ${WLMCD_ROOT} -Wno-psabi

%.o: %.cpp
	${CXX} ${CXXFLAGS} $< -c -o $@

all: cc1101-radbot-monitor cc1101-radbot-monitor-shared

# Statically linked libwlmcd
LDFLAGS_STATIC=${LDFLAGS} ${WLMCD_ROOT}/libwlmcd-dev.a ${WLMCD_ROOT}/libwlmcd-ui.a -lwiringPi -lpthread -lrt -lncurses -lcrypto
cc1101-radbot-monitor: cc1101-radbot-monitor.o
	${CXX} -o $@ $^ ${LDFLAGS_STATIC}

# Dynamically linked libwlmcd
LDFLAGS_SHARED=-L ${WLMCD_ROOT} -lwlmcd-dev -lwlmcd-ui -lwiringPi
cc1101-radbot-monitor-shared: cc1101-radbot-monitor.o
	${CXX} -o $@ $^ ${LDFLAGS_SHARED}

clean:
	rm -rf *.o cc1101-radbot-monitor cc1101-radbot-monitor-shared