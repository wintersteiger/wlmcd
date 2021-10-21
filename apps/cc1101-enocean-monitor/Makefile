# Copyright (c) Christoph M. Wintersteiger
# Licensed under the MIT License.

WLMCD_ROOT=../..

# CXX=clang++-8
# CXXFLAGS+=-O1 -g -fsanitize=address -fno-omit-frame-pointer -D_GLIBCXX_DEBUG
# LDFLAGS+=-g -fsanitize=address

CXXFLAGS+=-g -MMD -MP -I ${WLMCD_ROOT} -Wno-psabi

%.o: %.cpp
	${CXX} ${CXXFLAGS} $< -c -o $@

all: cc1101-enocean-monitor cc1101-enocean-monitor-shared

SRC = enocean.cpp enocean_telegrams.cpp enocean_ui.cpp enocean_gateway.cpp cc1101-enocean-monitor.cpp
OBJ = $(subst .cpp,.o,$(SRC))

# Static libwlmcd
LDFLAGS_STATIC=${LDFLAGS} ${WLMCD_ROOT}/libwlmcd-dev.a ${WLMCD_ROOT}/libwlmcd-ui.a -lwiringPi -lpthread -lrt -lncurses -lcrypto -lgpiod
cc1101-enocean-monitor: ${OBJ} ${WLMCD_ROOT}/libwlmcd-dev.a ${WLMCD_ROOT}/libwlmcd-ui.a
	${CXX} -o $@ ${OBJ} ${LDFLAGS_STATIC}

# Dynamic libwlmcd
LDFLAGS_SHARED=${LDFLAGS} -L ${WLMCD_ROOT} -lwlmcd-dev -lwlmcd-ui -lwiringPi -lpthread -lrt -lncurses -lcrypto -lgpiod
cc1101-enocean-monitor-shared: ${OBJ} ${WLMCD_ROOT}/libwlmcd-dev.so ${WLMCD_ROOT}/libwlmcd-ui.so
	${CXX} -o $@ ${OBJ} ${LDFLAGS_SHARED}

clean:
	rm -rf *.o cc1101-enocean-monitor cc1101-enocean-monitor-shared

-include *.d