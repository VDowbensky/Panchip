# 量产烧录工具

​    为配合PAN-LINK2.0烧录PAN107x/PAN101x芯片程序工具。



### 1.硬件准备

​    预先将 PAN-LINK2.0 通过 MiniUSB 线连接到 PC 电脑。

​    如果 PAN-LINK2.0 固件程序不支持 PAN10xx 芯片烧录，则需要根据提示自动更新升级。

​    或按照帮助文档方法更新 PAN-LINK2.0 固件程序。



#### 1.1.PAN10xx 芯片烧录接线

​    <b><font color=red>注：</font></b> PAN-LINK2.0 接口的 <b><font color=red>VCC</font></b> 与 <b><font color=red>VIO</font></b>通过跳线帽<b><font color=red>短接</font></b>。

<center><small>表1-1 PAN-LINK2.0 烧录 PAN107x 芯片的接线表</small></center>

| PAN-LINK2.0 接口脚                | 连接  | PAN107x 芯片脚                    |
| --------------------------------- | ----- | --------------------------------- |
| <b><font color=red>VDD</font></b> | <---> | <b><font color=red>VDD</font></b> |
| <b><font color=red>GND</font></b> | <---> | <b><font color=red>GND</font></b> |
| **A1**                            | <---> | **RST**                           |
| **A3**                            | <---> | **P01**                           |
| **A4**                            | <---> | **P00**                           |



### 2.烧录步骤

​    1、在下载程序配置中的下载程序配置项右键点击加载程序，实现加载烧录程序功能。

​    2、通过点击擦除模式前面图标或右键选择更改烧录擦除模式。

​    3、根据需求选择设置其他下载配置。

​    4、选择下载模式，或直接默认下载到 PAN-LINK 后下载到芯片模式。

​    5、点击下载开始下载程序到芯片。



### 3.查看帮助文档

​    通过烧录工具的**帮助**->**查看帮助文档**或直接通过快捷键**F1**,打开查看帮助文档。

   PAN-LINK2.0 程序更新方法、以及烧录工具的详细使用说明都在帮助文档中有详述。



## 更新日志

### pan107x download tool V0.0.002

​    1、修复一些潜在问题。



### pan107x download tool V0.0.003

​    1、更新帮助文档。



### pan10xx download tool V0.0.004

​    1、更新修改工具名称一个相关文档为 PAN10xx，兼容支持 PAN107x 芯片与 PAN101x芯片的烧录功能。

​    2、更新 PAN-LINK 相关功能。

​    3、更新帮助文档。



### pan10xx download tool V0.0.005

​    1、修复功能->读取 PHY 问题。

​    2、更新 PAN-LINK 支持外部 AD1 IO 输出控制 RST 输出实现，控制芯片的 RST 脚功能。

​    3、更新下载加密信息功能，配合SDK发布的加密信息文件，实现芯片 Flash 程序加密。

​    4、修复烧录擦除芯片结果 log 显示错误问题。

​    5、修复下载特殊程序有失败的问题。



### pan10xx download tool V0.0.006

​    1、修复自定义下载配置模式保存配置文件，在下载加密信息使能时，在下载配置模式加载配置失败问题。



### pan10xx download tool V0.0.007

​    1、修改上位机下载 MAC 接口，改为默认下载到 INFO 区 0x100 地址位置。

​    2、更新帮助文档，对下载 MAC 的说明。



### pan10xx download tool V0.0.008

​    1、添加支持无 RST 脚芯片进入 ROM 模式下载程序功能。

​    2、添加支持下载 SWD Debug 加密芯片，通过密钥解密进行再次下载功能。

​    3、优化烧录加密信息写 efuse，写之前先进行读取校验，防止重复写入。



### pan10xx download tool V0.0.009

​    1、优化部分功能。

   

### pan10xx download tool V0.0.010

​    1、修复项目名输入限制超出 26 个字符，造成项目名长度超出 26 个字符下载失败问题。



### pan10xx download tool V0.0.011

​    1、优化 PAN-LINK 烧录。

​    2、修复上位机勾选芯片连接自动开始烧录功能无效的问题。



### pan10xx download tool V0.0.012

​    1、修复下载 SWD 加密项目，无法重复下载的问题。

​    2、添加 PAN-LINK KEY3 按键短按，跳到信息显示界面，读取芯片信息与芯片 MAC，以及项目程序 MAC 显示功能。

​    3、添加下载 SWD 加密密钥缓存，通过缓存密钥解密可以对芯片信息进行读取，直到 PAN-LINK 重启。



### pan10xx download tool V0.0.013

​    1、更新 PAN-LINK 拓展串口命令，支持量产烧录控制设置选择指定名称的离线项目程序作为当前项目程序，支持开始离线下载命令。



### pan10xx download tool V0.0.014

​    1、添加帮助菜单版本日志显示功能。



### pan10xx download tool V0.0.015

​    1、添加芯片版本信息校验功能。

​    2、优化上位机部分功能。