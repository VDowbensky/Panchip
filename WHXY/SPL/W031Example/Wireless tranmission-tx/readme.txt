示例目的：
          W031的通信例程

硬件资源：
          1. CW32W031RxUx StarKit
          2. 时钟HSI
          3. 系统时钟设置为HSI时钟48MHz， PCLK、HCLK不分频，PCLK=HCLK=SysClk=48MHz
          4. 配置IO口和SPI,无线传输。

演示说明：
          实现两块W031的无线通信。
                    发送工程示例代码，使用连续发射模式发射

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
