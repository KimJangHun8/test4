#!/bin/sh
#*******************************************************************************
#	SUBJECT		:	UPDATE FILE DOWNLOAD SCRIPT
#	VERSION		:	REV 0.1
#	DATE		:	2019/07/02
#	WRITER		:	OK YOSEOP
#*******************************************************************************

# HOW TO USE
# get_file.sh [TARGET IP] [ARGUMENT]
# -- Argument list --
# 1 : version.txt file download
# 2 : Update(sbc_project_backup.tar.gz) file download

FILENAME=sbc_project_backup.tar.gz
VERSION=version.txt

echo "[Target] IP : $1"
echo ""
# NOT USE
#telnet auto connect script============
#tpipe() {
#sleep 1 ; echo sbc
#sleep 1 ; echo dusrnth
#sleep 1 ; echo su -
#sleep 1 ; echo dusrnth
#sleep 3 ;
#}
#tpipe | telnet $1
#======================================

cd /root/update

echo ""
#1. FTP CONNECT -> DOWNLOAD version.txt FILE FROM SERVER
if [ $2 -eq 1 ]; then 
	ftp -n $1 <<-EOF
	user root dusrnth
	binary
	cd /root/update/shared
	get version.txt
	bye
	EOF
fi
echo ""
#2. FTP CONNECT -> DOWNLOAD UPDATE FILE FROM SERVER
if [ $2 -eq 2 ]; then 
	ftp -n $1 <<-EOF
	user root dusrnth
	binary
	cd /root/update/shared
	get sbc_project_backup.tar.gz
	bye
	EOF

	echo ""
	if [ -f $FILENAME ]; then
	echo "[Target] Firmware update file download complete!!!"
		echo ""
	else
		echo "[Target] Firmware update file download fail!!!"
		echo ""
	fi
fi
