#!/bin/sh
#*******************************************************************************
#	SUBJECT		:	PROJECT COPY SCRIPT
#	VERSION		:	REV 0.2
#	DATE		:	2019/09/10
#	LAST MODIFY :	2019/09/10
#	WRITER		:	OK YOSEOP
#*******************************************************************************

# HOW TO USE
# project_copy.sh [TARGET IP] [OPTION]
# [OPTION] : NONE or 0 : Don't reboot, 1 : Copy after reboot

FILENAME=sbc_project_backup.tar.gz

echo "[Target] IP : $1"
echo ""
if [ $2 -eq 1 ]; then
	 FLAG=1
	else
	 FLAG=0
fi
#1. PROJECT FILES COMPRESSION
rm -rf /project/Cycler/*.tar.gz

/project/Cycler/current/cycler/App/AppControl/backup_program

mv /project/Cycler/*.tar.gz $FILENAME

echo "FTP File Transfer Start"
sleep 1
#2. FTP CONNECT -> FILE UPLOAD
	ftp -n $1 <<-EOF
	user root dusrnth
	binary
	put sbc_project_backup.tar.gz
	bye
	EOF
rm -rf $FILENAME
echo "FTP File Transfer Complete"
sleep 5
echo "Telnet connect -> project file change process"
#3. TARGET TELNET CONNECT -> CURRENT FILE CHANGE
#telnet auto connect script============
if [ $FLAG -eq 0 ]; then
tpipe() {
sleep 5 ; echo sbc
sleep 1 ; echo dusrnth
sleep 1 ; echo su -
sleep 1 ; echo dusrnth
sleep 1 ; echo cd /root
sleep 1 ; echo rm -rf project_backup_program
sleep 1 ; echo cd /root/cycler_data
sleep 1 ; echo rm -rf config
sleep 1 ; echo cd /project/Cycler
sleep 1 ; echo rm -rf current
sleep 1 ; echo cd /root
sleep 1 ; echo tar xzvf $FILENAME
sleep 1 ; echo cd project_backup_program
sleep 1 ; echo tar xzvf config_backup.tar.gz
sleep 1 ; echo tar xzvf current_backup.tar.gz
sleep 1 ; echo mv config_backup /root/cycler_data/config
sleep 1 ; echo mv current_backup /project/Cycler/current
sleep 1 ; echo cd /root
sleep 1 ; echo rm -rf project_backup_program
sleep 1 ; echo rm -rf *.tar.gz 
sleep 1 ; echo cd /project/Cycler/current/cycler/App/AppControl 
sleep 1 ; echo make dir
sleep 1 ; echo make clear_memory
sleep 1 ; echo exit
}
tpipe | telnet $1
fi
if [ $FLAG -eq 1 ]; then
tpipe() {
sleep 5 ; echo sbc
sleep 1 ; echo dusrnth
sleep 1 ; echo su -
sleep 1 ; echo dusrnth
sleep 1 ; echo cd /root
sleep 1 ; echo rm -rf project_backup_program
sleep 1 ; echo cd /root/cycler_data
sleep 1 ; echo rm -rf config
sleep 1 ; echo cd /project/Cycler
sleep 1 ; echo rm -rf current
sleep 1 ; echo cd /root
sleep 1 ; echo tar xzvf $FILENAME
sleep 1 ; echo cd project_backup_program
sleep 1 ; echo tar xzvf config_backup.tar.gz
sleep 1 ; echo tar xzvf current_backup.tar.gz
sleep 1 ; echo mv config_backup /root/cycler_data/config
sleep 1 ; echo mv current_backup /project/Cycler/current
sleep 1 ; echo cd /root
sleep 1 ; echo rm -rf project_backup_program
sleep 1 ; echo rm -rf *.tar.gz 
sleep 1 ; echo cd /project/Cycler/current/cycler/App/AppControl 
sleep 1 ; echo make dir
sleep 1 ; echo make clear_memory
sleep 2 ; echo reboot
sleep 1 ; echo exit
}
tpipe | telnet $1
fi
#======================================

echo "[Target:$1] project file copy complete!!!"
