# atcmd说明

## 1. AT+RF_INIT\r\n

- 功能：芯片初始化

- 参数：无
- 回复：OK/ERROR

## 2. AT+DEEPSLEEPMODE\r\n

- 功能：配置进入deepsleep模式

- 参数：无
- 回复：OK/ERROR

## 3. AT+SLEEPMODE\r\n

- 功能：配置进入sleep模式

- 参数：无
- 回复：OK/ERROR

## 4. AT+STB1MODE\r\n

- 功能：配置进入stb1模式

- 参数：无
- 回复：OK/ERROR

## 5. AT+STB2MODE\r\n

- 功能：配置进入stb2模式

- 参数：无
- 回复：OK/ERROR

## 6. AT+STB3MODE\r\n

- 功能：配置进入stb3模式

- 参数：无
- 回复：OK/ERROR

## 7. AT+RF_PARA?\r\n

- 功能：配置芯片射频参数
- 参数：`<freq>,<cr>,<bw>,<sf>,<txp>,<crc>,<ldr>,<modemMode>,<preamble>,<dcdc>`
  - freq：信道频率值（Hz）
  - cr：编码率（取值范围1-4，对应1=4/5，2=4/6，3=4/7，4=4/8）
  - bw：信道带宽（取值范围6-9，对应6=62.5KHz，7=125KHz，8=250KHz，9=500KHz）
  - sf：扩频因子（取值范围5-12）
  - txp：功率值（取值范围1-23）
  - crc：校验（取值范围0-1，0: 关闭, 1: 打开）
  - ldr：低速率模式（取值范围0-1，0: 关闭, 1: 打开）
  - modemMode：波形模式配置（取值范围1-2，1: 常规模式, 2: 特殊模式）
  - preamble：前导码个数（取值范围8-65535）
  - dcdc：DCDC模式（取值范围0-1，0: 关闭, 1: 打开）
- 回复：OK/ERROR

## 8. AT+FREQ=xx\r\n

- 功能：设置频率值
- 参数：信道频率值（Hz）
- 回复：OK/ERROR

## 9. AT+CR=x\r\n

- 功能：编码率
- 参数：取值范围1-4，对应1=4/5，2=4/6，3=4/7，4=4/8
- 回复：OK/ERROR

## 10. AT+BW=x\r\n

- 功能：设置信道带宽
- 参数：取值范围6-9，对应6=62.5KHz，7=125KHz，8=250KHz，9=500KHz
- 回复：OK/ERROR

## 11. AT+SF=x\r\n

- 功能：设置扩频因子
- 参数：取值范围5-12
- 回复：OK/ERROR

## 12. AT+PWR=x\r\n

- 功能：设置功率值
- 参数：取值范围1-23
- 回复：OK/ERROR

## 13. AT+CRC=x\r\n

- 功能：设置校验
- 参数：取值范围0-1，0: 关闭, 1: 打开
- 回复：OK/ERROR

## 14. AT+LDR=x\r\n

- 功能：设置低速率模式
- 参数：取值范围0-1，0: 关闭, 1: 打开
- 回复：OK/ERROR

## 15. AT+MODEMMODE=x\r\n

- 功能：设置波形模式配置
- 参数：取值范围1-2，1: 常规模式, 2: 特殊模式
- 回复：OK/ERROR

## 16. AT+PREAMLEN=x\r\n

- 功能：设置前导码个数
- 参数：取值范围8-65535
- 回复：OK/ERROR

## 17. AT+DCDC=x\r\n

- 功能：设置DCDC模式
- 参数：取值范围0-1，0: 关闭, 1: 打开
- 回复：OK/ERROR

## 18. AT+RF_CARRYWAVE=x\r\n

- 功能：设置单载波测试
- 参数：取值范围0-1，0: 关闭, 1: 打开
- 回复：OK/ERROR

## 19. AT+RF_RXMODE=x\r\n

- 功能：设置进入接收
- 参数：超时时间（ms），取值范围1000-1000000
- 回复：OK/ERROR

## 19. AT+RF_TXMODE=x\r\n

- 功能：设置开始发射
- 参数：`<payloadlen>,<count>,<interval>`
  - payloadlen：发射包长（字节）
  - count：发射次数
  - interval：两包之间的发射间隔（ms）


***注：以上参数均为十进制***

