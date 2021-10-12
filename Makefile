# Copyright (c) Christoph M. Wintersteiger
# Licensed under the MIT License.

all: libwlmcd-dev.a libwlmcd-ui.a libwlmcd-dev.so libwlmcd-ui.so tests

CXXFLAGS+=-g -MMD -MP -Wall -Wno-unused-variable -Wno-unused-function
CXXFLAGS+=-I .
CXXFLAGS+=-Wno-psabi
CXXFLAGS+=$(shell pkg-config dbus-1 --cflags)

LDFLAGS=-lrt -lcrypto -lwiringPi -lgpiod -lpthread -lgpiod

# CXX=clang++-8
# CXXFLAGS+=-g -fsanitize=address -fno-omit-frame-pointer -D_GLIBCXX_DEBUG
# CXXFLAGS+=-O1
# LDFLAGS+=-g -fsanitize=address

%.o: %.cpp
	${CXX} ${CXXFLAGS} $< -c -o $@

SRC = errors.cpp \
	decoder.cpp basic.cpp logfile.cpp \
	spidev.cpp i2c_device.cpp\
	evohome.cpp radbot.cpp enocean.cpp \
	cc1101.cpp \
	sx1278.cpp \
	gpio_device.cpp \
	gpio_button.cpp \
	si4463.cpp si4463_rt.cpp \
	rfm69.cpp \
	ble100.cpp \
	relay_device.cpp \
	ina219.cpp \
	ds18b20.cpp \
	rfm69.cpp \
	dht22.cpp \
	mcp9808.cpp \
	es9018k2m.cpp es9018k2m_rt.cpp \
	es9028pro.cpp es9028pro_rt.cpp \
	bme280.cpp bme680.cpp \
	mcp3423.cpp \
	ccs811.cpp

UI_SRC = field.cpp ui.cpp basic_ui.cpp raw_ui.cpp controller.cpp shell.cpp \
	cc1101_ui.cpp cc1101_ui_raw.cpp \
	radbot_ui.cpp evohome_ui.cpp \
	sx1278_ui_raw.cpp sx1278_ui.cpp \
	gpio_ui.cpp \
	gpio_button_field.cpp \
	si4463_ui_raw.cpp \
	rfm69_ui_raw.cpp \
	relay_ui.cpp \
	ina219_ui.cpp \
	rfm69_ui.cpp \
	dht22_ui.cpp \
	mcp9808_ui.cpp \
	es9018k2m_ui.cpp \
	es9028pro_ui.cpp \
	bme280_ui.cpp bme680_ui.cpp \
	mcp3423_ui.cpp \
	ccs811_ui.cpp

OBJ = $(subst .cpp,.o,$(SRC))
UI_OBJ = $(subst .cpp,.o,$(UI_SRC))

libwlmcd-dev.a: $(OBJ)
	${AR} rcs $@ $^

libwlmcd-ui.a: $(UI_OBJ)
	${AR} rcs $@ $^

libwlmcd-dev.so: $(OBJ)
	${CXX} -shared -o $@ $^ ${LDFLAGS}

libwlmcd-ui.so: $(UI_OBJ) libwlmcd-dev.so
	${CXX} -shared -o $@ $^ ${LDFLAGS} -lncurses -L . -lwlmcd-dev

tests: tests.o evohome_tests.o radbot_tests.o enocean_tests.o $(OBJ)
	${CXX} ${CXXFLAGS} -o $@ $^ ${LDFLAGS}

clean:
	rm -rf *.d *.o libwlmcd-dev.a libwlmcd-ui.a libwlmcd-dev.so libwlmcd-ui.so tests

-include *.d
