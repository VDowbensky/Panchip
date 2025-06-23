#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

//*** <<< Use Configuration Wizard in Context Menu >>> ***

/*
 * 
 */
// <q> Long Delay at Boot Stage
// <i> Add a long delay in front of main() function for debugging purpose
#define BOOT_WITH_LONG_DELAY                                0

// <o> Chip Power Mode
// <0=> LDO
// <1=> DCDC
#define CONFIG_SOC_DCDC_PAN1070                             1

// <o> System Clock
// <48=> 48 MHz (DPLL)
// <32=> 32 MHz (DPLL)
// <i> System main frequency, Unit MHz
#define CONFIG_SYSTEM_CLOCK                                 48

// <o> APB1 Clock Divisor
// <0=> No Divider
// <2=> 2
// <4=> 4
// <6=> 6
// <8=> 8
// <10=> 10
// <12=> 12
// <14=> 14
// <16=> 16
// <i> Divisor of peripheral clocks on APB1, It can only be even numbers.
#define CONFIG_APB1_CLOCK_DIVISOR                           2

// <o> APB2 Clock Divisor
// <0=> No Divider
// <2=> 2
// <4=> 4
// <6=> 6
// <8=> 8
// <10=> 10
// <12=> 12
// <14=> 14
// <16=> 16
// <i> Divisor of peripheral clocks on APB2, It can only be even numbers.
#define CONFIG_APB2_CLOCK_DIVISOR                           2

// <o> 32K Low-Speed Clock Source
// <0=> RCL (32000 Hz)
// <1=> XTL (32768 Hz)
// <i> Select a low-speed clock source
#define CONFIG_LOW_SPEED_CLOCK_SRC                          0

// <e> Enable UART Log
#define CONFIG_UART_LOG_ENABLE                              1

// <o> Log UART Tx Pin
// <0=> P05 (UART0)
// <1=> P11 (UART0)
// <2=> P16 (UART0)
// <3=> P01 (UART1)
// <4=> P10 (UART1)
// <5=> P12 (UART1)
// <6=> P25 (UART1)
// <7=> P31 (UART1)
// <i> Select a UART Tx pin for logging output.
#define CONFIG_LOG_UART_TX_PIN                              2

// <o> Log UART Rx Pin
// <0=> P06 (UART0)
// <1=> P12 (UART0)
// <2=> P15 (UART0)
// <3=> P17 (UART0)
// <4=> P00 (UART1)
// <5=> P07 (UART1)
// <6=> P24 (UART1)
// <7=> P30 (UART1)
// <i> Select a UART Tx pin for logging output.
#define CONFIG_LOG_UART_RX_PIN                              3

// <o> Log UART Baudrate
// <115200=> 115200
// <230400=> 230400
// <460800=> 460800
// <921600=> 921600
// <1000000=> 1M
// <2000000=> 2M
#define CONFIG_LOG_UART_BAUDRATE                            921600
// </e>

// <q> Enable Flash LDO
// <i> Enable the internal 1.8v flash LDO for flash power supply
// <i> instead of the default flash power from SoC VBAT.
#define CONFIG_FLASH_LDO_EN                                 1

// <q> Keep Flash Power in Low Power Mode
// <i> Select this means flash power would be retained in Low Power Mode, and
// <i> there would be a little avg-current increase (about 1uA). The benefit is that
// <i> the large peak current (>15mA) would not occur.
#define CONFIG_KEEP_FLASH_POWER_IN_LP_MODE                  1

// <o> Increase LPLDOH trim value
// <0=> +0
// <1=> +1
// <2=> +2
// <3=> +3
// <4=> +4
// <5=> +5
// <6=> +6
// <7=> +7
// <8=> +8
// <i> Increase LPLDOH voltage for specific LowPower scenario use.
#define CONFIG_SOC_INCREASE_LPLDOH_CALIB_CODE               0

//*** <<< end of configuration section >>>    ***

#endif /* __CONFIGURATION_H__ */
