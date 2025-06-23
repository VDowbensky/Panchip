#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H

//*** <<< Use Configuration Wizard in Context Menu >>> ***

//******************************************************************************
// <h> Application Config



// </h> Application Config End
//


//******************************************************************************
// <h> Platform Config

// <o> Chip Power Mode
// <0=> LDO
// <1=> DCDC
#define CONFIG_SOC_DCDC_PAN1070                             1

// <o> System Clock
// <48=> 48 MHz (DPLL)
// <32=> 32 MHz (DPLL)
// <i> System main frequency, Unit MHz
#define CONFIG_SYSTEM_CLOCK                                 32

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
// <2=> ACT32K (32000 Hz)
// <i> Select a low-speed clock source
#define CONFIG_LOW_SPEED_CLOCK_SRC                          0

// <q> Force Calib RCL Clock
// <i> Force calibrate the 32K RCL clock at system init stage.
// <i> NOTE this only take effect when the Low-Speed Clock Source is seleted to RCL.
#define CONFIG_FORCE_CALIB_RCL_CLK                          1

// <q> Enable RAM Function
// <i> Adding essential code to SRAM could improve running performance.
#define CONFIG_RAM_FUNCTION                                 1

// <q> Enable Flash LDO
// <i> Enable the internal 1.8v flash LDO for flash power supply
// <i> instead of the default flash power from SoC VBAT.
#define CONFIG_FLASH_LDO_EN                                 1

// <q> Remap Vector Table to SRAM
#define CONFIG_VECTOR_REMAP_TO_RAM                          1

// <e> Enable Auto Power Optimization
// <i> Several power configurations could be updated due to temperature change.
#define CONFIG_AUTO_OPTIMIZE_POWER_PARAM                    0

// <o> Temperature Sample Interval (in Seconds)
#define CONFIG_TEMP_SAMPLE_INTERVAL_S                       300

// <q> Enable DVDD Voltage Optimization
#define CONFIG_DVDD_VOL_OPTIMIZE_EN                         0
// </e>

// </h> Soc Platform End
//


//******************************************************************************
// <e> LowPower Enable
#define CONFIG_PM                                           1

// <q> Enable System Watchdog
#define CONFIG_SYSTEM_WATCH_DOG_ENABLE                      0

// <q> Keep Flash Power in Low Power Mode
// <i> Select this means flash power would be retained in Low Power Mode, and
// <i> there would be a little avg-current increase (about 1uA). The benefit is that
// <i> the large peak current (>15mA) would not occur.
#define CONFIG_KEEP_FLASH_POWER_IN_LP_MODE                  1

// <q> Enable DeepSleep Mode 2
// <i> Enable DeepSleep Mode 2 (Only LPLDOH in use), and the HW APB Timer Wakeup
// <i> and PWM waveform output can be use in this mode.
#define CONFIG_DEEPSLEEP_MODE_2                             0

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

// <q> Continue Run After Standby M1 Wakeup
// <i> Check this configuration to let CPU continue run after standby M1 waking up,
// <i> or CPU would reset after waking up from standby M1.
#define CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET           0

// <q> Enable AHB Clock Optimization
#define CONFIG_HCLK_OPTIMIZE_EN                             0

// </e> Low Power Enable End
//


//******************************************************************************
// <e> RTOS Enable
#define CONFIG_OS_EN                                        1

// <o> The Maximun Numble of OS Task
#define configMAX_PRIORITIES                                8

// <o> OS Total Heap Size(in byte)
#define configTOTAL_HEAP_SIZE                               5400

// <o> OS Main Thread Stack Size(in word)
#define CONFIG_MAIN_TASK_STACK_SIZE                         256

// <o> OS Main Thread Priority
#define CONFIG_MAIN_TASK_PRIO                               3

// <o> OS BLE Host Thread Stack Size(in word)
#define CONFIG_BLE_HOST_THREAD_STACK_SIZE                   256

// <o> OS BLE Host Thread Priority
#define CONFIG_BLE_HOST_THREAD_PRIO                         6

// <q> OS Timer Task Enable
#define configUSE_TIMERS                                    1

// <o> OS Timer Task Stack Size(in word)
#define configTIMER_TASK_STACK_DEPTH                        128

// <o> OS Timer Task Priority
#define configTIMER_TASK_PRIORITY                           2

// <o> The Maximun Number of OS Timer Queue Length
#define configTIMER_QUEUE_LENGTH                            12

// <q> Enable OS Idle Hook
#define configUSE_IDLE_HOOK                                 0

// <q> Enable OS Tick Hook
#define configUSE_TICK_HOOK                                 0

// <q> Enable OS Malloc Fail Hook
#define configUSE_MALLOC_FAILED_HOOK                        1

// <o> Enable OS Stack OverFlow Hook
// <0=>Disable <1=>Mode1 <2=>Mode2
#define configCHECK_FOR_STACK_OVERFLOW                      0

// <q> OS Log: Print Current Heap Usage
#define CONFIG_FREERTOS_HEAP_PRINT                          0

// </e> OS Enable End
//


//******************************************************************************
// <e> BLE Enable
#define BLE_EN                                              0

// <h> BLE Resource Config

// <q> Use Chip unique Mac Address
#define CONFIG_USER_CHIP_MAC_ADDR                           1

// <o> RF TX power
// <0=> 0dBm
// <1=> 1dBm
// <2=> 2dBm
// <3=> 3dBm
// <4=> 4dBm
// <5=> 5dBm
// <6=> 6dBm
// <7=> 7dBm
// <8=> 8dBm
// <9=> 9dBm
#define CONFIG_BT_CTLR_TX_POWER_DFT                         0

// <o> Support Maximun Number of BLE Master Link
#define CONFIG_BT_MAX_NUM_OF_CENTRAL                        0

// <o> Support Maximun Number of BLE Slave Link
#define CONFIG_BT_MAX_NUM_OF_PERIPHERAL                     1

// <q> Support GAP Broadcaster Role
#define MYNEWT_VAL_BLE_ROLE_BROADCASTER                     1

// <q> Support GAP Central Role
#define MYNEWT_VAL_BLE_ROLE_CENTRAL                         0

// <q> Support GAP Observser Role
#define MYNEWT_VAL_BLE_ROLE_OBSERVER                        0

// <q> Support GAP Peripheral Role
#define MYNEWT_VAL_BLE_ROLE_PERIPHERAL                      1

// <o> BLE Host Max ATT MTU Size
#define MYNEWT_VAL_BLE_ATT_PREFERRED_MTU                   247

// <o> BLE Host HCI Rx ACL buffer counts
#define MYNEWT_VAL_BLE_TRANSPORT_ACL_FROM_LL_COUNT          5

// <o> BLE Host HCI Rx ACL buffer size
#define MYNEWT_VAL_BLE_TRANSPORT_ACL_SIZE                   27

// <o> BLE Host HCI events counts
#define MYNEWT_VAL_BLE_TRANSPORT_EVT_COUNT                  4

// <o> BLE Host HCI discardable events counts
#define MYNEWT_VAL_BLE_TRANSPORT_EVT_DISCARDABLE_COUNT      0

// <o> BLE Host L2CAP buffer counts
#define MYNEWT_VAL_MSYS_1_BLOCK_COUNT                       4

// <o> BLE Host L2CAP buffer size
#define MYNEWT_VAL_MSYS_1_BLOCK_SIZE                        120

// <o> BLE Controller RF RX Buffer Number (must be a power of 2)
#define CONFIG_BLE_CONTROLLER_RF_RX_BUF_NUM                 16

// <o> BLE Controller RF TX Buffer Number (must be a power of 2)
#define CONFIG_BLE_CONTROLLER_RF_TX_BUF_NUM                 16

// <o> BLE Controller Packet Encrypt Time (unit:us)
#define CONFIG_BLE_CONTROLLER_LL_ENC_TIME                   100

// <o> BLE Controller More Data Number
#define CONFIG_BLE_CONTROLLER_MORE_DATA_NUM                 6

// <o> BLE Controller WhiteList Number
#define CONFIG_BLE_CONTROLLER_WIHTELIST_NUM                 1

// <o> BLE Controller Resolving List Number
#define CONFIG_BLE_CONTROLLER_RESOLVELIST_NUM               0

// <o> BLE Controller Master Link Margin (unit:0.625ms)
#define CONFIG_BLE_CONTROLLER_MASTER_LINK_MARGIN            6

// <o> BLE LL IRQ Priority
// <0=>Highest <1=>High <2=>Low <3=>Lowest
#define CONFIG_BLE_LL_IRQ_PRIO                              0

// <o> BLE Event Handler IRQ Priority
// <0=>Highest <1=>High <2=>Low <3=>Lowest
#define CONFIG_BLE_EVT_HANDLER_IRQ_PRIO                     1

// </h> BLE Resource End
//

// <h> BLE Security Manager

// <o> Select Security Level
#define MYNEWT_VAL_BLE_SM_SC_LVL                            2

// <q> Enable SM Legacy Pair
#define MYNEWT_VAL_BLE_SM_LEGACY                            0

// <q> Enable SM Security Pair
#define MYNEWT_VAL_BLE_SM_SC                                0

// <o> Select IO Capability
// <0=> DisplayOnly
// <1=> DisplayYesN
// <2=> KeyboardOnly
// <3=> NoInputNoOutput
// <4=> KeyboardDisplay Only
#define CONFIG_HS_IO_CAPABILITY                             3

// <q> Enable SM Bonding
#define MYNEWT_VAL_BLE_SM_BONDING                           0

// <q> Enable SM MITM
#define MYNEWT_VAL_BLE_SM_MITM                              0

// <q> Enable SM OOB
#define MYNEWT_VAL_BLE_SM_OOB_DATA_FLAG                     0

// <o> Set Local Distribute Key
// <0=> None
// <1=> LTK
// <3=> LTK and IRK
// <7=> LTK and IRK and CSRK
#define MYNEWT_VAL_BLE_SM_OUR_KEY_DIST                      1

// <o> Set Peer Distribute Key
// <0=> None
// <1=> LTK
// <3=> LTK and IRK
// <7=> LTK and IRK and CSRK
#define MYNEWT_VAL_BLE_SM_THEIR_KEY_DIST                    1

// <q> Enable SM Info Persist Store
#define MYNEWT_VAL_BLE_STORE_CONFIG_PERSIST                 0

// <o> Set Maximun Store Bonded Devices Number
#define MYNEWT_VAL_BLE_STORE_MAX_BONDS                      1

// <o> Set Maximun Store Bonded Device CCCD Number
#define MYNEWT_VAL_BLE_STORE_MAX_CCCDS                      8

// <q> Enable BLE Host RPA Resovling Function
#define MYNEWT_VAL_HOST_SOFTWARE_RPA                        0

// </h> BLE Security Manager End
//

// <h> BLE Services Config

// </h> BLE Services Config End
//

// </e> BLE Enable End
//

//******************************************************************************
// <h> Flash/Image Config

// <o> Chip Flash Size
// <0x7F000=>  508 (KB)
// <0x3F000=>  252 (KB)
// <i> You can select chip flash size in the pull-down list, but modify the list only when you know what you are doing!
#define CONFIG_FLASH_SIZE                                   0x7F000

// <h> Flash Partition Config

// <e> Enable Bootloader
// <i> App Image header should only be enabled when there is a bootloader to boot App Image.
#define CONFIG_ENABLE_BOOTLOADER                            0

// <o> Bootloader Flash Partition Size (KB)
// <i> Note that partition size should be multiple of 4KB!
#define CONFIG_FLASH_PARTITION_BOOTLOADER_SIZE_KB           40

// <e> Enable App Image Header
// <i> App image header should be enabled in case to support BLE OTA.
// <i> Once this config enabled, a "signed" image is generated after build, for later DFU/OTA use.
#define CONFIG_APP_USE_IMAGE_HEADER                         1

// <o> App Image Version - Major <0-255>
#define CONFIG_APP_IMG_VER_MAJOR                            0

// <o> App Image Version - Minor <0-255>
#define CONFIG_APP_IMG_VER_MINOR                            0

// <o> App Image Version - Revision <0-65535>
#define CONFIG_APP_IMG_VER_REVISION                         1

// <o> App Image Version - Build Num  <0x0-0xFFFFFFFF>
#define CONFIG_APP_IMG_VER_BUILD                            0
// </e> Enable App Image Header End

// </e> Enable Bootloader End

// <o> App Flash Partition Size (KB)
// <i> Note that partition size should be multiple of 4KB!
#define CONFIG_FLASH_PARTITION_APP_SIZE_KB                  480

// <o> App Backup Flash Partition Size (KB)
// <i> Note that partition size should be multiple of 4KB!
#define CONFIG_FLASH_PARTITION_APP_BACKUP_SIZE_KB           0

// <o> KVStore Flash Partition Size (KB)
// <i> Note that partition size should be multiple of 8KB!
#define CONFIG_FLASH_PARTITION_KVSTORE_SIZE_KB              16

// <o> User Custom Flash Partition Size (KB)
// <i> Note that partition size should be multiple of 4KB!
#define CONFIG_FLASH_PARTITION_USER_CUSTOM_SIZE_KB          12

//////////////////////////// Final Flash Map /////////////////////////////////
// Bootloader Partition
#define CONFIG_FLASH_PARTITION_BOOTLOADER_ADDR              0x0
#if CONFIG_ENABLE_BOOTLOADER
#define CONFIG_FLASH_PARTITION_BOOTLOADER_SIZE              (CONFIG_FLASH_PARTITION_BOOTLOADER_SIZE_KB * 1024)
#else
#define CONFIG_FLASH_PARTITION_BOOTLOADER_SIZE              0
#endif

// App Partition
#if CONFIG_ENABLE_BOOTLOADER
#define CONFIG_FLASH_PARTITION_APP_ADDR                     (CONFIG_FLASH_PARTITION_BOOTLOADER_ADDR + CONFIG_FLASH_PARTITION_BOOTLOADER_SIZE)
#else
#define CONFIG_FLASH_PARTITION_APP_ADDR                     0x0
#endif
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

// <e> Enable Firmware Encryption
// <i> Generate AES encrypted firmware at build stage for SoC with secure enabled.
// <i> Should be consistent with encrypt_info.yaml
#define CONFIG_FIRMWARE_ENCRYPTION                          0

#if CONFIG_FIRMWARE_ENCRYPTION
// <o> Encrypt Flash Offset
// <i> Flash offset index to indicate which page should be encrypted.
// <i> Should be consistent with encrypt_info.yaml file!
#define CONFIG_ENCRYPT_FLASH_OFFSET                         0x001
#endif /* CONFIG_FIRMWARE_ENCRYPTION */

// </e>

// </h> Flash/Image Config End
//

//******************************************************************************
// <h> Log/Debug Config

// <e> Enable Log
// <i> Enable log print or not. After enable this:
// <i> - The basic log APIs SYS_PRINT() and APP_LOG_PRINT() with corresponding configured backend can be used.
// <i> - The SYS and APP log with levels is available for configuring to use.
#define PAN_LOG_ENABLE                                      1

// <e> Enable APP Log
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
#define CONFIG_LOG_UART_PIN                                 2

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

// <q> Enable Startup Long Delay
// <i> Add a long delay at system startup stage for debugging purpose.
// <i> e.g. This can make jlink programing easier when low-power mode enabled.
#define CONFIG_STARTUP_LONG_DELAY                           0

// </h> Log/Debug Config End
//


//*** <<< end of configuration section >>>    ***

#endif /* SDK_CONFIG_H */
