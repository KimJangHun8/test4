#ifndef __SJA1000_H__
#define __SJA1000_H__

//sja1000 registers
#define CONTROL					0	//control(BASIC)
#define MODE                   	0   //mode(PELICAN)
#define COMMAND                1
#define CHIPSTATUS             2
#define INTERRUPT_STATUS       3
#define INTERRUPT_ENABLE       4      // acceptance code
#define TIMING0                6      // bus timing 0
#define TIMING1                7      // bus timing 1
#define OUTPUT_CONTROL         8      // output control
#define TESTREG                9

#define ARBIT_LOST_CAPTURE    11      // transmit buffer: Identifier
#define ERROR_CODE_CAPTURE    12      // RTR bit und data length code
#define ERROR_WARNING_LIMIT   13      // start byte of data field
#define RX_ERROR_COUNTER      14
#define TX_ERROR_COUNTER      15

#define ACCEPTANCE_CODE_BASE  16
#define RECEIVE_FRAME_BASE    16
#define TRANSMIT_FRAME_BASE   16

#define ACCEPTANCE_MASK_BASE  20

#define RECEIVE_MSG_COUNTER   29
#define RECEIVE_START_ADDRESS 30

#define CLKDIVIDER            31      // set bit rate and pelican mode

// important sja1000 register contents, MODE register
#define SLEEP_MODE             0x10
#define ACCEPT_FILTER_MODE     0x08
#define SELF_TEST_MODE         0x04
#define LISTEN_ONLY_MODE       0x02
#define RESET_MODE             0x01
#define NORMAL_MODE            0x00

// COMMAND register
#define CLEAR_DATA_OVERRUN     0x08
#define RELEASE_RECEIVE_BUFFER 0x04
#define ABORT_TRANSMISSION     0x02
#define TRANSMISSION_REQUEST   0x01

// CHIPSTATUS register
#define BUS_STATUS             0x80
#define ERROR_STATUS           0x40
#define TRANSMIT_STATUS        0x20
#define RECEIVE_STATUS         0x10
#define TRANS_COMPLETE_STATUS  0x08
#define TRANS_BUFFER_STATUS    0x04
#define DATA_OVERRUN_STATUS    0x02
#define RECEIVE_BUFFER_STATUS  0x01

// INTERRUPT STATUS register
#define BUS_ERROR_INTERRUPT    0x80
#define ARBIT_LOST_INTERRUPT   0x40
#define ERROR_PASSIV_INTERRUPT 0x20
#define WAKE_UP_INTERRUPT      0x10
#define DATA_OVERRUN_INTERRUPT 0x08
#define ERROR_WARN_INTERRUPT   0x04
#define TRANSMIT_INTERRUPT     0x02
#define RECEIVE_INTERRUPT      0x01

// INTERRUPT ENABLE register
#define BUS_ERROR_INTERRUPT_ENABLE    0x80
#define ARBIT_LOST_INTERRUPT_ENABLE   0x40
#define ERROR_PASSIV_INTERRUPT_ENABLE 0x20
#define WAKE_UP_INTERRUPT_ENABLE      0x10
#define DATA_OVERRUN_INTERRUPT_ENABLE 0x08
#define ERROR_WARN_INTERRUPT_ENABLE   0x04
#define TRANSMIT_INTERRUPT_ENABLE     0x02
#define RECEIVE_INTERRUPT_ENABLE      0x01

// OUTPUT CONTROL register
#define OUTPUT_CONTROL_TRANSISTOR_P1  0x80
#define OUTPUT_CONTROL_TRANSISTOR_N1  0x40
#define OUTPUT_CONTROL_POLARITY_1     0x20
#define OUTPUT_CONTROL_TRANSISTOR_P0  0x10
#define OUTPUT_CONTROL_TRANSISTOR_N0  0x08
#define OUTPUT_CONTROL_POLARITY_0     0x04
#define OUTPUT_CONTROL_MODE_1         0x02
#define OUTPUT_CONTROL_MODE_0         0x01

// TRANSMIT or RECEIVE BUFFER
#define BUFFER_EFF                    0x80 // set for 29 bit identifier
#define BUFFER_RTR                    0x40 // set for RTR request
#define BUFFER_DLC_MASK               0x0f

// CLKDIVIDER register
#define CAN_MODE                      0x80
#define CAN_BYPASS                    0x40
#define RXINT_OUTPUT_ENABLE           0x20
#define CLOCK_OFF                     0x08
#define CLOCK_DIVIDER_MASK            0x07

// additional informations
#define CLOCK_HZ                  	24000000 // crystal frequency

// time for mode register to change mode
#define MODE_REGISTER_SWITCH_TIME 100 // msec 

// some CLKDIVIDER register contents, hardware architecture dependend 
#define PELICAN_SINGLE  (CAN_MODE | CAN_BYPASS | 0x07 | CLOCK_OFF)
#define PELICAN_MASTER  (CAN_MODE | CAN_BYPASS | 0x07            )
#define PELICAN_DEFAULT (CAN_MODE |              0x07 | CLOCK_OFF)

// hardware depended setup for OUTPUT_CONTROL register
#define OUTPUT_CONTROL_SETUP ( OUTPUT_CONTROL_TRANSISTOR_P1 | OUTPUT_CONTROL_TRANSISTOR_N1 | OUTPUT_CONTROL_TRANSISTOR_P0 | OUTPUT_CONTROL_TRANSISTOR_N0 | OUTPUT_CONTROL_MODE_1)

// the interrupt enables
#define INTERRUPT_ENABLE_SETUP (RECEIVE_INTERRUPT_ENABLE | TRANSMIT_INTERRUPT_ENABLE | DATA_OVERRUN_INTERRUPT_ENABLE | BUS_ERROR_INTERRUPT_ENABLE | ERROR_PASSIV_INTERRUPT_ENABLE)

// the maximum number of handled messages in one interrupt 
#define MAX_MESSAGES_PER_INTERRUPT 8

// the maximum number of handled sja1000 interrupts in 1 handler entry
#define MAX_INTERRUPTS_PER_ENTRY   4

// constants from Arnaud Westenberg email:arnaud@wanadoo.nl
#define MAX_TSEG1  15
#define MAX_TSEG2  7
#define BTR1_SAM   (1<<1)

#endif
