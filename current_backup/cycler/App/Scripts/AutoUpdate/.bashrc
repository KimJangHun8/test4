# .bashrc

# User specific aliases and functions

alias rm='rm -i'
alias cp='cp -i'
alias mv='mv -i'

# Source global definitions
if [ -f /etc/bashrc ]; then
	. /etc/bashrc
fi

#Cycler
alias Cycler='cd /project/Cycler'
alias Backup='cd /project/Cycler/Backup'
alias parameter='cd /root/cycler_data/config/parameter'
alias sharedMemory='cd /root/cycler_data/config/sharedMemory'
alias caliData='cd /root/cycler_data/config/caliData'
alias log='cd /root/cycler_data/log'
alias resultData='cd /root/cycler_data/resultData'
alias userData='cd /root/cycler_data/userData'
alias tmpData='cd /root/cycler_data/tmpData'
alias hwTest_v1='cd /project/Cycler/current/hwTest/hwTest_v1'
alias hwTest_v1_fpga='cd /project/Cycler/current/hwTest/hwTest_v1_fpga'
alias INC='cd /project/Cycler/current/cycler/INC'
alias AppControl='cd /project/Cycler/current/cycler/App/AppControl'
alias DataSave='cd /project/Cycler/current/cycler/App/DataSave'
alias CaliMeter='cd /project/Cycler/current/cycler/App/CaliMeter'
alias CaliMeter2='cd /project/Cycler/current/cycler/App/CaliMeter2'
alias AnalogMeter='cd /project/Cycler/current/cycler/App/AnalogMeter'
alias AnalogMeter2='cd /project/Cycler/current/cycler/App/AnalogMeter2'
alias DumpMonitor='cd /project/Cycler/current/cycler/App/DumpMonitor'
alias DebugMonitor='cd /project/Cycler/current/cycler/App/DebugMonitor'
alias MainClient='cd /project/Cycler/current/cycler/App/MainClient'
alias FADM='cd /project/Cycler/current/cycler/App/FADM'
alias mControl='cd /project/Cycler/current/cycler/Module/mControl'
alias rt_can='cd /project/Cycler/current/cycler/Module/rt_can'
alias AutoUpdate='cd /project/Cycler/current/cycler/App/AutoUpdate'
alias Scripts='cd /project/Cycler/current/cycler/App/Scripts'
alias Config_backup='cd /project/Cycler/current/cycler/App/Config_backup'
alias Setup='cd /project/Cycler/current/cycler/App/Setup'
alias Update='cd /root/update'
alias LGES_parameter='cd /root/cycler_data/config/parameter/1_LGES_parameter'
alias SDI_parameter='cd /root/cycler_data/config/parameter/2_SDI_parameter'
alias CoolingControl='cd /project/Cycler/current/cycler/App/CoolingControl'