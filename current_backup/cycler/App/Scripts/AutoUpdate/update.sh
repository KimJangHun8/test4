#!/bin/bash
#*******************************************************************************
#	SUBJECT		:	FIRMWARE AUTO UPDATE SCRIPT
#	VERSION		:	REV 0.1
#	DATE		:	2019/07/02
#	WRITER		:	OK YOSEOP
#*******************************************************************************

cd /root/update/

rm -rf project_backup_program
rm -rf current_backup
rm -rf config_backup

tar xzf sbc_project_backup.tar.gz
mv ./project_backup_program/*.tar.gz ./

tar xzf current_backup.tar.gz
tar xzf config_backup.tar.gz

rm -rf /project/Cycler/current
mv current_backup /project/Cycler/current
echo ">> current update end"
echo ""

cd /root/cycler_data/

rm -rf config_backup
mv config config_backup

mv /root/update/config_backup config
mv -f config_backup/caliData/* config/caliData
mv -f config_backup/parameter/* config/parameter
rm -rf config_backup
echo ">> config update end"
echo ""

cd /root/update

rm -rf /root/update/lastUpdate/*
mv -f sbc_project_backup.tar.gz lastUpdate
mv -f version.txt lastUpdate
rm -rf project_backup_program
rm -rf current_backup.tar.gz
rm -rf config_backup.tar.gz
echo ">> Firmware update end"
echo ""
