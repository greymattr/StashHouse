#!/bin/bash
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


cmd=(dialog --keep-tite --menu "Select options:" 22 76 16)
options=( 
          1 "Arduino Uno" 
          2 "Arduino Micro" 
					3 "LilyPad Arduino USB" 
					4 "Arduino NG or older w/ ATmega168"
					5 "Arduino Duemilanove w/ ATmega328" 
					6 "Arduino NG or older w/ ATmega8" 
					7 "Arduino BT w/ ATmega168" 
					8 "Arduino BT w/ ATmega328" 
					9 "Arduino Diecimila or Duemilanove w/ ATmega168" 
					10 "Arduino Esplora" 
					11 "Arduino Ethernet" 
					12 "Arduino Fio"
					13 "Arduino Leonardo"
					14 "LilyPad Arduino w/ ATmega168"
					15 "LilyPad Arduino w/ ATmega328"
					16 "Arduino Mega (ATmega1280)"
					17 "Arduino Mega 2560 or Mega ADK"
					18 "Arduino Mini w/ ATmega168"
					19 "Arduino Mini w/ ATmega328"
					20 "Arduino Nano w/ ATmega168"
					21 "Arduino Nano w/ ATmega328"
					22 "Arduino Pro or Pro Mini (3.3V, 8 MHz) w/ ATmega168"
					23 "Arduino Pro or Pro Mini (3.3V, 8 MHz) w/ ATmega328"
					24 "Arduino Pro or Pro Mini (5V, 16 MHz) w/ ATmega168"
					25 "Arduino Pro or Pro Mini (5V, 16 MHz) w/ ATmega328"
         )
choices=$("${cmd[@]}" "${options[@]}" 2>&1 >/dev/tty)
clear
for choice in $choices
do
    case $choice in
        1)
            echo "Arduino Uno"
            echo "BOARD_TAG = uno" > .build.config
            ;;
        2)
            echo "Arduino micro"
            echo "BOARD_TAG = micro" > .build.config
            ;;
        3)
            echo "LilyPad Arduino USB"
            echo "BOARD_TAG = LilyPadUSB" > .build.config
            ;;
        4)
            echo "Arduino NG or older w/ ATmega168"
            echo "BOARD_TAG = atmega168" > .build.config
            ;;
        5)
            echo "Arduino Duemilanove w/ ATmega328"
            echo "BOARD_TAG = atmega328" > .build.config
            ;;
        6)
            echo "Arduino NG or older w/ ATmega8"
            echo "BOARD_TAG = atmega8" > .build.config
            ;;
        7)
            echo "Arduino BT w/ ATmega168"
            echo "BOARD_TAG = bt" > .build.config
            ;;
        8)
            echo "Arduino BT w/ ATmega328"
            echo "BOARD_TAG = bt328" > .build.config
            ;;
        9)
            echo "Arduino Diecimila or Duemilanove w/ ATmega168"
            echo "BOARD_TAG = diecimila" > .build.config
            ;;
        10)
            echo "Arduino Esplora"
            echo "BOARD_TAG = esplora" > .build.config
            ;;
        11)
            echo "Arduino Ethernet"
            echo "BOARD_TAG = ethernet" > .build.config
            ;;
        12)
            echo "Arduino Fio"
            echo "BOARD_TAG = fio" > .build.config
            ;;
        13)
            echo "Arduino Leonardo"
            echo "BOARD_TAG = leonardo" > .build.config
            ;;
        14)
            echo "LilyPad Arduino w/ ATmega168"
            echo "BOARD_TAG = lilypad" > .build.config
            ;;
        15)
            echo "LilyPad Arduino w/ ATmega328"
            echo "BOARD_TAG = lilypad328" > .build.config
            ;;
        16)
            echo "Arduino Mega (ATmega1280)"
            echo "BOARD_TAG = mega" > .build.config
            ;;
        17)
            echo "Arduino Mega 2560 or Mega ADK"
            echo "BOARD_TAG = mega2560" > .build.config
            ;;
        18)
            echo "Arduino Mini w/ ATmega168"
            echo "BOARD_TAG = mini" > .build.config
            ;;
        19)
            echo "Arduino Mini w/ ATmega328"
            echo "BOARD_TAG = mini328" > .build.config
            ;;
        20)
            echo "Arduino Nano w/ ATmega168"
            echo "BOARD_TAG = nano" > .build.config
            ;;
        21)
            echo "Arduino Nano w/ ATmega328"
            echo "BOARD_TAG = nano328" > .build.config
            ;;
        22)
            echo "Arduino Pro or Pro Mini (3.3V, 8 MHz) w/ ATmega168"
            echo "BOARD_TAG = pro" > .build.config
            ;;
        23)
            echo "Arduino Pro or Pro Mini (3.3V, 8 MHz) w/ ATmega328"
            echo "BOARD_TAG = pro328" > .build.config
            ;;
        24)
            echo "Arduino Pro or Pro Mini (5V, 16 MHz) w/ ATmega168"
            echo "BOARD_TAG = pro5v" > .build.config
            ;;
        25)
            echo "Arduino Pro or Pro Mini (5V, 16 MHz) w/ ATmega328"
            echo "BOARD_TAG = pro5v328" > .build.config
            ;;
    esac
done
# show an inputbox
# dev_port=$(dialog --title "Configure Device port" \
# --backtitle "Device port selection" \
# --inputbox "Enter the full path to the port ( e.g. /dev/ttyACM0 ) " 8 60 /dev/ttyACM0)&
 
# get respose
# respose=$?
# get data stored in $OUPUT using input redirection

echo "response = $dev_port"
# echo "name = $name"



