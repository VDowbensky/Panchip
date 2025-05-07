# auto_sf说明

## 1. 功能说明
- 芯片为实现轻量化网关设备，提供智能搜索功能。可实现在接收时智能化识别信道中的SF参数，达到接收不同SF信号数据的目的。
- 智能搜索功能使用时，接收端需要配置SF搜索范围，发射端需要根据不同SF配置不同的前导码个数。

## 2. 快速入门

- 设置main.c文件中的sf_range[]和chirp_counts[]，sf_range为智能搜索SF范围，chirp_counts用来存储对应的前导码个数。
- 烧录SDK工程到EVB测试板中，使用拨码开关切换发射端/接收端（OLED屏幕显示TX/RX）。通过拨码开关切换模组收发模式需复位测试板后才生效。

## 3. 接口说明

- int calculate_chirp_count(int sf_range[], int size, int chirp_counts[]);

  发射端配置。根据sf_range和size计算智能搜索需要的前导码个数，并将前导码个数存储到chirp_counts。

- RF_Err_t rf_set_auto_sf_tx_preamble(int sf, int sf_range[], int size, int chirp_counts[]);

  发射端配置。每次修改SF时，需要调用此接口，接口会根据SF设置对应的前导码个数。

- RF_Err_t rf_set_auto_sf_rx_on(int sf_range[], int size);

  接收端配置。进入接收前，设置智能搜索SF搜索范围。

- RF_Err_t rf_set_auto_sf_rx_off(void);

  发射端，接收端配置。关闭智能搜索功能，恢复默认配置。

## 4. 特别注意

- SDK中对低速率的参数时会自动开启LDR，为了确保收发双方都使用相同的LDR配置，所以在使用智能搜索功能时，收发双方都需要关闭LDR。
- 智能搜索功能支持根据实际需求配置不同的SF搜索范围，建议选择连续的SF值进行智能搜索，例如SF7~9。
