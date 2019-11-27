#CFLAGS_STD=-std=gnu11 -flto -fno-fat-lto-objects -O3
#CXXFLAGS_STD=-std=gnu++11 -fno-threadsafe-statics -flto -O3

USER_LIB_PATH=../avrz
ARDUINO_LIBS=fastadc
BOARD_TAG=nano
BOARD_SUB=atmega328
include /usr/share/arduino/Arduino.mk
