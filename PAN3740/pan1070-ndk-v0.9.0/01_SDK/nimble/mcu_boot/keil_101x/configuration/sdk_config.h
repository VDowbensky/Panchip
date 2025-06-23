#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H

//*** <<< Use Configuration Wizard in Context Menu >>> ***

//******************************************************************************
// <h> Bootloader Config

// <q> Enable UART DFU
#define BOOT_ENABLE_UART_DFU                                0

// <e> Enable USB DFU
#define BOOT_ENABLE_USB_DFU                                 1
// <o> USB DFU Mode
// <0x00=>  Mode 0
// <0x01=>  Mode 1
// <0x02=>  Mode 2
// <0x03=>  Mode 3
// <i> Select USB DFU mode, which would notify DFU Host (Panchip DFU Tool) to decide what to do when new image has been successfully upgrade to SoC flash (CRC ok).
// <i> Mode 0:
// <i> -> [Behavior] a. Jump to App (cmd 0x04 DFU End, param 0x00); b. DFU End
// <i> -> [Note] For this mode, bootloader itself has responsibility to provide a way for entering USB DFU flow (such as key press event).
// <i> Mode 1:
// <i> -> [Behavior] a. Erase DFU flag on flash (cmd 0x06 DFU Force Upgrade, param 0x01); b. Stay in bootloader unless reset; c. DFU End
// <i> -> [Note] App should set DFU flag to indicate bootloader to enter USB DFU proc.
// <i> Mode 2:
// <i> -> [Behavior] a. Erase DFU flag on flash (cmd 0x06 DFU Force Upgrade, param 0x01); b. Jump to App (cmd 0x04 DFU End, param 0x00); c. DFU End
// <i> -> [Note] App should set DFU flag to indicate bootloader to enter USB DFU proc.
// <i> Mode 3:
// <i> -> [Behavior] a. Jump to App (cmd 0x04 DFU End, param 0x00); b. Wait for App USB enum OK; c. App Erase DFU flag on flash (cmd 0x06 DFU Force Upgrade, param 0x01); c. DFU End
// <i> -> [Note] App should support USB HID, and support the DFU cmd 0x06 (DFU Force Upgrade) to set/erase DFU flag.
#define BOOT_USB_DFU_MODE                                   0x00
// </e> Enable USB DFU End

// <q> Enable Private 2.4G OTA
#define BOOT_ENABLE_PRF_OTA                                 0

// <h> Flash Partition Config

// <o> Chip Flash Size
// <0x7F000=>  508 (KB)
// <0x3F000=>  252 (KB)
// <i> You can select chip flash size in the pull-down list, but modify the list only when you know what you are doing!
#define CONFIG_FLASH_SIZE                                   0x3F000

// <o> Bootloader Flash Partition Size (KB)
// <i> Note that partition size should be multiple of 4KB, and should be same with the APP project config!
#define CONFIG_FLASH_PARTITION_BOOTLOADER_SIZE_KB           40

// <o> App Flash Partition Size (KB)
// <i> Note that partition size should be multiple of 4KB, and should be same with the APP project config!
#define CONFIG_FLASH_PARTITION_APP_SIZE_KB                  184

// <o> App Backup Flash Partition Size (KB)
// <i> Note that partition size should be multiple of 4KB, and should be same with the APP project config!
#define CONFIG_FLASH_PARTITION_APP_BACKUP_SIZE_KB           0

// <o> KVStore Flash Partition Size (KB)
// <i> Note that partition size should be multiple of 8KB!
#define CONFIG_FLASH_PARTITION_KVSTORE_SIZE_KB              16

// <o> User Custom Flash Partition Size (KB)
// <i> Note that partition size should be multiple of 4KB!
#define CONFIG_FLASH_PARTITION_USER_CUSTOM_SIZE_KB          12

// <q> App Has Image Header
// <i> This indicates that App has an image header, and then bootloader would make an extra offset when jumping to App.
#define CONFIG_APP_USE_IMAGE_HEADER                         1

// Bootloader Partition
#define CONFIG_FLASH_PARTITION_BOOTLOADER_ADDR              0x00000
#define CONFIG_FLASH_PARTITION_BOOTLOADER_SIZE              (CONFIG_FLASH_PARTITION_BOOTLOADER_SIZE_KB * 1024)

// App Partition
#define CONFIG_FLASH_PARTITION_APP_ADDR                     (CONFIG_FLASH_PARTITION_BOOTLOADER_ADDR + CONFIG_FLASH_PARTITION_BOOTLOADER_SIZE)
#define CONFIG_FLASH_PARTITION_APP_SIZE                     (CONFIG_FLASH_PARTITION_APP_SIZE_KB * 1024)

// App Backup Flash Partition
#define CONFIG_FLASH_PARTITION_APP_BACKUP_ADDR              (CONFIG_FLASH_PARTITION_APP_ADDR + CONFIG_FLASH_PARTITION_APP_SIZE)
#define CONFIG_FLASH_PARTITION_APP_BACKUP_SIZE              (CONFIG_FLASH_PARTITION_APP_BACKUP_SIZE_KB * 1024)

// KVStore Partition
#define CONFIG_FLASH_PARTITION_KVSTORE_ADDR                 (CONFIG_FLASH_PARTITION_APP_BACKUP_ADDR + CONFIG_FLASH_PARTITION_APP_BACKUP_SIZE)
#define CONFIG_FLASH_PARTITION_KVSTORE_SIZE                 (CONFIG_FLASH_PARTITION_KVSTORE_SIZE_KB * 1024)

// User Custom Partition
#define CONFIG_FLASH_PARTITION_USER_CUSTOM_ADDR             (CONFIG_FLASH_PARTITION_KVSTORE_ADDR + CONFIG_FLASH_PARTITION_KVSTORE_SIZE)
#define CONFIG_FLASH_PARTITION_USER_CUSTOM_SIZE             (CONFIG_FLASH_PARTITION_USER_CUSTOM_SIZE_KB * 1024)

// Check if flash partition configs value are valid
#if CONFIG_FLASH_PARTITION_BOOTLOADER_SIZE % 0x1000
#error "Bootloader Partition size should be multiple of 4KB!"
#endif
#if CONFIG_FLASH_PARTITION_APP_SIZE % 0x1000
#error "App Partition size should be multiple of 4KB!"
#endif
#if CONFIG_FLASH_PARTITION_APP_BACKUP_SIZE % 0x1000
#error "App Backup Partition size should be multiple of 4KB!"
#endif
#if CONFIG_FLASH_PARTITION_KVSTORE_SIZE % 0x2000
#error "KVStore Partition size should be multiple of 8KB!"
#endif
#if CONFIG_FLASH_PARTITION_USER_CUSTOM_SIZE % 0x1000
#error "User Custom Partition size should be multiple of 4KB!"
#endif
#if CONFIG_FLASH_PARTITION_APP_SIZE == 0
#error "App Partition size should not be 0!"
#endif
#if (CONFIG_FLASH_PARTITION_BOOTLOADER_SIZE + CONFIG_FLASH_PARTITION_APP_SIZE + CONFIG_FLASH_PARTITION_APP_BACKUP_SIZE + CONFIG_FLASH_PARTITION_KVSTORE_SIZE + CONFIG_FLASH_PARTITION_USER_CUSTOM_SIZE > CONFIG_FLASH_SIZE)
#error "The size of all flash partitions excceeds the total flash size!"
#endif

// </h> Flash Partition Config End

// </h> Bootloader Config End
//


//******************************************************************************
// <h> SoC Platform

// <o> Chip Power Mode
// <0=> LDO
// <1=> DCDC
#define CONFIG_SOC_DCDC_PAN1070                             0

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
#define CONFIG_APB1_CLOCK_DIVISOR                           0

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
#define CONFIG_APB2_CLOCK_DIVISOR                           0

// <o> 32K Low-Speed Clock Source
// <0=> RCL (32000 Hz)
// <1=> XTL (32768 Hz)
// <2=> ACT32K (32000 Hz)
// <i> Select a low-speed clock source
#define CONFIG_LOW_SPEED_CLOCK_SRC                          0

// <q> Force Calib RCL Clock
// <i> Force calibrate the 32K RCL clock at system init stage.
// <i> NOTE this only take effect when the Low-Speed Clock Source is seleted to RCL.
#define CONFIG_FORCE_CALIB_RCL_CLK                          0

// <q> Enable RAM Function
// <i> Adding essential code to SRAM could improve running performance.
#define CONFIG_RAM_FUNCTION                                 0

// <q> Enable Flash LDO
// <i> Enable the internal 1.8v flash LDO for flash power supply
// <i> instead of the default flash power from SoC VBAT.
#define CONFIG_FLASH_LDO_EN                                 0

// </h> Soc Platform End
//


//******************************************************************************
// <h> Log & Debug Config

// <e> Enable Log
// <i> Enable log print or not. After enable this:
// <i> - The basic log APIs SYS_PRINT() and APP_LOG_PRINT() with corresponding configured backend can be used.
// <i> - The SYS and APP log with levels is available for configuring to use.
#define PAN_LOG_ENABLE                                      1

// <e> Enable Boot Log
#define APP_LOG_EN                                          1

// <o> Select Log Level
// <4=> DEBUG
// <3=> INFO
// <2=> WRN
// <1=> ERR
// <0=> NONE
#define APP_LOG_LVL                                         4

// <q> Print Log Level String
// <i> Enable to print log level with [] in front of the log string.
#define APP_LOG_LVL_OUTPUT_EN                               1

// <q> Print Log Verbose String
// <i> Enable to print verbose string (includes file/line/function info) in front of the log string.
#define APP_LOG_TRACE_OUTPUT_EN                             0

// </e> APP Log Enable End

// <e> Enable SYS Log
#define PAN_SYS_LOG_ENABLE                                  1

// <o> Select Log Level
// <4=> DEBUG
// <3=> INFO
// <2=> WRN
// <1=> ERR
// <0=> NONE
#define PAN_SYS_LOG_LVL                                     3

// <q> Print Log Level String
// <i> Enable to print log level with [] in front of the log string.
#define PAN_SYS_LOG_PRINT_LEVEL_STRING                      1

// <q> Print Log Verbose String
// <i> Enable to print verbose string (includes file/line/function info) in front of the log string.
#define PAN_SYS_LOG_PRINT_VERBOSE_STRING                    0

// </e> SYS Log Enable End

// <e> Log Output to UART
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
#define CONFIG_LOG_UART_PIN                                 1

// <o> Log UART Baudrate
// <115200=> 115200
// <230400=> 230400
// <460800=> 460800
// <921600=> 921600
// <1000000=> 1M
// <2000000=> 2M
#define CONFIG_LOG_UART_BAUDRATE                            921600
// </e> Enable UART Log End

// <e> Log Output to RTT
// <i> Note that the Low Power Mode (CONFIG_PM) should be disabled
// <i> while using RTT log, since the Jlink SWD connnection would be lost
// <i> at SoC DeepSleep or Standby Mode.
#define CONFIG_RTT_LOG_ENABLE                               0

// <o> RTT Log Buffer Size (Bytes)
// <i> Configure Log RTT Up Buffer Size in Bytes (Channel 0).
#define CONFIG_LOG_RTT_UP_BUFFER_SIZE                       512
// </e> Enable RTT Log End

// <o> APP Log Backend
// <0=> printf()
// <1=> printk()
// <i> printk() can significantly reduce the cost of stack and code compared with printf().
#define APP_LOG_PRINT_MODE                                  1

// <o> SYS Log Backend
// <0=> printf()
// <1=> printk()
// <i> printk() can significantly reduce the cost of stack and code compared with printf().
#define PAN_SYS_LOG_PRINT_MODE                              1

// <o> BLE Controller Log Backend
// <0=> Disable
// <1=> vprintf()
// <2=> vprintk()
#define BLE_LL_LOG_PRINT_MODE                               2

// </e> Enable Logging System End

// <e> Enable Assert
// <i> Enable Assert or not.
// <i> - Note1: The standard C library assert() API defined in the  <assert.h> is also supported, but note that there is a little
// <i> difference b/w this PAN_ASSERT_ENABLE config is enabled or not. See soc_config.h for more information.
// <i> - Note2: Assertion with log is also supported when the logging mechanism is enabled (PAN_LOG_ENABLE == 1).
#define PAN_ASSERT_ENABLE                                   1

// <q> Enable APP Assert
// <i> Enable APP assert API APP_ASSERT() or not
#define APP_ASSERT_EN                                       1

// <q> Enable SYS Assert
// <i> Enable SYS assert API SYS_ASSERT() or not
#define PAN_SYS_ASSERT_EN                                   1
// </e> Enable Assert End

// <e> Enable IO Timing Track
#define CONFIG_IO_TIMING_TRACK                              0

// <q> (Internal) Enable BLE Controller Timing Track
// <i> This config is used to enable timing track of BLE controller internal signals and events.
// <i> Do NOT enable this config if you are not sure how it actually work!
// <i> - Some fixed pins are used for RF debugging: P04 / P07 / P10.
// <i> - Some configurable pins are used for BLE events, see app_track.c for current pin config.
#define CONFIG_BT_CTLR_LINK_LAYER_DEBUG                     0

// <o> DeepSleep Mode Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for DeepSleep Mode Timing Track.
#define CONFIG_TRACK_PIN_DEEPSLEEP_MODE                     0x22

// <o> Sleep Mode Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for Sleep Mode Timing Track.
#define CONFIG_TRACK_PIN_SLEEP_MODE                         0x23

// <o> LinkLayer IRQ Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for BLE-LinkLayer / 2.4G-RF IRQ Timing Track.
#define CONFIG_TRACK_PIN_LL_IRQ                             0x99

// <o> BLE Event IRQ Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for BLE Event IRQ Timing Track.
// <i> Currently this IRQ is borrowed from an unused peripheral (e.g. ADC).
#define CONFIG_TRACK_PIN_BLE_EVNT_IRQ                       0x99

// <o> OS Tick IRQ Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for OS Tick IRQ Timing Track.
#define CONFIG_TRACK_PIN_OS_TICK_IRQ                        0x99

// <o> SleepTimer IRQ Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for SleepTimer IRQ Timing Track.
#define CONFIG_TRACK_PIN_SLPTMR_IRQ                         0x99

// <o> Hardfault IRQ Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for Hardfault IRQ Timing Track.
#define CONFIG_TRACK_PIN_HARDFAULT_IRQ                      0x99

// <o> HAL DMA IRQ Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for DMA IRQ Timing Track.
#define CONFIG_TRACK_PIN_DMA_IRQ                            0x99

// <o> HAL GPIO P0 IRQ Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for GPIO Port0 IRQ Timing Track.
#define CONFIG_TRACK_PIN_GPIO0_IRQ                          0x99

// <o> HAL GPIO P1 IRQ Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for GPIO Port1 IRQ Timing Track.
#define CONFIG_TRACK_PIN_GPIO1_IRQ                          0x99

// <o> HAL GPIO P2 IRQ Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for GPIO Port2 IRQ Timing Track.
#define CONFIG_TRACK_PIN_GPIO2_IRQ                          0x99

// <o> HAL GPIO P3 IRQ Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for GPIO Port3 IRQ Timing Track.
#define CONFIG_TRACK_PIN_GPIO3_IRQ                          0x99

// <o> HAL I2C IRQ Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for I2C IRQ Timing Track.
#define CONFIG_TRACK_PIN_I2C_IRQ                            0x99

// <o> HAL SPI0 IRQ Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for SPI0 IRQ Timing Track.
#define CONFIG_TRACK_PIN_SPI0_IRQ                           0x99

// <o> HAL SPI1 IRQ Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for SPI1 IRQ Timing Track.
#define CONFIG_TRACK_PIN_SPI1_IRQ                           0x99

// <o> HAL TMR0 IRQ Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for TMR0 IRQ Timing Track.
#define CONFIG_TRACK_PIN_TMR0_IRQ                           0x99

// <o> HAL TMR1 IRQ Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for TMR1 IRQ Timing Track.
#define CONFIG_TRACK_PIN_TMR1_IRQ                           0x99

// <o> HAL TMR2 IRQ Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for TMR2 IRQ Timing Track.
#define CONFIG_TRACK_PIN_TMR2_IRQ                           0x99

// <o> HAL UART0 IRQ Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for UART0 IRQ Timing Track.
#define CONFIG_TRACK_PIN_UART0_IRQ                          0x99

// <o> HAL UART1 IRQ Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for UART1 IRQ Timing Track.
#define CONFIG_TRACK_PIN_UART1_IRQ                          0x99

// <o> HAL WDT IRQ Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for WDT IRQ Timing Track.
#define CONFIG_TRACK_PIN_WDT_IRQ                            0x99

// <o> HAL WWDT IRQ Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for WWDT IRQ Timing Track.
#define CONFIG_TRACK_PIN_WWDT_IRQ                           0x99

// <o> User App Channel 0 Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for User App Timing Track Channel 0.
#define CONFIG_TRACK_USER_APP_CHN0                          0x99

// <o> User App Channel 1 Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for User App Timing Track Channel 1.
#define CONFIG_TRACK_USER_APP_CHN1                          0x99

// <o> User App Channel 2 Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for User App Timing Track Channel 2.
#define CONFIG_TRACK_USER_APP_CHN2                          0x99

// <o> User App Channel 3 Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for User App Timing Track Channel 3.
#define CONFIG_TRACK_USER_APP_CHN3                          0x99

// <o> User App Channel 4 Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for User App Timing Track Channel 4.
#define CONFIG_TRACK_USER_APP_CHN4                          0x99

// <o> User App Channel 5 Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for User App Timing Track Channel 5.
#define CONFIG_TRACK_USER_APP_CHN5                          0x99

// <o> User App Channel 6 Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for User App Timing Track Channel 6.
#define CONFIG_TRACK_USER_APP_CHN6                          0x99

// <o> User App Channel 7 Track Pin
// <0x99=> None
// <0x00=> P00 (SWD_CLK)
// <0x01=> P01 (SWD_DAT)
// <0x02=> P02
// <0x03=> P03
// <0x04=> P04
// <0x05=> P05
// <0x06=> P06
// <0x07=> P07
// <0x10=> P10
// <0x11=> P11
// <0x12=> P12
// <0x13=> P13
// <0x14=> P14
// <0x15=> P15
// <0x16=> P16
// <0x17=> P17
// <0x20=> P20 (XTL1)
// <0x21=> P21 (XTL0)
// <0x22=> P22
// <0x23=> P23
// <0x24=> P24
// <0x25=> P25
// <0x26=> P26
// <0x27=> P27
// <0x30=> P30
// <0x31=> P31
// <i> Select a GPIO pin for User App Timing Track Channel 7.
#define CONFIG_TRACK_USER_APP_CHN7                          0x99
// </e> Enable IO Timing Track End

// </h> Log Config End
//


//*** <<< end of configuration section >>>    ***

#endif /* SDK_CONFIG_H */
