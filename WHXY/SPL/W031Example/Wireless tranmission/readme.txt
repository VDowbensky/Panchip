示例目的：
          W031的通信例程

硬件资源：
          1. CW32W031RxUx StarKit
          2. 时钟HSI
          3. 系统时钟设置为HSI时钟48MHz， PCLK、HCLK不分频，PCLK=HCLK=SysClk=48MHz
          4. 配置IO口和SPI,无线传输。

演示说明：
          实现两块W031的无线通信。
          key6  功能键，没按下的时候其他键是第一功能，按下时按下其他键是第二功能。
          key5  第一功能   清除收发包统计计数         第二功能   LNA高低切换（+代表高增益，-代表低增益）
          key4  第一功能   开始发送  停止发送          第二功能   切换频点，屏幕FQ显示，单位0.1MHz。
          key3  第一功能   切换ABC三种发射模式      第二功能  切换码率coderate 
          key2  第一功能   设置BW                          第二功能，设置PW功率值
          key1  第一功能   设置SF，可以设置的        第二功能，设置payload长度

使用说明：
+ EWARM
          1. 打开project.eww文件
          2. 编译所有文件：Project->Rebuild all
          3. 载入工程镜像：Project->Debug
          4. 运行程序：Debug->Go(F5)

+ MDK-ARM
          1. 打开project.uvproj文件
          2. 编译所有文件：Project->Rebuild all target files
          3. 载入工程镜像：Debug->Start/Stop Debug Session
          4. 运行程序：Debug->Run(F5)
