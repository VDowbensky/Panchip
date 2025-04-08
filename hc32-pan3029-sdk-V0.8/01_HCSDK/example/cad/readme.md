# cad说明

## 1. 功能说明
- 芯片支持CAD-IRQ中断，开启CAD功能并进入Rx模式后，芯片会检测信道中是否存在ChirpIOT™信号，如果存在则将CAD-IRQ置高，外部MCU可以通过在一定时间内检测CAD-IRQ信号是否拉高来判断信道中是否存在ChirpIOT™信号。

## 2. cad_tx

- CAD功能可以被用于发射前的信道检测，以保证当前信道空闲，随后进行数据发射，避免无线信号碰撞干扰，提高通信成功率。
- 软件应用参考
  - 设置待检测信道的基本参数，SF/BW/FREQ等
  - 调用rf_cad_detect_start();接口。检测接口内设置cad_tx_detect_flag为MAC_EVT_TX_CAD_NONE，打开CAD功能，设置CAD接收检测，设置超时定时器
  - 如果触发CAD IO中断处理函数，则将cad_tx_detect_flag置位为MAC_EVT_TX_CAD_ACTIVE
  - 如果触发定时器超时回调函数，则将cad_tx_detect_flag置位为MAC_EVT_TX_CAD_TIMEOUT
  - 根据上述cad_tx_detect_flag变化，判断信道空闲状态，并决定是否进行发射。

## 3. cad_rx

- CAD功能被用于接收前的信道检测，用来检查当前信道是否存在有用信号，随后决定，继续接收，或是关闭接收，进入待机或休眠状态，以降低功耗。

- 软件应用参考
  - 设置待检测信道的基本参数，SF/BW/FREQ等
  - 打开CAD功能，设置CAD接收检测
  - 调用check_cad_rx_inactive();接口
  - 如果检测到有用信号，返回LEVEL_ACTIVE，可继续等待接收
  - 如果检测不到有用信号，则进入STB3待机状态，返回LEVEL_INACTIVE，用户可自行选择后续操作


## 4. 特别注意

- 在使用CAD功能时，需要根据应用场景配置rf_cad_on(uint8_t threshold, uint8_t chirps)函数中的传参，在使用完CAD功能后，建议调用rf_set_cad_off()函数，rf_set_cad_off()函数可以关闭CAD功能并将接收阈值恢复。
