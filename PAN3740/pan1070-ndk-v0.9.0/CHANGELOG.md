# 更新日志

## **PAN10XX NDK v0.9.0**

PAN10XX Nimble DK `v0.9.0` *(2025-05-01)* 已发布：

### 1. SDK

#### nimble

- SDK Config 框架：
  - 优化 SDK Config 顶层菜单显示，使结构更清晰易懂
  - 新增 Config **BLE Enable** (`BLE_EN`)，将其从 Keil Target C/C++ Options - Proprocessor Symbols 中解耦
  - 优化 Flash/Image Config - Flash Partition Config 配置，精简各个 Flash 分区配置项，移除了分区起始地址配置，使其更加简单易用
  - 优化 Log/Debug Config 相关配置
    - 新增 Config **Enable Log** (`PAN_LOG_ENABLE`) 总开关
    - 新增 Config **Enable SYS Log** (`PAN_SYS_LOG_ENABLE`) 及对应配置子项，用于使能 SDK 底层 Log（以区分 App 层 Log）
    - 新增 SYS/App Log 的 Backend 选择，新增 `printk()` 实现并将其作为默认选项，以减少 Log 接口的代码和 Stack 占用；若有浮点数打印需求，则可手动切换至 `printf()`
    - 新增 Config **Enable Assert** (`PAN_ASSERT_ENABLE`) 及对应配置子项，用于使能 SDK 底层和 App 上层的 Assert 断言功能
- Component 组件：
  - 新增 **SFUD** 组件：通用 SPI Flash 驱动（Serial Flash Universal Driver），内置支持多种 SPI Flash 型号，且支持型号扩展
  - 新增 **mcumgr** 组件：MCU Manager 框架，用于支持 BLE OTA（之前 SDK 版本已存在，本次版本修复了一些问题，并从 Nimble/Host 层解耦至 Component 组件层）
  - 新增 **CherryUSB** 组件：CherryUSB 框架，支持常见 USB Device Class（CDC、HID、MSC 等）
  - 更新 **App Timer** 组件，修复了使用过程中功耗变高的问题
  - 更新 **App Log** 组件，优化了一些细节，提升使用体验
  - 更新 **KVStore** 组件，新增接口 `app_kv_key_exists()` 用于获取特定 Key 是否存在，并提升了 Flash Read 接口效率
  - 更新所有组件代码中的 Log 接口，统一使用本次新增的 SYS LOG 相关接口
- SoC 平台：
  - 更新 `soc_busy_wait()` API 接口定义，将其强制编译成 RAM Code 以确保延时准确
  - 更新 系统初始化 Log，根据当前 Flash 分区配置，绘制当前工程的 Flash Map 图并打印出来
  - 新增 `irq_lock()` 与 `irq_unlock()` 接口，用于 OS 无关的开关中断场景（需成对使用，支持嵌套）
  - 更新 Log 打印接口，统一使用本次新增的 SYS LOG 相关接口
- OS：
  - 修复 使能低功耗情况下，故意长时间不进入低功耗有概率引起空闲任务 Assert 的问题
  - 优化 DeepSleep 低功耗相关流程，并修复了一些问题
  - 优化 `pvPortMalloc()`、`vApplicationStackOverflowHook()` 与 `vApplicationMallocFailedHook()` 等函数，使用新增的 Simple Print 相关接口，以最大限度精简因为 Log 打印而占用的 Task Stack
- BLE Host：
  - 修复 SM Security Request 与 Pairing Request 无法共存的问题
  - 优化 Service，将 GAP Service 作为默认 Service
  - 新增 SM Common API
  - 新增 BMS Profile
  - 新增 CGM Profile（暂未公开，详请联系Panchip）
- Bootloader：
  - 新增 App Image Header 校验，若校验未通过则拒绝跳转至 App
  - 修复 2.4G PRF OTA 流程的一些问题
  - 修复 UART Xmodem DFU 流程的一些问题
  - 更新 USB DFU 代码实现，默认使用 Panchip USB DFU 协议（以代替之前版本的 ROM USB DFU 协议），以方便后续修改
  - 优化 SDK Config 配置，并新增 Config **USB DFU Mode** (`BOOT_USB_DFU_MODE`)，用于配置与上位机工具 Panchip DFU Tool 的交互流程
  - 新增 PAN101x Keil 工程文件
- Scripts：
  - 更新 `signed_image.py` 脚本，修复一些潜在的风险
  - 新增 `flash_ram_usage.py` 脚本，用于在 Keil After Build 阶段打印当前工程的 Flash 和 SRAM 使用情况
  - 新增 `dfu_porting_tool.py` 脚本，用于快速在 App 工程中增添 BLE OTA 支持，详见文档中心 *开发指南 - NDK 蓝牙 OTA 移植指南* 文档中的相关介绍

#### Panchip HAL

- Panchip Spark BLE Controller Library：
  - PAN101x:
    - 修复 RF Rx 的抗频偏问题
    - 新增 DTM API（not SVC）
    - 调整 SVC DTM API
    - 优化 ANT 时序
    - 新增 `pan_misc_register_adv_evt_closed_cb()` API
    - 调整 Max RX ACL data size 为 251B
  - PAN107x:
    - 修复 RF Rx 的抗频偏问题
    - 新增 DTM API（not SVC）
    - 调整 SVC DTM API
    - 优化 ANT 时序
    - 优化 SRAM 占用，减少约 3.3KB
    - 新增 `pan_misc_register_adv_evt_closed_cb()` API
    - 新增 `pan_misc_register_rf_debug_signal()` API
    - 新增 RF SEQ Refresh API

- Panchip PRF (2.4G Private RF) Library:
  - 新增 适配外部 PA 芯片的接口
  - 优化 RF Rx 流程，默认使能 Timeout 功能

- BSP:
  - 新增 **printk** 相关文件与接口
  - 新增 **Simple Print** 相关文件与接口
  - 重构 **SYS Log** 相关接口，用于适配 SDK 新增的 SYS Log 功能
  - 新增 编译期 Assert 接口 `BUILD_ASSERT()`
  - 优化 `CLK_ResetChip()` 与 `CLK_ResetSystemToRomMode()` 接口实现，简化使用方法
  - 优化 `PW_AutoOptimizeParams()` 流程，修复一些潜在的芯片兼容性问题
  - 优化 `SystemInit()` 流程，修复 DCDC 使能后，在特定场景下芯片动态功耗偏高的问题
  - 优化 `SystemHwParamLoader()` 流程，修复与旧版本芯片（FT Version < 9）的兼容性问题
  - 修复 一些注释错误的问题

#### Samples

- bluetooth:
  - 更新 **ble_cental** 例程，优化 BLE 连接与断开事件处理流程
  - 更新 **ble_cental_periph** 例程，新增 BLE_GAP_EVENT_REPEAT_PAIRING 事件处理
  - 更新 **ble_distance** 例程，新增 BLE_GAP_EVENT_REPEAT_PAIRING 事件处理
  - 更新 **ble_periph_hr** 例程，新增 BLE_GAP_EVENT_REPEAT_PAIRING 事件处理，新增 `CONFIG_PA_ENABLE` 演示某些场景下需控制外部 PA 芯片的流程
  - 更新 **ble_periph_hr_ota** 例程，新增 BLE_GAP_EVENT_REPEAT_PAIRING 事件处理，并使用更新后的 OTA 框架（mcumgr 组件）
  - 更新 **ble_throughput** 例程，新增 BLE_GAP_EVENT_REPEAT_PAIRING 事件处理，新增 `CONFIG_PA_ENABLE` 演示某些场景下需控制外部 PA 芯片的流程
  - 新增 **ble_hci** 例程，演示通过 BLE HCI UART 接口与外部 BLE Host 通信
- component（新增）：
  - 新增 **flash_kvstore** 例程，演示 KVStore 组件的基本功能与使用方法
  - 新增 **sfud** 例程，演示通过 SFUD 组件操作 EVB 底板外部 SPI Flash 的方法
  - 新增 **usbd_cdc_acm** 例程，演示 CherryUSB 组件的 CDC-ACM Device 虚拟串口设备功能
  - 新增 **usbd_cdc_acm_msc_ram_disk** 例程，演示 CherryUSB 组件的 CDC-ACM 虚拟串口和 MSC 大容量存储复合设备功能
  - 新增 **usbd_hid_custom_inout** 例程，演示 CherryUSB 组件的 HID Custom Device 自定义 HID 设备功能
  - 新增 **usbd_hid_custom_msc_ram_disk** 例程，演示 CherryUSB 组件的 HID Custom 自定义 HID 通信与 MSC 大容量存储复合设备功能
  - 新增 **usbd_hid_keyboard** 例程，演示 CherryUSB 组件的 HID Keyboard Device 键盘设备功能
  - 新增 **usbd_hid_mouse** 例程，演示 CherryUSB 组件的 HID Mouse Device 鼠标设备功能
  - 新增 **usbd_msc_flash_disk** 例程，演示 CherryUSB 组件的 MSC Device 大容量存储设备功能，使用 SoC Flash 或外部 SPI Flash 作为存储介质
  - 新增 **usbd_msc_ram_disk** 例程，演示 CherryUSB 组件的 MSC Device 大容量存储设备功能，使用 SoC SRAM 作为存储介质
- solutions:
  - 更新 **rf_mini_test** 例程，支持设置单载波功率 Tx Power 的 Config 配置
  - 更新 **ble_prf_sample** 例程，增加蓝牙和 2.4G Rx 模式
  - 新增 **ble_prf_multi_mode_toggle** 例程，演示通过按键动态切换 BLE 与 2.4G 模式的方法
  - 新增 **ble_peripheral_cgm** 例程，演示新增的 CGM Profile 相关功能（暂未公开，详请联系Panchip）
- solutions_hid（暂未公开，详请联系Panchip）：
  - 新增 **bootloader_hid** 例程，仅支持 USB DFU，可通过 Flash Flag 触发进入
  - 新增 **mult_ms** 例程，PAN107x 三模实体鼠标，可配对 PAN2628 Dongle
  - 新增 **mult_ms_evb** 例程，PAN107x HID Demo 的 EVB 版本，支持三模切换
  - 新增 **prf_dongle** 例程，USB 1K 2.4G Dongle，目前仅支持 PAN107x（后续版本支持 PAN101x）

### 2. HDK

- 新增 PAN1070UA2A 核心板图纸、设计源文件、生产文件
- 更新 PAN1070UA1A 核心板图纸、设计源文件、生产文件
- 更新 PAN1070UAEC 核心板图纸、设计源文件、生产文件
- 更新 PAN1010M9BA 核心板图纸、设计源文件、生产文件
- 更新 PAN1010S9FA 核心板图纸、设计源文件、生产文件
- 更新 PAN3740U33BBA 核心板图纸、设计源文件、生产文件

### 3. MCU

- 所有例程：
  - 优化 校准信息载入流程，默认使用更加保守的电压配置策略
  - 优化 Log 接口，使用新的 SYS Log 接口替换掉：
    - 旧的 SYS Log 接口
    - 直接的 `printf()` 调用
    - 直接的 `sprintf()` 调用
- 更新 **DMA** 例程，修复一些问题

### 4. DOC

- 更新 **NDK 快速入门指南** 文档，新增 PAN1070UA2A EVB 核心板简介
- 移除 **Nimble 简介** 文档
- 更新 **PAN10xx 硬件参考设计** 文档，新增 PAN1070UA2A 硬件参考设计原理图，更新板载天线参考设计，提供 2 种天线匹配
- 更新 **NDK 蓝牙开发指南** 文档，新增使能额外的 IO 控制外部 PA 芯片的方法
- 更新 **NDK Configuration 配置指南** 文档，介绍更新后的 SDK Config 配置项（`sdk_config.h`）
- 更新 **NDK Bootloader 开发指南** 文档，介绍重构后的 Bootloader 工程
- 更新 **NDK PRF 2.4G 开发指南** 文档，新增使能额外的 IO 控制外部 PA 芯片的方法
- 更新 **NDK 常见问题（FAQs）** 文档，补充更多应对 SWD 难以烧录问题的方法
- 更新 **量产烧录** 文档，增加对于裸片烧录的接线提醒和建议
- 更新 **RF Test** 文档，更新 PAN107x ToolBox 的下载链接
- 更新 **BLE Peripheral OTA** 例程文档，新增了 Image 烧录注意事项，并移除了 OTA 功能移植小节（OTA 功能移植介绍请参考**开发指南 - NDK 蓝牙 OTA 移植指南**文档）
- 更新 **Standby Mode1 GPIO Key Wakeup** 例程文档，新增了 `soc_enter_standby_mode_1()` 接口对 SRAM 保电配置的介绍，并增加了 GPIO 模块在 StandbyM1 唤醒后不会复位的相关注意事项介绍
- 更新 **GPIO Input With Interrupt** 例程文档，新增 *开发者说明* 小节，增加 GPIO 模块框图及 GPIO HAL Driver 初始化配置说明
- 更新 **GPIO Input Polling** 例程文档，新增 *开发者说明* 小节，增加 GPIO 模块框图及 GPIO HAL Driver 初始化配置说明
- 更新 **GPIO Open-Drain Output** 例程文档，新增 *开发者说明* 小节，增加 GPIO 模块框图及 GPIO HAL Driver 初始化配置说明
- 更新 **GPIO Push-Pull Output** 例程文档，新增 *开发者说明* 小节，增加 GPIO 模块框图及 GPIO HAL Driver 初始化配置说明
- 新增 **NDK BLE 协议栈简介** 文档，简要介绍 NDK BLE 协议栈及支持的特性
- 新增 **NDK 蓝牙 OTA 移植指南** 文档，介绍在普通蓝牙工程中增加 OTA 支持的方法
- 新增 **PAN10xx ROM DFU 终端工具** 文档
- 新增 **Panchip DFU 工具** 文档
- 新增 **Panchip 工具目录工具** 文档
- 新增 **PAN107x 低功耗蓝牙功耗分析工具** 文档
- 新增 **BLE HCI** 例程文档
- 新增 **Flash KVStore** 例程文档
- 新增 **Serial Flash Universal Driver** 例程文档
- 新增 **USBD CDC-ACM** 例程文档
- 新增 **USBD CDC-ACM and MSC RAM Disk** 例程文档
- 新增 **USBD HID Custom In/Out** 例程文档
- 新增 **USBD HID Custom In/Out and MSC RAM Disk** 例程文档
- 新增 **USBD HID Keyboard** 例程文档
- 新增 **USBD HID Mouse** 例程文档
- 新增 **USBD MSC Flash Disk** 例程文档
- 新增 **GPIO Input With Interrupt** 例程文档
- 新增 **BLE PRF Multi Mode Toggle** 例程文档
- 新增 **solutions_hid** 相关例程文档
- 更新 BQB Test Report PDF 文档

### 5. TOOLS

- 更新 量产烧录工具 `PAN10xx Download Tool` 至 `v0.0.015` 版本：
  - 添加芯片版本信息校验功能
  - 优化上位机部分功能
- 更新 工具箱工具 `Panchip ToolBox` 至 `v0.0.010` 版本：
  - ​修复 RF 测试界面 2.4G 接收测试问题
  - 修复 RF 测试界面 USB 设备连接检测异常断开问题
  - 添加 RF 测试发射等待成功之后的具体等待时间显示
  - 针对 PAN107x/PAN101x 的 DFU 界面，添加下载终端版本 ROM DFU 工具接口
  - 添加 PAN107x/PAN101x 引出脚界面，添加支持芯片型号 PAN1070UA1A、PAN1070GAEC
  - 优化 引出脚界面绘制非对称引脚芯片的显示
  - 更新 帮助菜单，添加版本日志显示功能
  - 修复 引出脚界面优化之后的问题
  - 新增 对 Win7 系统的支持，生成兼容 Win7 系统运行的可执行文件
- 更新 OTA 工具 `Panchip 2.4G OTA` 至 `v0.0.008` 版本：
  - 优化 菜单栏帮助菜单，添加 OTA Dongle 固件程序下载链接
  - 添加 帮助菜单版本日志显示，并支持下载历史版本功能
  - 修改 发射功率范围为 -10 ~ 7 dbm
  - 修复 执行设置配置与开始扫描设备问题执行无效问题
- 新增 PANCHIP 工具目录工具 `Panchip Tools Directory`，用于下载并打开 PANCHIP 支持的各种工具软件
- 新增 功耗分析工具 `Power Profiler`，用于评估 PAN107x SoC 功耗
- 新增 USB DFU 工具集 `Panchip DFU Tool`，包含 `PanchipDFUTool`、`PanchipConsoleDfu`、`Pan10xxRomDfuConsoleTool` 等工具，可分别用于不同的 USB DFU 场景
- 更新 RF 测试固件，修复 2M BQB 测试失败的问题

### 6. ISSUES

#### 遗留问题

- `BUG #873`: 兼容问题--peripheral_ota---与小米手机11配合升级，小米11安装的nRF Conenct软件版本是4.28时，无法升级
- `BUG #1081`: PAN1070 鼠标 + PAN2628 Dongle 测试发现一些功能不稳定的问题


## **PAN10XX NDK v0.8.0**

PAN10XX Nimble DK `v0.8.0` *(2025-01-20)* 已发布：

### 1. SDK

#### nimble

- SDK 框架重构：
  - 文件目录组织结构调整，使其更清晰易懂，方便查阅，并新增 `component`、`profiles`、`soc`、`utilities` 等 4 个顶层目录
  - SDK Config 配置文件优化，调整了一些平台相关的菜单等级和项目，并新增了一些常用配置
  - 例程工程优化：
    - Keil 工程目录树组织结构调整，所有工程均添加 HAL Driver 代码，同时增加更多目录分类，并按功能划分调整工程文件顺序
    - Keil 工程配置调整，尽可能将所有例程的工程配置保持一致，以方便用户移植代码
  - 将芯片 SoC 层与 OS 解耦，使得 App 可以方便地根据需要使能或禁用 OS
  - 优化系统启动流程，将 main() 函数直接暴露到 App 层：
    - 若未使能 OS，则 main() 函数可视作裸机程序的 main() 函数
    - 若使能了 OS，则 main() 函数会被视作一个 FreeRTOS Task
  - 新增 Component 组件的概念，提升 SDK 的扩展性，后续对于相对独立的功能将会通过组件的方式提供，目前已经支持的组件有：
    - BLE Device Filter
    - App Timer
    - App Log
    - App Assert
    - App FIFO
    - Ring Buffer
    - Sort Queue
    - Memory Manager
    - Key-Value Store (KVStore)
  - BLE 蓝牙开发框架优化：
    - 将 BLE 蓝牙功能与 SDK 框架解耦，BLE 不再作为 SDK 必要的组成部分，而是将其视作一个独立的模块，用户可通过 Config 和简单的代码调用自行决定增加或删除 BLE 功能
    - 优化蓝牙 App 暴露给用户的接口，简化蓝牙 App 开发复杂度
    - 优化 SDK Config 中与蓝牙相关的配置，使其更清晰易懂，配置项更加灵活
- 新增 App Log 框架（组件）
- 新增 Flash Partition 机制，用户可更加自由地规划 Flash 各个分区的地址和大小，并可以更清晰地配置 Bootloader、DFU、OTA 相关参数
- 新增 IO Timing Track 机制，用户可以方便地通过翻转 GPIO 的方式调试对时序敏感的代码（如各个外设的中断等）
- 重构 Bootloader 例程：
  - 重构 Bootloader Config 配置文件（`sdk_config.h`）
  - 重构 Bootloader 工程框架，使其支持新增的 Flash Partition 机制
  - 优化代码，调整 DFU 按键检测（使用 EVB 底板的 Key1 和 Key2）


#### Panchip HAL

- Panchip Spark BLE Controller Library：
  - PAN101x:
    - 新增 Connection Event Close 回调函数
    - 新增 User IRQ 回调函数
    - 新增 ActTimer Start Evnet Cmp Setting 保护
    - 优化 More Data 特性
    - 优化 PHY Update 逻辑
  - PAN107x:
    - 新增 Connection Event Close 回调函数
    - 新增 User IRQ 回调函数
    - 新增 ActTimer Start Evnet Cmp Setting 保护
    - 使能 Scan Duty 功能
    - 优化 More Data 特性
    - 优化 PHY Update 逻辑

- Panchip PRF (2.4G Private RF) Library:
  - 优化内部 Phy 配置接口
  - 新增对外 API 接口 `panchip_prf_phy_cfg_mode_set()`

- BSP:
  - 优化 底层 DMA Driver，新增接口：`DMAC_ContinueChannel()`、`DMAC_GetXferredBlockCount()`
  - 优化 底层 FMC Driver，新增接口：`FMC_IsBusyWorking()`
  - 优化 HAL DMA Driver，修复一些问题
  - 重构 HAL UART Driver，使其在使用上更符合一般习惯，并修复了一些问题
  - 修复 HAL I2C Driver 中的一些问题
  - 新增 HAL Driver IO Timing Track 支持

#### Samples

- **所有例程适配新的 SDK 框架**
- peripheral：
  - 修复 `i2c_master_dma_receive` 与 `i2c_slave_dma_send` 例程无法进行任意数量数据通信的问题
  - 更新 `uart_dma` 和 `uart_fifo` 例程，以适配重构后的 HAL UART Driver
- low_power:
  - 将一些使用底层 Driver 的代码替换为使用 HAL Driver
  - 修复一些细节功能问题
- solutions:
  - 新增 `mult_ms` 例程，可使用 EVB 演示多模鼠标方案
  - 移除一些废弃的例程

### 2. HDK

- 更新 PAN1070UA1A 核心板图纸、设计源文件、生产文件
- 更新 PAN1070UAEC 核心板图纸、设计源文件、生产文件
- 更新 PAN1010M9BA 核心板图纸、设计源文件、生产文件
- 更新 PAN1010S9FA 核心板图纸、设计源文件、生产文件
- 新增 PAN3740U33BBA 核心板图纸、设计源文件、生产文件

### 3. MCU

- 重构 **FMC** 例程：
  - 梳理并重构例程代码框架，重写所有测试用例，使例程更加易懂和易于移植
  - 更新例程演示文档
- 更新 **LowPower** 例程：
  - 修复一些代码问题
- 新增 **PRF_BLE_BEACON** 例程：
  - 演示 PRF 2.4G 模拟蓝牙 Beacon 功能
  - 新增例程演示文档

### 4. DOC

- 更新 `NDK 整体框架介绍` 文档，介绍重构后的 SDK 框架
- 更新 `PAN10xx 硬件参考设计` 文档，新增更多芯片封装介绍，新增调优版本的板载天线介绍
- 新增 `PAN3740 EVB 介绍与硬件参考设计` 文档，介绍 PAN3740 （Sub-1G）芯片的 EVB 及硬件参考设计
- 更新 `NDK Configuration 指南` 文档，介绍重构后的 SDK Config 配置机制，并解释各个 Config 选项的含义
- 更新 `NDK 低功耗开发指南` 文档，更新电源结构简图，并更详细地介绍了芯片支持的低功耗模式特点
- 更新 `NDK Bootloader 开发指南` 文档，介绍重构后的 Bootloader 例程
- 更新 `NDK Chip Migration` 文档，将其重命名为 `NDK PAN101x 工程移植指南`，并介绍重构后 SDK 框架下 PAN101x 与 PAN107x 工程的移植方法
- 更新 `NDK App 开发指南` 文档，将其重命名为 `NDK 蓝牙开发指南`，并介绍重构后的蓝牙 BLE 开发框架及使用方法
- 新增 `NDK PRF 2.4G开发指南` 文档，介绍 Panchip 私有 2.4G 的开发方法及注意事项
- 更新 `Panchip Toolbox 工具箱` 文档，介绍新版本 Toolbox 工具箱软件各个功能的使用方法
- 更新 蓝牙相关例程文档，基于新的蓝牙 BLE 开发框架进行介绍
- 更新 低功耗相关例程文档，基于新的 SDK 框架进行介绍
- 更新 解决方案相关例程文档，基于新的 SDK 框架和蓝牙 BLE 开发框架进行介绍
- 修复 一些文档中的描述错误
- 移除 一些废弃例程的对应文档

### 5. TOOLS

- 更新 量产烧录工具 `PAN10xx Download Tool` 至 `v0.0.014` 版本：
  - 修复项目名输入限制超出 26 个字符，造成项目名长度超出 26 个字符下载失败问题
  - 修复上位机勾选芯片连接自动开始烧录功能无效的问题
  - 修复下载 SWD 加密项目，无法重复下载的问题
  - 添加 PAN-LINK KEY3 按键短按，跳到信息显示界面，读取芯片信息与芯片 MAC，以及项目程序 MAC 显示功能
  - 添加下载 SWD 加密密钥缓存，通过缓存密钥解密可以对芯片信息进行读取，直到 PAN-LINK 重启
  - 更新 PAN-LINK 拓展串口命令，支持量产烧录控制设置选择指定名称的离线项目程序作为当前项目程序，支持开始离线下载命令
  - 优化部分功能
  - 添加帮助菜单版本日志显示功能
- 更新 串口工具 `Panchip Serial Tool` 至 `v0.0.005` 版本：
  - 重构版本
- 更新 工具箱工具 `Panchip ToolBox` 至 `v0.0.006` 版本：
  - ​添加引出脚界面 PAN107BUA1A 芯片型号支持
  - 修复引出脚界面存在的潜在问题
- 新增 2.4G RF 抓包工具 `Panchip RF Packet Capture Tool`：
  - 通过 PAN107x 射频模块抓取对应协议的包数据，解析包数据信息并显示
  - ​支持 XN297、NRF52、B250K、BLE 等 RF 包格式
- 更新 RF 测试固件，新增 MSOP10 封装专用的测试固件

### 6. ISSUES

#### 遗留问题

- `BUG #873`:  兼容问题--peripheral_ota---与小米手机11配合升级，小米11安装的nRF Conenct软件版本是4.28时，无法升级


## **PAN10XX NDK v0.7.0**

PAN10XX Nimble DK `v0.7.0` *(2024-10-20)* 已发布：

### 1. SDK

#### nimble

- App Config 配置框架重构，所有配置选项统一使用 `sdk_config.h` 配置，并重新梳理各个配置的层次结构，使之更容易理解和使用
  - 将蓝牙 Host 与 Controller 相关的重要配置开放到 App 配置文件中
  - 将 FreeRTOS 相关的重要配置开放到 App 配置文件中
  - 将 Flash KVStore 区域的配置开放到 App 配置文件中
  - 将 Flash Map 与 Bootloader 相关配置开放到 APp 配置文件中
  - 使能 UART Log 后，支持从 App 配置文件中修改 Log UART Tx 引脚及波特率
  - 新增使能 Segger RTT Log 的配置
- 更新 所有例程的入口函数，由 `app_main()` 修改为 `app_init()`，以提醒用户此函数仅可编写初始化逻辑，不可以写 `while (1)`
- 新增 获取自定义蓝牙 MAC 地址滚码地址的接口 `pan10x_roll_mac_addr_get()`，用于配合 PanLink 的蓝牙 MAC 滚码烧录功能使用
- 修复 Nimble Standby Mode 1 低功耗流程中，配置为“唤醒不复位方式”后，唤醒后获取不到正确的 32K Counter 值的问题
- 新增 PAN3740（Sub-1G）支持及对应例程

#### Panchip HAL

- Panchip Spark BLE Controller Library：
  - PAN101x:
    - 新增 Local SCA 的动态更新逻辑
    - 优化 ACL 数据发送逻辑
    - 优化 Heap 占用计算方法
    - 修复 一些兼容性问题
    - 修复 RF 实际 Tx Power 比预期偏大的问题
  - PAN107x:
    - 新增 Local SCA 的动态更新逻辑
    - 优化 ACL 数据发送逻辑
    - 优化 Heap 占用计算方法
    - 修复 一些兼容性问题
    - 新增 硬件 ADV PDU Filter 功能
    - 修复 HCI_Encrypt_Changed_Evt 事件报告比秘钥分配 ACL（Master）晚的问题
    - 修复 RF 实际 Tx Power 比预期偏大的问题

- Panchip PRF (2.4G Private RF) Library:
  - 新增 `panchip_prf_get_pipe()` 接口
  - 优化 PRF IRQ 流程
  - 更新 Set Mac Address 接口
  - 更新 `panchip_prf_enable_rssi()` 与 `panchip_prf_disable_rssi()` 接口
  - 修复 RF 实际 Tx Power 比预期偏大的问题

- BSP:
  - 新增 Segger RTT Driver
  - 新增 Sub-1G Driver
  - 更新 量产芯片校准参数载入流程，新增 `CONFIG_SOC_INCREASE_LPLDOH_CALIB_CODE` 用于抬高 LPLDOH 电压
  - 更新 ADC Driver，以稍微牺牲温度采样精度的代价，缩短 `ADC_MeasureSocTemperature()` 的执行时间和占用 RAM
  - 更新 FMC Driver，支持 PAN101x MSOP10 芯片，并修复一些潜在问题

#### Samples

- bluetooth：
  - ble_cent_prph:
    - 新增 SMP 支持
  - bleprph_hr:
    - 新增 BLE_GAP_EVENT_CONN_UPDATE 事件处理
    - 新增 CONFIG_USER_ROLL_MAC_ADDR 配置，用于使能自定义的滚码 MAC 地址（默认不使能）
- os_debug:
  - os_rtt_logging:
     - 重构例程，使用新增加的系统级开启 RTT Log 机制的方式
- peripheral:
  - spi_master_dma_send_receive:
    - 新增 全双工收发数据的场景
  - spi_master_int_send_receive:
    - 新增 全双工收发数据的场景
  - spi_master_poll_send_receive:
    - 新增 全双工收发数据的场景
  - spi_slave_dma_receive_send:
    - 新增 全双工收发数据的场景
  - spi_slave_int_receive_send:
    - 新增 全双工收发数据的场景
  - spi_slave_poll_receive_send:
    - 新增 全双工收发数据的场景
- security:
  - swd_protection (新增):
    - 演示芯片通过禁止 SWD 正常通信的方式保护芯片数据的方法，并演示如何通过载入正确调试秘钥的方式恢复正常的 SWD 通信
- solutions:
  - ble_hid_selfie:
    - 优化兼容性，新增通过了 500 台手机的兼容性测试（总计 1000 台）
  - ble_hid_uart_capture (新增):
    - 演示基于 BLE HID 服务和串口透传服务实现用按键进行拍照、录像和调焦，并通过串口透传数据到手机的功能
  - prf_dongle (新增):
    - 配合 PAN108x 鼠标，演示 2.4G Dongle 的功能
  - rf_mini_test (新增):
    - 演示发送 RF 单载波的功能
- sub_1g:
  - cad_rx (新增):
    - 演示使用 PAN3740 的 CAD 特性检测 Sub-1G 信号的功能
  - cad_tx (新增):
    - 演示使用 PAN3740 的 CAD 特性检测 Sub-1G 信号的功能
  - intelligent_search_rx (新增):
    - 演示 PAN3740 Sub-1G 的智能搜索功能
  - intelligent_search_tx (新增):
    - 演示 PAN3740 Sub-1G 的智能搜索功能
  - packet_reception_rate (新增):
    - 演示 PAN3740 Sub-1G 收报率统计功能
  - plhd (新增):
    - 演示 PAN3740 Sub-1G 接收提前中断功能
  - trx_sample (新增):
    - 演示 PAN3740 Sub-1G 收发切换功能
- 其他：
  - 所有例程更新 configuration 配置框架，统一使用新的 `sdk_config.h` 配置文件

### 2. HDK

- 移除 PAN1070UA1A v1.4 版本核心板图纸、设计源文件、生产文件
- 新增 PAN1070UAEC 核心板图纸、设计源文件、生产文件
- 新增 PAN1010M9BA 核心板图纸、设计源文件、生产文件

### 3. MCU

- 重构 **LowPower** 例程：
  - 梳理并重构例程代码框架，重写所有测试用例，使例程更加易懂和易于移植
  - 新增例程演示文档
- 新增 **PRF_API_RX** 和 **PRF_API_TX** 例程：
  - 演示 PRF 2.4G API 收发接口
  - 新增例程演示文档
- 新增 **PRF_RX_MULTI_PIPE** 和 **PRF_TX_MULTI_PIPE** 例程：
  - 演示 PRF 2.4G API Multi-Pipe 功能
  - 新增例程演示文档
- 更新 **usb_hid** 例程：
  - 修复一些代码问题
  - 新增例程演示文档

### 4. DOC

- 更新 `NDK 快速入门指南` 文档，补充一些描述模糊的部分，并新增一些参考文档的跳转链接
- 更新 `NDK 开发环境搭建` 文档，补充更多内容
- 新增 `NDK Chip Migration` 文档，介绍 PAN107x 和 PAN101x 芯片的不同点以及移植的一般方法
- 更新 `NDK Configuration 开发指南` 文档，介绍重构后的 SDK Config 配置机制，并解释各个 Config 选项的含义
- 更新 `NDK 低功耗开发指南` 文档，增加芯片电源管理框图
- 更新 `BLE MULTI ROLE` 例程文档，新增不同版本 Controller Lib 的选择介绍
- 更新 `BLE Peripheral HR OTA` 例程文档，添加 OTA 功能移植相关介绍
- 更新 `Firmware Encryption` 例程文档
- 新增 `OS RTT Logging` 例程文档
- 新增 `SWD Protection` 例程文档
- 新增 `PRF Dongle` 方案例程文档
- 新增 `RF Mini Test` 方案例程文档
- 新增 `Sub1G - CAD` 例程文档
- 新增 `Sub1G - Intelligent Search` 例程文档
- 新增 `Sub1G - Packet Reception Rate` 例程文档
- 新增 `Sub1G - PLHD` 例程文档
- 新增 `Sub1G - TRx Sample` 例程文档
- 更新 例程介绍主页，补充一些之前版本漏掉的文档链接
- 修复 一些文档中的描述错误，并优化一些文档的显示效果

### 5. TOOLS

- 更新 量产烧录工具 `PAN10xx Download Tool` 至 `v0.0.008` 版本：
  - 修复自定义下载配置模式保存配置文件，在下载加密信息使能时，在下载配置模式加载配置失败问题
  - 固定蓝牙 MAC 滚码写入位置为 Flash INFO 区 0x100 地址
  - 添加支持无 RST 脚芯片进入 ROM 模式下载程序的功能
  - 添加支持下载 SWD Debug 加密芯片，通过密钥解密进行再次下载功能
  - 优化烧录加密信息写 efuse，写之前先进行读取校验，防止重复写入
- 更新 RF测试固件 至 `v003`，支持更多封装的芯片

### 6. ISSUES

#### 遗留问题

- `BUG #873`:  兼容问题--peripheral_ota---与小米手机11配合升级，小米11安装的nRF Conenct软件版本是4.28时，无法升级


## **PAN10XX NDK v0.6.0**

PAN10XX Nimble DK `v0.6.0` *(2024-08-05)* 已发布：

### 1. SDK

#### nimble

- 优化 Nimble 蓝牙 Host 层代码，新增 Mem Pool 机制，以支持 PAN101x 芯片小 Memroy 的场景
- 优化 Nimble 系统启动流程，将蓝牙初始化部分与 OS 初始化部分解耦，使得蓝牙功能关闭后，系统仍可正常运行
- 优化 Nimble DeepSleep 低功耗流程，并新增 Standby Mode 1 和 Standby Mode0 支持
- 修复 DeepSleep 唤醒瞬间出现大电流的问题
- 新增 一些与低功耗相关的 API 接口（os_lp.h）：
  - `soc_lptmr_cycle_get()`
  - `soc_32k_clock_freq_get()`
  - `soc_lptmr_uptime_get_ms()`
  - `soc_busy_wait()`
  - `soc_reset_reason_get()`
  - `soc_stbm1_gpio_wakeup_src_get()`
  - `soc_enter_standby_mode_0()`
  - `soc_enter_standby_mode_1()`
- 修改 FreeRTOS `vApplicationIdleHook()` 函数的定义，使其有返回值，以满足低功耗场景的使用需求
- 更新 bootloader，优化 2.4G OTA 升级时间与稳定性

#### Panchip HAL

- Panchip Spark BLE Controller Library：
  - 优化 时序以降低功耗
  - 优化 RF 性能

- Panchip PRF (2.4G Private RF) Library:
  - 新增 TRX 最小转换时间配置接口
  - 优化 B250K 通信

- BSP:
  - 更新 量产芯片校准参数载入流程
  - 更新 DMA Driver，移除不必要的接口
  - 更新 FMC Driver，优化一些接口的实现以提示 Flash 操作的可靠性
  - 更新 I2C Driver，移除一些不必要的代码
  - 更新 Timer Driver，修复某些场景下 Timer0 无法正常使用的问题
  - 新增 各个外设的 HAL 层 Driver，它们是基于各自底层 Driver 抽象出来的较上层的 Driver，简化了外设的使用方法

#### Samples

- bluetooth：
  - ble_multi_role:
    - 新增 两主三从演示
  - bleprph_enc:
    - 支持 pan101x 芯片 (新增 pan101x 芯片工程)
  - bleprph_hr:
    - 新增 非连接广播演示开关（默认不使能）
    - 新增 更新 PHY 开关（默认不使能）
  - bleprph_throughput:
    - 新增 手机测试方法，简化调试流程
- low_power:
  - deepsleep_gpio_key_wakeup (新增):
    - 演示 SoC 进入 DeepSleep 状态，并通过 GPIO 按键将其唤醒
  - deepsleep_gpio_pwm_wakeup (新增):
    - 演示 SoC 进入 DeepSleep 状态，使用外部 PWM 波形通过 GPIO 将其唤醒
  - deepsleep_pwm_waveform_generator (新增):
    - 演示 SoC 在 DeepSleep 状态下输出 PWM 波形，并使用 APB HW Timer0 定时唤醒并修改 PWM 波形周期和占空比
  - deepsleep_slptmr_wakeup (新增):
    - 演示 SoC 进入 DeepSleep 状态，并通过 SleepTimer 定时器将其唤醒
  - standby_m1_gpio_key_wakeup (新增):
    - 演示 SoC 进入 Standby Mode 1 状态，并通过 GPIO 按键将其唤醒
  - standby_m1_slptmr_wakeup (新增):
    - 演示 SoC 进入 Standby Mode 1 状态，并通过 SleepTimer 定时器将其唤醒
  - standby_m0_p02_key_wakeup (新增):
    - 演示 SoC 进入 Standby Mode 0 状态，并通过 WKUP (P02) 按键将其唤醒
  - multiple_wakeup_source (新增):
    - 演示 SoC 多种唤醒源、多种低功耗模式之间的切换
- os_debug:
  - os_rtt_logging (新增):
     - 演示使用 Segger Jlink RTT 的方式打印 Log 的方法
- peripheral:
  - adc_read_multiple_channels (新增):
    - 演示 ADC HAL Driver 的使用方法
  - gpio_digital_input_interrupt (新增):
    - 演示使用 GPIO HAL Driver 实现中断方式的 GPIO 输入检测功能
  - gpio_digital_input_polling (新增):
    - 演示使用 GPIO HAL Driver 实现查询方式的 GPIO 输入检测功能
  - gpio_output_open_drain (新增):
    - 演示使用 GPIO HAL Driver 实现 GPIO 开漏（Open-Drain）输出功能
  - gpio_output_push_pull (新增):
    - 演示使用 GPIO HAL Driver 实现 GPIO 推挽（Push-Pull）输出功能
  - gpio_simple_convenient_apis (新增):
    - 演示 GPIO 底层 Driver 中提供的几个简单好用的接口
  - i2c_master_dma_receive (新增):
    - 演示使用 I2C HAL Driver 实现 I2C Master 的 DMA 接收功能
  - i2c_master_int_send (新增):
    - 演示使用 I2C HAL Driver 实现 I2C Master 的中断发送功能
  - i2c_master_poll_send (新增):
    - 演示使用 I2C HAL Driver 实现 I2C Master 的查询发送功能
  - i2c_slave_dma_send (新增):
    - 演示使用 I2C HAL Driver 实现 I2C Slave 的 DMA 发送功能
  - i2c_slave_int_receive (新增):
    - 演示使用 I2C HAL Driver 实现 I2C Slave 的中断接收功能
  - i2c_slave_poll_receive (新增):
    - 演示使用 I2C HAL Driver 实现 I2C Slave 的查询接收功能
  - pwm (新增):
    - 演示 PWM HAL Driver 的使用方法
  - spi_master_dma_send_receive (新增):
    - 演示使用 SPI HAL Driver 实现 SPI Master DMA 方式先发后收功能
  - spi_master_int_send_receive (新增):
    - 演示使用 SPI HAL Driver 实现 SPI Master 中断方式先发后收功能
  - spi_master_poll_send_receive (新增):
    - 演示使用 SPI HAL Driver 实现 SPI Master 查询方式先发后收功能
  - spi_slave_dma_receive_send (新增):
    - 演示使用 SPI HAL Driver 实现 SPI Slave DMA 方式先收后发功能
  - spi_slave_int_receive_send (新增):
    - 演示使用 SPI HAL Driver 实现 SPI Slave 中断方式先收后发功能
  - spi_slave_poll_receive_send (新增):
    - 演示使用 SPI HAL Driver 实现 SPI Slave 查询方式先收后发功能
  - timer_basic (新增):
    - 演示 Timer HAL Driver 的计数（计时）功能
  - timer_capture (新增):
    - 演示 Timer HAL Driver 的输入捕获功能
  - uart_dma (新增):
    - 演示使用 UART HAL Driver 实现 UART DMA 方式收发数据功能
  - uart_fifo (新增):
    - 演示使用 UART HAL Driver 实现 UART 中断方式收发数据功能
  - wdt (新增):
    - 演示 WDT (Watchdog) HAL Driver 的使用方法
  - wwdt (新增):
    - 演示 WWDT (Window Watchdog) HAL Driver 的使用方法
- security:
  - fw_encryption (新增):
    - 演示芯片通过固件加密、硬件解密的机制保护 Flash 关键代码的方法
- solutions:
  - ble_accelerometer (新增):
    - 演示通过蓝牙自定义服务将 EVB 加速度传感器数据上报到对端的方法
  - ble_app_uart (新增):
    - 演示蓝牙从机串口透传功能，从机设备和手机或主机设备连接后可以和串口模块进行数据透传
  - ble_spi_tft_lcd (新增):
    - 演示使用蓝牙自定义复位将字符数据通过蓝牙传输到芯片中并显示在 EVB OLED 屏幕中的方法
  - ble_hid_selfie:
    - 支持 pan101x 芯片 (新增 pan101x 芯片工程)
  - ble_hid_uart_mult_roles:
    - 修复 UART1 作为串口透传功能不正常的问题
    - 修复 从机 Notify 到主机的异常
    - 修复 Watchdog 使能后会异常复位的问题
- 其他：
  - 所有例程更新 configuration 配置，修复一些错误，并新增一些配置选项

### 2. HDK

- 新增 PAN1070UA1A 图纸、设计源文件、生产文件至 v1.5 版本

### 3. MCU

- 更新 **ADC** 例程：
  - 使能 ADC Buffer 以提升采样准确性
- 更新 PAN101x/PAN107x 芯片 Keil Flash 烧录算法（FLM）文件，提高稳定性 (位于 **mcu_misc** 目录)
- 底层 Driver 例程移除一些不必要的代码

### 4. DOC

- 更新 文档中心主页，新增 PAN101x 规格书等下载链接，并优化一些表述
- 更新 `PAN10xx 硬件参考设计` 文档
- 更新 `BLE MULTI ROLE` 例程文档
- 更新 `BLE Peripheral Throughput Test` 例程文档
- 新增 Low Power 低功耗相关例程文档 8 篇
- 新增 Peripheral 外设相关例程文档 25 篇
- 新增 Security 固件加密例程文档 1 篇
- 更新 `BLE Accelerometer` 方案例程文档
- 更新 `BLE APP UART` 方案例程文档
- 更新 `BLE HID Selfie` 方案例程文档
- 更新 `BLE Spi Tft Lcd` 方案例程文档
- 更新 `NDK App 开发指南` 文档，更新 PAN1070 的功耗测试结果，并新增 PAN1010 的功耗测试结果
- 修复 一些文档中的描述错误

### 5. TOOLS

- 更新 Panchip 2.4G OTA 工具至 `v0.0.004` 版本：
  - 修改通讯速率支持最大到 2000000 Hz
  - 添加 OTA 过程传输与等待配置
  - 修改 OTA 传输数据回复协议支持
- 更新 量产烧录工具 `PAN10xx Download Tool` 至 `v0.0.005` 版本：
  - 修复功能->读取 PHY 问题
  - 更新 PAN-LINK 支持外部 AD1 IO 输出控制 RST 输出实现，控制芯片的 RST 脚功能
  - 更新下载加密信息功能，配合SDK发布的加密信息文件，实现芯片 Flash 程序加密
  - 修复烧录擦除芯片结果 log 显示错误问题
  - 修复下载特殊程序有失败的问题
- 更新 调试工具 目录：
  - 更新 JLink 工具中的 FLM 文件至最新版本

### 6. ISSUES

#### 遗留问题

- `BUG #873`:  兼容问题--peripheral_ota---与小米手机11配合升级，小米11安装的nRF Conenct软件版本是4.28时，无法升级


## **PAN1070 NDK v0.5.0**

PAN1070 Nimble DK `v0.5.0` *(2024-06-07)* 已发布：

**注**：PAN1070 NDK 现已兼容 PAN101x 系列芯片。

### 1. SDK

#### nimble

- 优化 Nimble Samples Configuration 配置选项
- 添加 系统看门狗功能
- 优化 例程 SRAM 资源消耗
- 新增 对 PAN101x 芯片的支持
- 优化 温度自动检测流程，修复与 App 层同时使用 ADC 产生冲突的问题

#### Panchip HAL

- Panchip Spark BLE Controller Library：
  - 更新 PHY 驱动，优化 RF 性能
  - 新增 动态修改 Tx Power 接口

- Panchip PRF (2.4G Private RF) Library:
  - 更新 PHY 配置，优化 RF 性能
  - 更新 Tx Power

- BSP:
  - 更新 FT 校准信息载入流程，并节约一些 SRAM
  - 新增 对 PAN101x 芯片的支持，包括 Driver 及 Flash 烧录算法等
  - 更新 ADC Driver，使其能够兼容不同 FT 版本的芯片
  - 更新 CLK Driver，优化 WDT/WWDT 时钟源选择接口
  - 更新 I2C Driver，修复 I2C 例程无法正常工作的问题
  - 更新 LP Driver，关闭 DeepSleep 状态下 Flash 的供电，以节约功耗
  - 新增 Power Driver，用于 Nimble 相关例程中根据当前温度动态修改芯片各个供电配置

#### Samples

- 蓝牙：
  - `bluetooth/bleprph_hr`
    - 支持 pan101x 芯片 (新增 pan101x 芯片工程)
  - `solutions/ble_rgb_light`
    - 支持 pan101x 芯片 (新增 pan101x 芯片工程)
- 其他：
  - `mcu_boot`: 更新 bootloader，优化 2.4G OTA 功能
  - 所有例程优化 configuration 配置框架

### 2. HDK

- 新增 PAN1010S9FA 核心板图纸

### 3. MCU

- 更新 **ADC** 例程：
  - 优化使用流程，使采样结果更准确
- 更新 PAN1070_PRF_TRX开发指南.pdf 文档
- 新增 PAN101x 芯片 Keil Flash 烧录算法（FLM）文件 PAN101X_252KB_FLASH.FLM (位于 **mcu_misc** 目录)
- 所有例程：
  - 更新 芯片校准信息载入流程及相关 Log 输出
  - 增加 对 PAN101x 芯片的支持（源码与 PAN107x 共用，但新增 PAN101x Keil Project，注意有少数例程因 PAN101x 引脚限制无法支持）

### 4. DOC

- 更新 文档中心主页，新增 PAN101x 相关内容介绍
- 更新 `NDK 快速入门指南` 文档，并增加对 PAN101x 的描述
- 新增 `NDK Configuration` 配置开发指南文档
- 更新 `PAN107x EVB 介绍` 文档，将其更名为 `PAN10xx EVB 介绍`，并新增对 PAN101x 相关介绍
- 更新 `PAN107x 硬件参考设计` 文档，将其更名为 `PAN10xx 硬件参考设计`，并新增对 PAN101x 相关介绍
- 更新 `BLE Peripheral HR` 例程文档，新增对 PAN101x 芯片支持情况的描述
- 优化 `BLE RGB Light` 例程文档，新增对 PAN101x 芯片支持情况的描述
- 更新 `NDK Mcu Boot` 开发指南文档，新增生成签名文件的环境配置介绍
- 更新 `量产烧录` 工具说明文档，增加对 PAN101x 芯片的描述
- 新增 `RF TEST` 说明文档，介绍 RF 测试固件的使用方法
- 新增 `JFlash 烧录` 说明文档，介绍使用 Segger J-FLash 工具烧录固件到 PAN107x SoC 的方法
- 新增 `Panchip 2.4G OTA 工具` 说明文档，介绍 2.4G OTA 的主机对从机设备进行 OTA 升级的方法
- 更新 `NDK 常见问题（FAQs）` 文档，阐述某些情况下，芯片正常工作的时候，使用 JLink (SWD) 无法（或很难）再次烧录程序的原因及解决方法
- 更新 所有文档中与特定芯片相关的描述，新增对 PAN101x 芯片支持情况的描述
- 修复 一些表述上的问题

### 5. TOOLS

- 新增 Panchip 2.4G OTA 工具，用于配合 OTA 主机对从设备进行 OTA 升级
- 更新 量产烧录工具 `PAN10xx Download Tool` 的介绍：
  - 新增 对 PAN101x 支持情况介绍
- 更新 RF测试固件 至 `v002`，优化性能
- 更新 调试工具 目录：
  - 新增 `ForceEraseVectorTable_PAN107x.bat` 脚本，可擦除 芯片 Flash 上的 Vector Table，阻止程序正常执行（详见 开发指南/FAQs 文档相关说明）

### 6. ISSUES

#### 新增问题

- `BUG #873`:  兼容问题--peripheral_ota---与小米手机11配合升级，小米11安装的nRF Conenct软件版本是4.28时，无法升级

#### 遗留问题

- `BUG #802`:  PRF OTA，带OTA作为client，利用ota升级其他设备偶尔会失败



## **PAN1070 NDK v0.4.0**

PAN1070 Nimble DK `v0.4.0` *(2024-04-03)* 已发布：

### 1. SDK

#### nimble

- 优化 Keil 工程编译信息，清除编译警告
- 优化 app_config_spark.h 的配置选项和结构层次
- 优化 SoC Power Domain，进而优化功耗（支持定时检测温度并根据当前温度优化芯片Power配置）

#### Panchip HAL

- Panchip Spark BLE Controller Library：
  - 优化 RF 性能
  - 修复 RCL 作为低功耗时钟时的连接问题
  - 修复 RF PHY 问题

- Panchip PRF (2.4G Private RF) Library:
  -  更新 DCOC 校准流程
  -  修复 频点设置接口 Bug
  -  更新 Tx Power 档位
  -  更新 g_250k deviation 为 170k
  -  优化 读 rssi 接口, 增加 rssi 全局变量

- BSP:
  - 更新 FT 校准信息载入流程
  - 更新 ADC Driver，新增一些 API 接口，以简化 ADC 使用流程
  - 更新 CLK Driver，新增选择 PWM 时钟源的 API 接口
  - 更新 I2C Driver，修复潜在的问题
  - 更新 PWM Driver，新增一些易用的 API 接口
  - 更新 TIMER Driver，修复一些问题
  - 修复 一些寄存器名称错误

#### Samples

- 蓝牙：
  - `bluetooth/ble_multi_role` (新增)
     - BLE 多主多从例程
  - `bluetooth\bleprph_throughput` (新增)
     - BLE 从机吞吐率例程
  - `bluetooth\bleprph_distance` (新增)
     - BLE 距离测试例程（心跳服务以及支持不同phy切换）
  - `bluetooth/peripheral_hr`
     - 修复多次断连后重连死机问题
- 方案：
  - `solutions/ble_vehicles_key`
    - 适配RSSI波形显示
    - 修复不同手机配对多次产生cccd settings条目不够最终导致音量调整失效的情况
- 其他：
  - `pan107x_mcu_boot`: 更新 bootloader，新增 2.4G OTA 功能

### 2. HDK

- 移除 过期的测试板图纸

### 3. MCU

- 新增 **PRF_OTA_CLIENT** 例程：
  - 2.4G OTA 客户端工程，演示 2.4G OTA 功能
- 新增 **PRF_TX_SAMPLE_UI** 和 **PRF_RX_SAMPLE_UI** 例程：
  - 带屏幕显示的 2.4G 距离测试例程
- 移除 **mcu_misc** 目录下的旧版本 Keil Flash 烧录算法（FLM）文件，新增 PAN107X_508KB_FLASH.FLM
- 更新 所有 Keil 工程默认使用的 FLM 文件

### 4. DOC

- 新增 `ble_multi_role` 例程文档
- 新增 `bleprph_distance` 例程文档
- 新增 `bleprph_throughput` 例程文档
- 新增 `mcu_samples_doc/PAN1070_PRF_UI距离测试说明.pdf` 例程文档
- 更新 `ndk_develop_environment_intro` 介绍文档，更新 FLM 文件说明
- 更新 `ndk_mcu_boot` 开发指南文档，新增生成签名文件的环境配置介绍
- 优化 文档目录架构，拆分了 NDK 和 ZDK 文档，使得文档架构更加清晰

### 5. TOOLS

- 更新 工具箱工具 `PAN107x ToolBox` 至 `v0.0.004`：
  - 新增 USB 通信兼容
- 新增 RF测试固件：
  - 新增 PAN107x RF测试固件
  - 新增 PAN107x RSSI VIEWER测试固件
- 新增 JLink v6.44b 软件
  - 支持 PAN107x 芯片的 Jlink 命令行调试，JFlash 烧录等

### 6. ISSUES

#### 新增问题

- `BUG #802`:  PRF OTA，带OTA作为client，利用ota升级其他设备偶尔会失败


## **PAN1070 NDK v0.3.0**

PAN1070 Nimble DK `v0.3.0` *(2024-01-19)* 已发布：

### 1. SDK

#### nimble

- 新增 Bootloader，并默认在各例程中使能，可通过 App 工程配置文件禁用
- 新增 SMP BT 子系统，以支持蓝牙 OTA 功能
- 更新 nimble ble host 一些细节
- 新增 蓝牙低功耗定向优化配置，用于一些特殊的功耗测试场景

#### Panchip HAL

- Panchip Spark BLE Controller Library：
  - 优化 SRAM 占用
  - 优化 MD
  - 新增 运动健康协议支持
  - 新增 DTM 支持
  - 优化 adv Rx timeout 至 60us
  - 优化 RF Post Tx Time
  - 更新 PHY 参数
  - 修复 断连信息未及时清除问题
  - 修复 0x28 断连问题

- Panchip PRF (2.4G Private RF) Library:
  - 更新 PHY 参数
  - 完善 一些 API 接口

- BSP:
  - 更新 FT 校准信息载入流程
  - 更新 ADC Driver，新增一些 API 接口，以简化 ADC 使用流程
  - 优化 系统启动流程
  - 修复 一些引脚定义错误
  - 修复 GPIO_DB 相关结构体名称错误的问题
  - 修复 低功耗 Driver 的潜在问题
  - 移除 一些不必要的代码以避免潜在的编译错误风险

#### 演示例程

- 蓝牙：
  - `bluetooth/peripheral_hr_ota` (新增)
    - 演示蓝牙 OTA 功能
- 方案：
  - `solutions/ble_mouse` (新增)
    - BLE 鼠标方案
  - `solutions/multimode_mouse` (新增)
    - 多模鼠标方案
  - `solutions/multimode_mouse_dongle` (新增)
    - 多模鼠标配套 Dongle 方案
  - `solutions/ble_prf_sample` (新增)
    - BLE & 2.4G 双模方案
- 其他：
  - 所有例程均添加了 OTA 支持，并提供了 3 种编译和配置模式：
    - Bare Metal
    - OTA in Bootloder
    - OTA in App

### 2. HDK

- 新增 PAN107x EVB 底板图纸、设计源文件、生产文件 v1.1

### 3. MCU

- 更新 **ADC** 演示例程：
  - 优化 ADC Convert Test、VDD/4 Test、Temperature Test 流程，使用新的接口以简化使用
- 更新 **CLK** 演示例程：
  - 修复 一些问题
- 更新 **LP** 演示例程：
  - 重命名例程名称为 **LowPower**
- 新增 **PRF_Template_SAMPLE** 例程：
  - 2.4G 模板工程，以方便用户快速创建自己的 2.4G 工程
- 其他：
  - 修复 GPIO_DB 相关结构体名称错误的问题
  - 修复 例程生成的 Image 名称与预期不一致的问题

### 4. DOC

- 新增 `ble_mouse` 例程文档
- 新增 `multimode_mouse` 例程文档
- 新增 `multimode_mouse_dongle` 例程文档
- 新增 `ble_prf_sample` 例程文档
- 更新 `mcu_samples_doc/PAN1070_ADC例程说明.pdf` 例程文档，以匹配工程最新的修改
- 新增 `ndk_mcu_boot` 开发指南文档，介绍 NDK 的 Bootloader
- 新增 `pan107x_evb_intro` 硬件资料文档，介绍 PAN107X EVB 相关内容
- 更新 `pan107x_hw_reference_design` 硬件参考设计文档，修改了一些具体描述
- 新增 `to0lbox_intro` 工具箱工具介绍文档

### 5. TOOLS

- 更新 量产烧录工具 `PAN107x Download Tool` 至 `v0.0.002`：
  - 修复 一些潜在问题
- 新增 工具箱工具 `PAN107x ToolBox v0.0.003`：
  - 新增 引出脚界面
  - 新增 RF 信号采集界面


## **PAN1070 NDK v0.2.0**

PAN1070 Nimble DK `v0.2.0` *(2023-11-19)* 已发布：

### 1. SDK

#### nimble

- 更新 BLE Controller，优化一些内部流程并修复一些问题
- 新增 获取 MAC 地址的接口

#### Panchip HAL

- 新增 载入 Hardware Calibration 校准参数的接口
- 优化 WDT 接口，扩大 WDT Reset 的复位范围
- 更新 RF Lib，优化 2.4G 通信流程

#### 演示例程

- `ble_cent_prph`（新增）: 演示蓝牙主从一体功能
- `ble_central`（新增）: 演示蓝牙主机功能
- `bleprph_hr`（新增）: 演示蓝牙从机功能，包含 GATT服务：HR (Heart Rate)，连接订阅服务后，会上报虚拟的心率值
- `bleprph_enc`（新增）: 演示外设以及加密配对功能，可以和主机示例进行对测
- `ble_hid_selfie`（新增）: 自拍解决方案，通过蓝牙HID控制手机拍照
- `ble_panchip_cte_beacon`（新增）: Panchip 蓝牙定位标签方案，通过发送特定的广播数据，实现蓝牙定位功能
- `ble_rgb_light`（新增）: 蓝牙 RGB 灯控方案，演示 BLE RGB 灯与手机 APP 进行连接，通过 APP 控制 RGB 灯的亮度与颜色
- `ble_hid_uart_mult_roles`（新增）: 蓝牙串口透传解决方案，演示蓝牙hid串口透传功能，支持1主1从
- `ble_vehicles_key>`（新增）: 蓝牙车钥匙解决方案，演示基于HID服务的自动连接服务

### 2. HDK

- 新增 PAN1070 UA1A EVB 图纸、设计源文件、生产文件

### 3. MCU

- 更新 LP 低功耗例程，优化 CPU Retention and Remap 流程
- 更新 2.4G 例程及对应文档，演示更多的通信模式
- 更新 各个底层 Driver 例程，增加初始化阶段载入芯片校准信息的流程

### 4. DOC

- 新增 `ble_cent_prph` 例程文档
- 新增 `ble_central` 例程文档
- 新增 `bleprph_enc` 例程文档
- 新增 `bleprph_hr` 例程文档
- 新增 `ble_hid_selfie` 例程文档
- 新增 `ble_hid_uart_mult_roles` 例程文档
- 新增 `ble_pcte_beacon` 例程文档
- 新增 `ble_rgb_light` 例程文档
- 新增 `ble_vehicles_key` 例程文档
- 新增 `NDK App 开发指南` 文档
- 新增 `PAN107x 硬件参考设计` 文档
- 新增 `量产烧录` 说明文档

### 5. TOOLS

- 新增 量产烧录工具 `PAN107x Download Tool`
- 新增 Testbox RF 测试固件


## **PAN1070 NDK v0.1.0**

PAN1070 Nimble DK `v0.1.0` *(2023-10-24)* 已发布：

### 1. SDK

NDK 软件开发框架基于 Keil + FreeRTOS + NimBLE，其中：

- Keil 是 SDK 支持的软件开发环境
- FreeRTOS 是一个开源实时操作系统（RTOS），用于配合 NimBLE 实现蓝牙应用
- NimBLE 是一个开源低功耗蓝牙（BLE）5.1 协议栈，其实际上是 Apache Mynewt 项目的一部分

#### 解决方案

- `esl`: ESL 价签方案演示例程，支持外部 SPI Flash 存储、EPD 墨水屏、低功耗模式、RF 通信等功能。

### 2. HDK

目前版本提供了如下硬件相关资料：

- PAN107B QFN40 测试板图纸、设计源文件、生产文件

### 3. MCU

目前版本提供了如下 MCU 裸机 Keil 例程及相关文档：

- ADC
- CLK
- CLKTRIM
- DebugProtect
- DMA
- EFUSE
- FMC
- GPIO
- I2C
- LP
- PRF_B250K_RX
- PRF_B250K_TX
- PWM
- SPI
- TIMER
- UART
- USB_HID
- WDT
- WWDT

### 4. DOC

目前版本提供了如下文档：

- NDK 快速入门指南
- NDK 开发环境介绍
- NDK 整体框架介绍
- Nimble 简介
- PAN107x 硬件参考设计指南
- ESL 电子货架标签方案例程说明
- MCU 底层外设驱动例程说明
- 低功耗开发指南
- NDK RAM 使用情况分析以及优化指南

### 5. TOOLS

目前版本提供了如下工具：
- 串口工具（PC工具）
- Air Sync Debugger（手机测试软件安卓APK）
- Google Home（手机测试软件安卓APK）
- nRF Connect（手机测试软件安卓APK）
- nRF Mesh（手机测试软件安卓APK）
- Siliconlabs Bluetooth Mesh（手机测试软件安卓APK）

### 6. 已知问题

- MCU USB_HID 例程暂未通过测试
