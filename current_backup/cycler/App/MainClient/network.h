#ifndef __NETWORK_H__
#define __NETWORK_H__

int		InitNetwork(void);
int		NetworkPacket_Receive(void);
void	NetworkPacket_Parsing(void);
int		NetworkCommand_Receive(void);
int		NetworkCommand_Parsing(void);
int		Parsing_NetworkEvent(void);

int		CmdHeader_Check(char *);
int		Check_ReplyCmd(char *);

int		rcv_cmd_module_info_request(void);
int		rcv_cmd_cali_meter_connect(void);
int		rcv_cmd_cali_start(void);
int		rcv_cmd_cali_update(void);
int		rcv_cmd_cali_not_update(void);		//add <--20180707
int		rcv_cmd_cali_lan_connect(void);		//add <--20161220
int		rcv_cmd_temp_cali_point_set(void);    //200102 add
int		rcv_cmd_temp_cali_start(void);	      //200102 add
int		rcv_cmd_temp_cali_update(void);	 	  //200102 add
int		rcv_cmd_temp_cali_info_request(void); //200102 add
int		rcv_cmd_temp_cali_backup_read(void); //200102 add
int		rcv_cmd_response(void);
int		rcv_cmd_run(void);
int		rcv_cmd_dll_stop(void);			//20210871 hun
int		rcv_cmd_stop(void);
int		rcv_cmd_dll_pause(void);		//20210817 hun
int		rcv_cmd_pause(void);
int		rcv_cmd_continue(void);
int		rcv_cmd_chamber_wait_release(void); //211022
int		rcv_cmd_init(void);
int		rcv_cmd_next_step(void);
int		rcv_cmd_shutdown(void); 		//20181108
int		rcv_cmd_parameter_update(void); //190801 lyhw
int		rcv_cmd_ch_data_backup(void); 	//add <--20150512 oys
int		rcv_cmd_ch_data_restore(void); 	//add <--20150512 oys
int		rcv_cmd_reset(void);
int		rcv_cmd_testcond_start(void);
int		rcv_cmd_testcond_safety(void);
int		rcv_cmd_testcond_step(void);
int		rcv_cmd_testcond_end(void);
int		rcv_cmd_step_cond_request(void);
int		rcv_cmd_step_cond_update(void);
int		rcv_cmd_safety_cond_request(void);
int		rcv_cmd_safety_cond_update(void);
int		rcv_cmd_reset_reserved_cmd(void);
int		rcv_cmd_sensor_limit_set(void);
int		rcv_cmd_comm_check_reply(void);
int		rcv_cmd_comm_check(void);
int		rcv_cmd_testcond_user_pattern(int); //add <--20071106
int		rcv_cmd_testcond_user_pattern_NoUserData(int); //add <--20200427
int		rcv_cmd_testcond_user_map(void); //add <--20111215
int		rcv_cmd_real_time_reply(void); //add <--20131228
int		rcv_cmd_set_measure_data(void); //add <--20071106
int		rcv_cmd_set_swelling_data(void); //add <--210316 lyhw
int		rcv_cmd_set_gas_measure_data(void); //add <--210316 lyhw
int		rcv_cmd_module_set_data(void);

int		rcv_cmd_aux_info_request(void); //add <--20090424
int		rcv_cmd_can_info_request(void); //add <--20090424
int		rcv_cmd_ch_attribute_request(void); // add <--20090424
int		rcv_cmd_ch_attribute_set(void); //add <--20090424
int		rcv_cmd_chamber_ch_no_request(void); // add <--210415 hun
int		rcv_cmd_chamber_ch_no_set(void); //add <--210415 hun
int		rcv_cmd_sw_fault_config_request(void); // add <--210415 hun
int		rcv_cmd_sw_fault_config_set(void); //add <--210415 hun
int		rcv_cmd_ch_compdata_request(void); // add <--20120206
int		rcv_cmd_ch_compdata_set(void); //add <--20120206
int		rcv_cmd_aux_set(void); //add <--20090424
int		rcv_cmd_can_set(void); //add <--20090424
int 	rcv_cmd_gui_version_info(void); //add <--20190829
int		rcv_cmd_hwfault_request(void);	//hun_200219
int		rcv_cmd_ch_end_data_recovery(void);	//hun_200219
int 	rcv_cmd_ch_convert_adv_cycle_step(void); //hun_200219
int		rcv_cmd_cell_diagnosis_alarm(void);
int		rcv_cmd_acir_value(void);	//220124 hun
		
int		send_cmd_response(char *, int);
int		send_cmd_ch_data(void);
int		send_cmd_ch_data2(void);
int		send_cmd_ch_pulse_data(int);
void	send_cmd_ch_pulse_data_2(int, int);
int		send_cmd_ch_pulse_data_iec(int);		//add <--20171227
void	send_cmd_ch_pulse_data_2_iec(int, int);	//add <--20171227
int		send_cmd_fadm_pulse_data(int);
void	send_cmd_fadm_pulse_data_2(int, int);
int		send_cmd_sensor_data(void);
void	send_cmd_step_cond_update(int, int); //add <--20160510
int		send_cmd_step_cond_reply(void);
int		send_cmd_safety_cond_reply(void);
void	send_cmd_meter_connect_reply(void);
void	send_cmd_cali_start_reply(int, int, int);
void	send_cmd_cali_normal_result(int, int);
void	send_cmd_cali_check_result(int, int);
void	send_cmd_comm_check_reply(void);
void	send_cmd_comm_check(void);
void	send_cmd_emg_status(int, int);
void	send_cmd_jig_status();
void	send_cmd_unknown(int, int);
void	send_cmd_real_time_request(void); //add <--20131228
int		send_cmd_module_info_reply(void);
int		send_cmd_set_measure_data(int); //add <--20071106
void 	send_cmd_acir_standby(int);	//220124 hun

int		send_cmd_aux_info_reply(void); //add <--20090424
int		send_cmd_can_info_reply(void); //add <--20090424
int		send_cmd_ch_attribute_reply(void); //add <--20090424
int		send_cmd_chamber_ch_no_reply(void); //add <--20210415 hun
int		send_cmd_sw_fault_config_reply(void); //add <--20210415 hun
int		send_cmd_ch_compdata_reply(void); //add <--20120206
int		send_cmd_temp_cali_info_reply(void);   //200102 add
int		send_cmd_temp_cali_backup_read_reply(int);   //200102 add

void	send_cmd_ch_data_backup_reply(void); //add <--20150512 oys
void	send_cmd_ch_data_restore_reply(void); //add <--20150512 oys
int		send_cmd_hwfault_reply(void);	//hun_200219

void	make_header(char *, char, int, int, int);
void	make_header_2(char *, char, int, int, int, int);
int		send_command(char *, int, int);
int		get_reply_cmdid(char *);

int		Check_NetworkState(void);
void	network_ping(void);
int		check_network_timeout(void);
void	check_cmd_reply_timeout(void);
int		make_user_pattern_file(int, int, S_MAIN_RCV_CMD_TESTCOND_USER_PATTERN, int);
int		make_user_pattern_file_NoUserData(int, S_MAIN_RCV_CMD_TESTCOND_USER_PATTERN, int); //add <-- 20200427
int		make_user_map_file(int, int, S_MAIN_RCV_CMD_TESTCOND_USER_MAP, int);
int		user_data_copy(int, int, int); //add <-- 191029
int		make_user_pattern_file_copy(int, int, int, int); //add <--20200427
int		delete_user_pattern_file(int);
int		delete_user_map_file(int);
int		delete_user_data_file(void);
int		rcv_cmd_testcond_tracking_file(void); //210609
int		make_tracking_file_copy(int, int, int, int); //210609
int		delete_tracking_file(int); //210609

void	send_cmd_ch_10mS_data(int);
void	send_cmd_ch_10mS_data_2(int, int, int);
//20180710 add for digital
int		send_cmd_ch_pulse_data_10mS(int);
void	send_cmd_ch_pulse_data_2_10mS(int, int, int);
//20171008 sch add
int		rcv_cmd_limit_user_vi_request(void);
int		send_cmd_limit_user_vi_reply(void);
int		rcv_cmd_limit_user_vi_set(void);
//210204 lyhw
int		rcv_cmd_hwfault_config_request(void);
int		rcv_cmd_hwfault_config_set(void);
int		send_cmd_hwfault_config_reply(void);
//20180717 sch add
int 	rcv_cmd_ch_recovery(void);
int 	rcv_cmd_goto_count_recovery(void);
void	send_cmd_goto_count_update(int, int);
int		rcv_cmd_buzzer_off(void);
int		rcv_cmd_buzzer_on(void);
int		rcv_cmd_dio_control(void);		//190809 lyhw
int		rcv_cmd_calimeter_set(void);	//190809 lyhw
void	Convert_ADV_CYCLE_Step(int,int,int);	//190809 lyhw
int 	rcv_cmd_fault_channel_code(void);
int 	rcv_cmd_fault_alarm_request(void);
int		rcv_cmd_fault_gui_code(void);	//210917 LJS
int		rcv_cmd_goto_count_request(void);	//20210929 hun	
int		rcv_cmd_goto_count_data(void);	//20210929 hun	
void	send_cmd_goto_count(int,int,int);	//20210929 hun	
void	recovery_data(int,int,int);	//20210929 hun	
int 	rcv_cmd_lges_fault_config_request(void);	//211025 hun
int 	rcv_cmd_lges_fault_config_set(void);	//211025 hun
int 	send_cmd_lges_fault_config_reply(void); //211025 hun
int 	rcv_cmd_sdi_cc_cv_hump_config_request(void);	//211125 hun
int 	rcv_cmd_sdi_cc_cv_hump_config_set(void);	//211125 hun
int		send_cmd_sdi_cc_cv_hump_config_reply(void);	//211125 hun
int 	rcv_cmd_sdi_pause_save_config_request(void);	//211209 hun
int 	rcv_cmd_sdi_pause_save_config_set(void);	//211209 hun
int		send_cmd_sdi_pause_save_config_reply(void);	//211209 hun
int		rcv_cmd_dyson_maintenance_config_set(void); //220121 jsh
int		rcv_cmd_dyson_maintenance_config_request(void); //220121 jsh
int		send_cmd_dyson_maintenance_config_reply(void); //220121 jsh
int		rcv_cmd_chamber_fault(void); //220214_hun
#endif
