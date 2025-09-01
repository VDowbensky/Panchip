# 12_fhss: FHSS 跳频通信示例（含配对与数据通信）

本示例在两块开发板上实现一个简化版 FHSS（Frequency Hopping Spread Spectrum）通信：
- 固定调制参数：BW=500K，SF=7，私有 SyncWord。
- 配对阶段：TX 在绑定频点广播 Bind 包，RX 解析后回复 ACK，双方共享随机种子与启动时间。
- 数据阶段：双方基于相同种子生成相同跳频序列，在统一时隙节拍上同步跳频并收发数据。

## 1. 目录结构
- TX 端：`01_SDK/example/12_fhss/tx/`
    - 源码：`source/main.c`
    - 工程：`keil/`
- RX 端：`01_SDK/example/12_fhss/rx/`
    - 源码：`source/main.c`
    - 工程：`keil/`

## 2. 环境要求
- 两块 PAN3029/3060 开发板（分别烧录 TX 与 RX）。
- USB 线 2 根，用于供电与串口日志。
- J-Link 下载器 1 个。
- 串口参数：115200 8N1。

## 3. 编译与烧录
- TX 端：打开 `tx/keil` 下工程编译、下载至开发板。
- RX 端：打开 `rx/keil` 下工程编译、下载至另一开发板。

## 4. 工作流程概述
1) 上电初始化 RF：设置 BW=500K、SF=7、私有 SyncWord、Preamble 等。
2) 配对阶段：
     - TX 在绑定频点（默认 490MHz）周期性发送 Bind 包（包含：随机种子、信道数、时隙时长、启动延迟、TX ID），并短窗监听 ACK。
     - RX 在绑定频点监听 Bind，解析后立即回复 ACK；双方以相同随机种子生成相同跳频序列，并约定一个未来的启动时间（当前 tick + 启动延迟）。
3) 数据阶段：
     - 到达启动时间后，双方以固定时隙周期（默认 10ms）按相同顺序跳频；
     - TX 在每个槽发送一帧数据；RX 在对应槽开启带超时接收窗口并打印数据、SNR、RSSI。

## 5. 关键参数（可在源码顶部宏修改）
（TX 与 RX 需保持一致）
- 基准频点：`FHSS_BASE_FREQ_HZ`（默认 490500000 Hz）
- 频点间隔：`FHSS_CH_SPACING_HZ`（默认 1 MHz）
- 信道数量：`FHSS_CH_NUM`（默认 16）
- 时隙长度：`FHSS_SLOT_MS`（默认 100 ms）
- 启动延迟：`FHSS_START_DELAY_MS`（默认 500 ms）
- 绑定频点：`FHSS_BIND_FREQ_HZ`（默认等于基准频点）
- 私有同步字：`RF_MAC_PRIVATE_SYNCWORD`

说明：FHSS 序列通过相同随机种子进行 Fisher-Yates 洗牌生成；频点表按“基准频点 + n×间隔”构成。

## 6. 绑定与帧格式（简要）
- Bind 包（TX→RX）：
    - 字段：`[0xB1 | 0x01 | seed(4) | chNum(1) | slotMs(2) | startDelayMs(2) | txId(4)]`
- ACK 包（RX→TX）：
    - 字段：`[0xA1 | 0x01 | txId(4) | rxId(4)]`
- 数据包（TX→RX）：
    - 字段：`[0xD1 | 0x01 | cnt(4) | payload(8)]`（示例负载）

## 7. 运行与观测
1) 上电运行 RX，串口日志会显示等待绑定：
```
[RX] Wait binding on 490500000Hz...
```
2) 上电运行 TX，TX 端打印发送 Bind、收到 ACK、开始 FHSS：
```
[TX] Enter binding... base=490500000Hz, ch=16, slot=10ms, delay=500ms
[TX] Got ACK. Pair success.
[TX] FHSS start at tick=XXXXX, slot=10ms, ch=16
[TX] slot=3 ch=5 freq=494500000Hz cnt=0
...
```
3) RX 端打印配对成功与接收数据：
```
[RX] Pair OK. ch=16 slot=10ms startDelay=500ms
[RX] slot=3 ch=5 freq=493500000Hz len=10
+RxHexData:
...
SNR:8dB, RSSI:-28dBm
```

## 8. 注意事项与建议
- TX 与 RX 的宏参数需一致，特别是通道数、间隔、启动延迟等。
- 槽位调度依赖系统 `SysTick_GetTick()`，确保其时基准确且为 ms 计数。
- 示例为简化实现：未加入时钟漂移补偿、丢槽重同步、信道黑名单等机制；如需更强鲁棒性可在此基础上扩展。
- 频点与步进应遵循当地法规与硬件频段支持范围。

## 9. 常见问题（FAQ）

本示例仅用于演示 FHSS 的基本流程，便于二次开发与验证。若需与现有协议兼容或扩展安全性（鉴权/加密），请基于该框架增加相应逻辑。
