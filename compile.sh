#!/bin/sh
g++ \
main.cpp \
msl/file_util.cpp \
msl/json.cpp \
msl/serial.cpp \
msl/serial_sync.cpp \
msl/serial_util.cpp \
msl/socket.cpp \
msl/socket_util.cpp \
msl/string_util.cpp \
msl/time_util.cpp \
msl/webserver.cpp \
-O -Wall \
-o smarthouse