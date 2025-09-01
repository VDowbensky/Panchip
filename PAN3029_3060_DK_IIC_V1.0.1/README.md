# PAN3029_3060 SDK

# PAN3029_3060 开发套件

PAN3029 开发套件为PANCHIP无线通信芯片开发提供完整的资源支持。本内部版本包含最新的开发工具、技术文档和参考实现。

## 目录结构

```plaintext
├── 01_SDK/          # SDK开发包，包含外设驱动和示例工程
├── 02_HDK/          # 硬件开发包，包含原理图和PCB设计文件
├── 03_DOC/          # 技术文档和使用指南
├── 04_TOOLS/        # 开发工具，包括代码生成工具和调试工具
├── README.md        # 项目说明文档
├── CHANGELOG.md     # 版本更新日志
└── VERSION          # 版本信息
```



## 快速入门指南

### 1. 开始使用

- 从[PAN3029 Development Kit 文档中心](https://docs.panchip.com/pan3029dk-doc)下载最新版本开发套件（PAN3029_3060_DK_Vx.x.x.zip）

### 2. 开发流程

1. **环境搭建**
   - 查看`03_DOC/HC32F460_DOCS/UM_小华半导体MCU开发环境使用_Rev1.1.pdf`了解开发环境要求
   - 安装必需的开发工具，配置编译环境(KEIL&小华HC32F460)
   - 连接并烧录PAN3029评估板，观察程序运行结果
2. **示例工程**
   - 进入`01_SDK/example/01_normal_trx`目录
   - 运行tx和rx演示工程进行快速验证
3. **代码移植**
   - `《PAN3029_3060_驱动接口使用说明_v1.6.pdf》`

### 3. 高级功能

更多高级功能开发指南请参考如下文档：

- `《PAN3029_3060_AT指令说明_v1.2.pdf》`
- `《PAN3029_3060_CAD应用指南_v1.4.pdf》`
- `《PAN3029_3060_MAPM应用指南_v1.4.pdf》`
- `《PAN3029_3060_自动识别SF应用指南_v1.2.pdf》`
- `《PAN3029_3060_评估板使用说明_v1.4.pdf》`

## 相关资源

- [文档中心](https://wiki.panchip.com)
- [技术论坛](https://bbs.panchip.com)
- [官方网站](https://www.panchip.com)

## 技术支持

如需技术支持和交流，请访问我们的[技术论坛](https://bbs.panchip.com)。
