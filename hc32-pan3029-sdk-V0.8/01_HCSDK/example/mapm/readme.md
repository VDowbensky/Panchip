# mapm说明

## 1. 功能说明
- 只有TX,RXmapm地址参数ADDR1相同时，RX才能进入mapm中断。
- 本次验证使用3个模组进行，一个发送TX,两个接收RX，验证代码共用同一工程。

## 2. 快速入门

- 烧录SDK工程到EVB测试板中，使用拨码开关切换发射端/接收端。通过拨码开关切换模组收发模式需复位测试板后才生效。
- 通过代码中宏ADDR_MATCH来选择RX与TXADDR2是否相 同，从而判断是否接收TX发送数据。两个RX，一个开启宏，一个关闭宏进行对比。
- 开启宏的RX端可以接收到TX发出的数据；未开启的无法接收。

## 3. 接收端接口说明

-  rf_set_mapm_on

  使能mapm模式，开mapm中断允许

- void rf_set_mapm_para

  设置mapm模式相关参数

- rf_set_mapm_addr

  配置mapmGroup中地址

## 4. 特别注意

- 无。
