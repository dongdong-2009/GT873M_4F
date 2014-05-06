#!/bin/sh 
echo ""
echo "1.GT812C"
echo "2.GT811D"
echo "3.GT811G"
echo "4.GT873_M_4F4S"
echo ""
read -p "Select Product Type NO.:" ptype
echo ""
if [ $ptype -eq 1 ];then
	echo "You select GT812C"
	sleep 2
	make PRODUCT=GT812C USING_SW_TYPE=MRV
	elif [ $ptype -eq 2 ];then
	echo "You select GT811D"
	sleep 2
	make PRODUCT=GT811D USING_SW_TYPE=RTK
		elif [ $ptype -eq 3 ];then
		echo "You select GT811G"
		sleep 2
		make PRODUCT=GT811G USING_SW_TYPE=RTK
			elif [ $ptype -eq 4 ];then
			echo "You select GT873_M_4F4S"
			sleep 2
			make PRODUCT=GT873_M_4F4S USING_SW_TYPE=RTK
else
echo "Input error!"
fi
