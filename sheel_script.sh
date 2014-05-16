#!/bin/sh 
echo ""
echo "1.make GT812C"
echo "2.make clean GT812C"
echo "3.make GT811D"
echo "4.make clean GT811D"
echo "5.make GT811G"
echo "6.make clean GT811G"
echo "7.make GT873_M_4F4S"
echo -n "8.make clean GT873_M_4F4S\n\n"
read -p "Select Product Type NO.:" ptype
echo ""
if [ $ptype -eq 1 ];then
	echo "You select make GT812C"
	sleep 2
	make PRODUCT=GT812C USING_SW_TYPE=MRV
	echo "make GT812C end!!"
	elif [ $ptype -eq 2 ];then
	echo "You select make clean GT812C"
	sleep 2
	make clean PRODUCT=GT812C USING_SW_TYPE=MRV
	echo "make clean GT812C end!!"
	elif [ $ptype -eq 3 ];then
	echo "You select make GT811D"
	sleep 2
	make PRODUCT=GT811D USING_SW_TYPE=RTK
	echo "Make GT811D end!!"
	elif [ $ptype -eq 4 ];then
	echo "You select make clean GT811D"
	sleep 2
	make clean PRODUCT=GT811D USING_SW_TYPE=RTK
	echo "Make clean GT811D end!!"
	elif [ $ptype -eq 5 ];then
	echo "You select make GT811G"
	sleep 2
	make PRODUCT=GT811G USING_SW_TYPE=RTK
	echo "Make GT811G end!!"
	elif [ $ptype -eq 6 ];then
	echo "You select make clean GT811G"
	sleep 2
	make clean PRODUCT=GT811G USING_SW_TYPE=RTK
	echo "Make clean GT811G end!!"
	elif [ $ptype -eq 7 ];then
	echo "You select make GT873_M_4F4S"
	sleep 2
	make PRODUCT=GT873_M_4F4S USING_SW_TYPE=RTK
	echo "Make GT873_M_4F4S end!!"
	elif [ $ptype -eq 8 ];then
	echo "You select make clean GT873_M_4F4S"
	sleep 2
	make clean PRODUCT=GT873_M_4F4S USING_SW_TYPE=RTK
	echo "Make clean GT873_M_4F4S end!!"
else
echo "Input error!"
fi
