#!/bin/sh
#*******************************************************************************
#	SUBJECT		:	SysConfig Delete SCRIPT
#	VERSION		:	REV 0.3
#	DATE		:	2019/12/21
#	LAST MODIFY :	2019/12/21
#	WRITER		:	OK YOSEOP
#*******************************************************************************

PARAMETER="/root/cycler_data/config/parameter/SysConfig.h"
INC="/project/Cycler/current/cycler/INC/SysConfig.h"
echo "Delete SysConfig file in INC folder"
rm -rf $INC
