# tx说明

## 1. 功能说明
- 演示芯片连续发射功能。与rx示例搭配使用。

## 2. 快速入门

- 烧录SDK工程到EVB测试板中，使用USB Mini-B 电源线连接电脑，使用电脑串口助手观察结果，串口波特率115200。
- 串口打印：Tx cnt xxxx，每次发送cnt递增，理论与rx端接收cnt保持一致。

- 代码流程图

  ![image-20240429165842872](C:\Users\panchip\Desktop\SDK\sdknew\bbs-hc32-pan3029-sdk\01_HCSDK\example\tx\picture\image-20240429165842872.png)

## 3.特别注意

- 连续发射模式中，发射完成后，如果不退出发射状态（切换至standby3模式可退出发射状态），那么芯片的工作电流会一直保持为发射电流。
