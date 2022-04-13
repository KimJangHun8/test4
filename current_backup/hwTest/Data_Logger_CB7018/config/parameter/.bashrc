# .bashrc

# User specific aliases and functions

alias cp='cp -i'
alias mv='mv -i'
alias rm='rm -i'

alias ls='ls -F --show-control-chars --color=auto'
alias l='ls -l'
alias ll='ls -al'

alias tart='tar tvvzf'
alias tarx='tar xvvzf'
alias tarc='tar cvvzf'

#main NS_EDLC_Tester
alias AppControl='cd /project/NS_EDLC_Tester/current/cycler/App/AppControl'
alias MainClient='cd /project/NS_EDLC_Tester/current/cycler/App/MainClient'
alias DataSave='cd /project/NS_EDLC_Tester/current/cycler/App/DataSave'
alias HP2000Comm='cd /project/NS_EDLC_Tester/current/cycler/App/HP2000Comm'
alias Monitor='cd /project/NS_EDLC_Tester/current/cycler/App/DebugMonitor'
alias DumpMonitor='cd /project/NS_EDLC_Tester/current/cycler/App/DumpMonitor'
alias lControl='cd /project/NS_EDLC_Tester/current/cycler/Module/lControl'
alias INC='cd /project/NS_EDLC_Tester/current/cycler/INC'
alias config='cd /project/NS_EDLC_Tester/config'
alias parameter='cd /project/NS_EDLC_Tester/config/parameter'
alias log='cd /project/NS_EDLC_Tester/config/log'
alias ver64v23='cd /project/NS_EDLC_Tester/current/hwTest/ver64v23'

#switching formation
#alias sControl='cd /project/sw_f/current/form/Module/sControl'
#alias SerialInterface='cd /project/sw_f/current/form/Module/SerialInterface'
#alias Client='cd /project/sw_f/current/form/App/Client'
#alias DataBase='cd /project/sw_f/current/form/App/DataBase'
#alias Monitor='cd /project/sw_f/current/form/App/Monitor'
#alias DumpMonitor='cd /project/sw_f/current/form/App/DumpMonitor'
#alias INC='cd /project/sw_f/current/form/INC'

#skc 5V/3A formation
#alias Client='cd /project/SKC/current_Test2/form/App/Client'
#alias DataBase='cd /project/SKC/current_Test2/form/App/DataBase'
#alias Monitor='cd /project/SKC/current_Test2/form/App/Monitor'
#alias INC='cd /project/SKC/current_Test2/form/INC'
#alias ver64v22='cd /project/SKC/current_Test2/hwTest/ver64v22'
#alias lControl='cd /project/SKC/current_Test2/form/Module/lControl'

#labeler
#alias AppControl='cd /project/M_project/Labeler/current/edlc/App/AppControl'
#alias MainClient='cd /project/M_project/Labeler/current/edlc/App/MainClient'
#alias HP2000Comm='cd /project/M_project/Labeler/current/edlc/App/HP2000Comm'
#alias PLCComm='cd /project/M_project/Labeler/current/edlc/App/PLCComm'
#alias Monitor='cd /project/M_project/Labeler/current/edlc/App/Monitor'
#alias DumpMonitor='cd /project/M_project/Labeler/current/edlc/App/DumpMonitor'
#alias lControl='cd /project/M_project/Labeler/current/edlc/Module/lControl'
#alias INC='cd /project/M_project/Labeler/current/edlc/INC'

#main c_dcr
#alias AppControl='cd /project/M_project/C_DCR/Main_C_DCR/current/edlc/App/AppControl'
#alias MainClient='cd /project/M_project/C_DCR/Main_C_DCR/current/edlc/App/MainClient'
#alias DataSave='cd /project/M_project/C_DCR/Main_C_DCR/current/edlc/App/DataSave'
#alias HP2000Comm='cd /project/M_project/C_DCR/Main_C_DCR/current/edlc/App/HP2000Comm'
#alias Monitor='cd /project/M_project/C_DCR/Main_C_DCR/current/edlc/App/Monitor'
#alias DumpMonitor='cd /project/M_project/C_DCR/Main_C_DCR/current/edlc/App/DumpMonitor'
#alias lControl='cd /project/M_project/C_DCR/Main_C_DCR/current/edlc/Module/lControl'
#alias INC='cd /project/M_project/C_DCR/Main_C_DCR/current/edlc/INC'
#alias config='cd /project/M_project/C_DCR/Main_C_DCR/config'
#alias app='cd /project/M_project/C_DCR/Main_C_DCR/currentO/edlc/app'
#alias mod='cd /project/M_project/C_DCR/Main_C_DCR/currentO/edlc/mod'
#alias inc='cd /project/M_project/C_DCR/Main_C_DCR/currentO/edlc/inc'

#sub c_dcr
#alias AppControl='cd /project/M_project/C_DCR/Sub_C_DCR/current/edlc/App/AppControl'
#alias MainClient='cd /project/M_project/C_DCR/Sub_C_DCR/current/edlc/App/MainClient'
#alias DataSave='cd /project/M_project/C_DCR/Sub_C_DCR/current/edlc/App/DataSave'
#alias Monitor='cd /project/M_project/C_DCR/Sub_C_DCR/current/edlc/App/Monitor'
#alias DumpMonitor='cd /project/M_project/C_DCR/Sub_C_DCR/current/edlc/App/DumpMonitor'
#alias lControl='cd /project/M_project/C_DCR/Sub_C_DCR/current/edlc/Module/lControl'
#alias INC='cd /project/M_project/C_DCR/Sub_C_DCR/current/edlc/INC'
#alias config='cd /project/M_project/C_DCR/Sub_C_DCR/config'
#alias app='cd /project/M_project/C_DCR/Sub_C_DCR/currentO/edlc/app'
#alias mod='cd /project/M_project/C_DCR/Sub_C_DCR/currentO/edlc/mod'
#alias inc='cd /project/M_project/C_DCR/Sub_C_DCR/currentO/edlc/inc'

# Source global definitions
if [ -f /etc/bashrc ]; then
	. /etc/bashrc
fi
