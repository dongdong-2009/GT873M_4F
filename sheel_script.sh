#!/bin/sh 
echo ""
SWHOME=$(cd `dirname $0`;pwd)
export SWHOME
echo "SWHOME = "$SWHOME
echo ""
echo "[1] make GT812_C"
echo "[2] make GT812C_B"
echo "[3] make GT812C_PSE"
echo "[4] make GT811D"
echo "[5] make GT811G"
echo "[6] make GT873_M_4F4S"
echo "[7] make clean"
echo ""
read -p "Select Product Type NO.:" ptype
echo ""

if [ $ptype -eq 1 ];then
	PRODUCT=GT812C
	USING_SW_TYPE=MRV
elif [ $ptype -eq 2 ];then
	PRODUCT=GT812C_B
	USING_SW_TYPE=MRV
elif [ $ptype -eq 3 ];then
	PRODUCT=GT812C_PSE
	USING_SW_TYPE=MRV
elif [ $ptype -eq 4 ];then
	PRODUCT=GT811D
	USING_SW_TYPE=RTK
elif [ $ptype -eq 5 ];then
	PRODUCT=GT811G
	USING_SW_TYPE=RTK
elif [ $ptype -eq 6 ];then
	PRODUCT=GT873_M_4F4S
	USING_SW_TYPE=RTK
elif [ $ptype -eq 7 ];then
	make clean USING_SW_TYPE=MRV
	make clean USING_SW_TYPE=RTK
	exit 0
else
echo "Select Error!"
exit 0
fi

echo "You select make ${PRODUCT}"
sleep 2
if make PRODUCT=${PRODUCT} USING_SW_TYPE=${USING_SW_TYPE}
then
	echo "============================="
	echo "make ${PRODUCT} successfully!!"
	echo "============================="
else
	echo "====================="
	echo "======= Error ======="
	echo "====================="

fi
