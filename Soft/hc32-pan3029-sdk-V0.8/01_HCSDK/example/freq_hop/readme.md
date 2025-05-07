# freq_hop说明

## 1. 功能说明
- 芯片支持CAD-IRQ中断，开启CAD功能并进入Rx模式后，芯片会检测信道中是否存在ChirpIOT™信号，如果存在则将CAD-IRQ置高，外部MCU可以通过在一定时间内检测CAD-IRQ信号是否拉高来判断信道中是否存在ChirpIOT™信号。
- 使用CAD功能，通过软件逻辑实现跳频通信。
- 跳频功能实现逻辑（2个频点）
  - 接收端进行2个频点切换，切换频点后，进行CAD-RX检测。如果检测到有用信号，则继续接收；如果未检测到有用信号，则切换另一个频点，重新进行CAD-RX检测。
  - 发射端根据实际情况配置合适的前导码个数。发射前进行CAD-TX信道检测，如果信道空闲，则进行数据发射；如果信道忙，则随机延时退避后，重新进行CAD-TX检测。

## 2. freq_hop_tx

- CAD功能可以被用于发射前的信道检测，以保证当前信道空闲，随后进行数据发射，避免无线信号碰撞干扰，提高通信成功率。
- 软件应用参考
  - 设置待检测信道的基本参数，SF/BW/FREQ等
  - 设置合适的前导码个数，计算方式参考文末
  - 随机选择频点和数据包长度
  - 调用rf_cad_detect_start();检测接口。进行CAD-TX检测。
  - 检测接口内设置cad_tx_detect_flag为MAC_EVT_TX_CAD_NONE，打开CAD功能，设置CAD接收检测，设置超时定时器
  - 如果触发CAD IO中断处理函数，则将cad_tx_detect_flag置位为MAC_EVT_TX_CAD_ACTIVE
  - 如果触发定时器超时回调函数，则将cad_tx_detect_flag置位为MAC_EVT_TX_CAD_TIMEOUT
  - 根据上述cad_tx_detect_flag变化，判断信道空闲状态，并决定是否进行发射
  - 完成CAD-TX检测过程后，随机延时，重新随机选择频点和数据包长度，重复CAD-TX检测过程。

## 3. freq_hop_rx

- CAD功能被用于接收前的信道检测，用来检查当前信道是否存在有用信号，随后决定，继续接收，或是关闭接收，进入待机或休眠状态，以降低功耗。

- 软件应用参考
  - 设置待检测信道的基本参数，SF/BW/FREQ等
  - 打开CAD功能，设置CAD接收检测
  - 调用check_cad_rx_inactive();接口
  - 如果检测到有用信号，返回LEVEL_ACTIVE，可继续等待接收结果。接收超时或接收完成后，重新调用check_cad_rx_inactive();接口进行检测
  - 如果检测不到有用信号，则进入STB3待机状态，返回LEVEL_INACTIVE，随后随机切换频点，并设置CAD接收检测，重新调用check_cad_rx_inactive();接口进行检测
  
- TX代码流程图

  ![image-20240429170449752](picture\image-20240429170449752.png)

- RX代码流程图

![image-20240429170630892](picture\image-20240429170630892.png)

## 4. 特别注意

- freq_hop_tx设置前导码个数计算方法。

  多频点切换收发时，发射端需要根据不同的MCU和代码逻辑实测一下RX程序切换耗时，然后根据这个耗时计算TX发射端应当配置的前导码个数。例如，对于双频点切换的场景，测得程序切换耗时为T1，根据rf_get_chirp_time函数计算one_chirp_time，RF启动RX时有一个固定的启动耗时约360us。双频点场景极限情况下所需前导码示例表如下：

  ![image-20240426101019768](picture\image-20240426101019768.png)

  极限前导码个数计算时，假设当前发射端在发射FREQ1信号，接收端首次FREQ1（由于接收不全）未检测到有用信号，再次使用FREQ2（由于频点不对）未检测到有用信号，再次使用FREQ1可以检测到有用信号。一次CAD-RX的极限扫描时间为one_chirp_time*7，芯片启动RX需要耗时360us。FREQ1和FREQ2频点切换耗时需要实测T1。

  TX端额外前导码个数计算：（（one_chirp_time**7+360）*3+T1*2）/one_chirp_time。
  
  

