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
#  NOTES: 
#    useful shell script functions
#
#
#

user_prompt()
{
	if [ ! "$1" ] ; then
		echo ""
		exit
	fi
	read -e -p "$1" -i "$2" USERSEL
	echo "$USERSEL"
}
# EXAMPLE:
# UNAME=$(user_prompt "please enter your name : " "Matt")
# echo "UNAME = $UNAME"

user_prompt_yn()
{
	read -r -p "$1 [y/N] " response
	case $response in
    [yY][eE][sS]|[yY]) 
        echo "1"
        ;;
    *)
        echo "0"
        ;;
	esac
}
# EXAMPLE:
# response=$(user_prompt_yn "are you sure? ")
# if [ "$response" == "1" ] ; then
# 	echo "user said yes"
# else
# 	echo "user said no"
# fi


make_menu()
{
	tmpmnu="`date +%m%M%S%s`.sh"
	echo "#!/bin/bash" > $tmpmnu
	echo "#"			>>  $tmpmnu
	echo "tmpmnu_menu()"			>>  $tmpmnu
	echo "{"			>>  $tmpmnu
	echo "cmd=(dialog --keep-tite --menu \"$1\" 22 76 16)" >> $tmpmnu
	echo "options=(" >> $tmpmnu
	count="1"
	for i in $2
	do
		echo "$count \"$i\"" >> $tmpmnu
		count="`expr $count + 1`"
	done
	echo ")" >> $tmpmnu
	echo "choices=\$(\"\${cmd[@]}\" \"\${options[@]}\" 2>&1 >/dev/tty)" >> $tmpmnu
	echo "# clear" >> $tmpmnu
	echo "for choice in \$choices" >> $tmpmnu
	echo "do" >> $tmpmnu
	echo "		case \$choice in" >> $tmpmnu
	count="1"
	for i in $2
	do
		echo "$count)" >> $tmpmnu
		echo "echo \"$i\"" >> $tmpmnu
		#echo "return \"$i\"" >> $tmpmnu
		echo ";;" >> $tmpmnu
		count="`expr $count + 1`"
	done
	echo "    esac" >> $tmpmnu
	echo "done" >> $tmpmnu
	echo "}" >> $tmpmnu
	echo "" >> $tmpmnu
	echo "tmpmnu_menu" >> $tmpmnu
	chmod +x $tmpmnu
	./$tmpmnu
	rm -rf $tmpmnu
	clear
}
# EXAMPLE:
# res=$(make_menu "Test Menu" "George Natasha Matt Sonja")
# echo "res = $res"


countdown()
{
	echo -e "\t Please press [CTRL]-C in the next $1 seconds to cancel"
	secs=$1
	while [ $secs -gt 0 ]; do
		echo -ne "\t[ $secs ]\033[0K\r"
		sleep 1
		: $((secs--))
	done
	echo ""
}
# EXAMPLE:
# countdown 5 
# ( shows user countdown for 5 seconds )

countdown2()
{
	echo -e "\t Please press [CTRL]-C in the next $1 seconds to cancel"
	echo -e "\t Press [Enter] to continue..."
	
	secs=$1
	while [ $secs -gt 0 ]; do
		echo -ne "\t[ $secs ]\033[0K\r"
		: $((secs--))
		read -t 1 line
 		if [ "$?" == "0" ] ; then
			if [ "$line" == "" ] ; then
				secs=0
			fi
		fi
	done
	echo ""
}
# EXAMPLE:
# countdown 5 
# ( shows user countdown for 5 seconds )
# user pressing [ Enter ] will break;