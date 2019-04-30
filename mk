#!/bin/sh
# create by liuqiming
# date: 2011-11-24

MODE=$1

CPU_NUM=$(cat /proc/cpuinfo |grep processor|wc -l)
CPU_NUM=$((CPU_NUM+1))

# 如果没有传入参数，则提示错误
if [ -z $MODE ]; then
	echo "ERROR: Must set the compile mode"
	echo "Example: $0 nand or $0 inand"
	exit
fi

# 如果传入参数大于1，则提示错误
if [ $# -gt 1 ]; then
	echo -e "\033[40;32m"
	echo "Usage: ./mk [mode]"
	echo "       mode:    nand, inand"
	echo "compile: ./mk nand or ./mk inand"
	echo -e "\033[40;37m" 
fi

if [ $MODE = "nand" ]; then
	make distclean
	make x210_nand_config
	make -j${CPU_NUM}
	mv u-boot.bin uboot_nand.bin
	if [ -f uboot_nand.bin ]; then
		cp uboot_nand.bin ../out/release/uboot.bin
		echo "^_^ uboot_nand.bin is finished successful!"
		exit
	else
		echo "make error,cann't compile u-boot.bin!"
		exit
	fi
elif [ $MODE = "inand" ]; then
	make distclean
	make x210_sd_config
	make -j${CPU_NUM}
	mv u-boot.bin uboot_inand.bin
	if [ -f uboot_inand.bin ]; then 
		cp uboot_inand.bin ../out/release/uboot.bin
		echo "^_^ uboot_inand.bin is finished successful!"
		exit
	else
		echo "make error,cann't compile u-boot.bin!"
		exit
	fi
else
	echo -e "\033[40;32m"
	echo "Usage: ./mk [mode]"
	echo "       mode:    nand, inand"
	echo "compile: ./mk nand or ./mk inand"
	echo -e "\033[40;37m" 
	exit
fi


