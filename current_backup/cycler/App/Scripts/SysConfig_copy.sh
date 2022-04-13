#!/bin/sh
#*******************************************************************************
#	SUBJECT		:	SysConfig COPY SCRIPT
#	VERSION		:	REV 0.3
#	DATE		:	2019/12/21
#	LAST MODIFY :	2019/12/21
#	WRITER		:	OK YOSEOP
#*******************************************************************************

PARAMETER="/root/cycler_data/config/parameter/SysConfig.h"
INC="/project/Cycler/current/cycler/INC/SysConfig.h"
if [ -f "$PARAMETER" ]; then
	echo "SysConfig file in parameter folder exist"
	if [ -L "$INC" ]; then
		echo "SysConfig link file in INC folder exist"
	else
		if [ -f "$INC" ]; then
			echo "SysConfig file in INC folder exist"
			echo "SysConfig file updated and made link"
			rm -rf $PARAMETER
			mv $INC /root/cycler_data/config/parameter
			ln -s $PARAMETER $INC
		else
			echo "Made SysConfig file link"
			ln -s $PARAMETER $INC
		fi
	fi
else
	echo "SysConfig file in parameter folder not exist"
	if [ -L "$INC" ]; then
		echo "SysConfig link file in INC folder exist"
	else
		if [ -f "$INC" ]; then
			echo "SysConfig file in INC folder exist"
			echo "SysConfig file updated and made link"
			rm -rf $PARAMETER
			mv $INC /root/cycler_data/config/parameter
			ln -s $PARAMETER $INC
		else
			echo "SysConfig file in INC folder not exist"
		fi
	fi
fi
