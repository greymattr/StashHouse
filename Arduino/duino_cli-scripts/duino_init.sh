#!/bin/bash
#
#
# The MIT License (MIT)
# 
# Copyright (c) 2014 Matthew Fatheree 
# greymattr [@t] gmail (dot) com.
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
MY_EDITOR=kate


print_use() {
	echo "Use:"
	echo "$0 <project name>"
	exit
}

if [ ! "$1" ] ; then
	print_use
fi

blank_template()
{
(
cat <<'End-of-Text'
// Blink

void setup(void) {
  pinMode(13, OUTPUT);
}

void loop() {
  digitalWrite(13, LOW);
  delay(1000);
  digitalWrite(13, HIGH);
  delay(1000);
}
End-of-Text
) > $1
   return 0
}


makefile_template()
{
(
cat <<'End-of-Text'

include .build.config

#  define the board below:
# BOARD_TAG = uno						// Arduino Uno
# BOARD_TAG = micro				// Arduino micro
# BOARD_TAG = LilyPadUSB  // LilyPad Arduino USB
# BOARD_TAG = atmega168   // Arduino NG or older w/ ATmega168
# BOARD_TAG = atmega328   // Arduino Duemilanove w/ ATmega328
# BOARD_TAG = atmega8     // Arduino NG or older w/ ATmega8
# BOARD_TAG = bt          // Arduino BT w/ ATmega168
# BOARD_TAG = bt328       // Arduino BT w/ ATmega328
# BOARD_TAG = diecimila   // Arduino Diecimila or Duemilanove w/ ATmega168
# BOARD_TAG = esplora     // Arduino Esplora
# BOARD_TAG = ethernet    // Arduino Ethernet
# BOARD_TAG = fio         // Arduino Fio
# BOARD_TAG = leonardo    // Arduino Leonardo
# BOARD_TAG = lilypad     // LilyPad Arduino w/ ATmega168
# BOARD_TAG = lilypad328  // LilyPad Arduino w/ ATmega328
# BOARD_TAG = mega        // Arduino Mega (ATmega1280)
# BOARD_TAG = mega2560    // Arduino Mega 2560 or Mega ADK
# BOARD_TAG = micro       // Arduino Micro
# BOARD_TAG = mini        // Arduino Mini w/ ATmega168
# BOARD_TAG = mini328     // Arduino Mini w/ ATmega328
# BOARD_TAG = nano        // Arduino Nano w/ ATmega168
# BOARD_TAG = nano328     // Arduino Nano w/ ATmega328
# BOARD_TAG = pro         // Arduino Pro or Pro Mini (3.3V, 8 MHz) w/ ATmega168
# BOARD_TAG = pro328      // Arduino Pro or Pro Mini (3.3V, 8 MHz) w/ ATmega328
# BOARD_TAG = pro5v       // Arduino Pro or Pro Mini (5V, 16 MHz) w/ ATmega168
# BOARD_TAG = pro5v328    // Arduino Pro or Pro Mini (5V, 16 MHz) w/ ATmega328

#  define the port the device is connected to
ARDUINO_PORT = /dev/ttyACM0
# ARDUINO_PORT = /dev/ttyUSB0


ARDUINO_LIBS =
ARDUINO_DIR = /usr/share/arduino
# ARDUINO_DIR = /usr/share/arduino
ARDMK_DIR = /usr
AVR_TOOLS_DIR = /usr

#  Make commands available
# make – no upload
# make upload – compile and upload
# make clean – remove all our dependencies
# make depends – update dependencies
# make reset – reset the Arduino by tickling DTR on the serial port
# make raw_upload – upload without first resetting
# make show_boards – list all the boards defined in boards.txt

include ../Arduino.mk

build-config:
	sh /home/matthewf/bin/scripts/duino_config.sh

End-of-Text
) > Makefile
   return 0
}

make_help()
{
	echo ""
	echo "Make commands available"
	echo "make         – no upload"
	echo "make upload  – compile and upload"
	echo "make clean   – remove all our dependencies"
	echo "make depends – update dependencies"
	echo "make reset   – reset the Arduino by tickling DTR on the serial port"
	echo "make raw_upload – upload without first resetting"
	echo "make show_boards – list all the boards defined in boards.txt"
	echo ""
}

mkdir -p ${1}
cd ${1}
ln -s /usr/share/arduino/Arduino.mk
mkdir ${1}
cd ${1}
echo "BOARD_TAG = uno" > .build.config
makefile_template
blank_template ${1}.ino
# $MY_EDITOR ${1}.ino
# make_help
