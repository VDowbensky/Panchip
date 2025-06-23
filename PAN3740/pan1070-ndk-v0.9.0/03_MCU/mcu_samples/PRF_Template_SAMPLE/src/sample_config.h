//*** <<< Use Configuration Wizard in Context Menu >>> ***

#ifndef SAMPLE_CONFIG_H
#define SAMPLE_CONFIG_H


// <h> PAN_PRF Config
// <o> prf work mode select   <0=> PRF_MODE_NORMAL    <1=> PRF_MODE_ENHANCE    <2=> PRF_MODE_NORMAL_M1
#define PRF_WORK_MODE                   1
// <i> prf work mode select

// <o> prf chip mode select   <2=> PRF_CHIP_MODE_SEL_XN297    <3=> PRF_CHIP_MODE_SEL_NRF
#define PRF_CHIP_MODE                  2
// <i> prf chip mode select

// <o> prf trx mode select   <0=> PRF_TX_MODE    <1=> PRF_RX_MODE
#define PRF_TRX_SELECT                  0
// <i> prf trx mode select

// <o> prf phy select   <1=> PRF_PHY_1M    <2=> PRF_PHY_2M    <3=> PRF_PHY_CODED_S8    <4=> PRF_PHY_CODED    <5=> PRF_PHY_250K
#define PRF_PHY_SELECT                  1
// <i> prf phy select

// <o> prf crc select   <0=> PRF_CRC_SEL_NOCRC    <1=> PRF_CRC_SEL_CRC8    <2=> PRF_CRC_SEL_CRC16    <3=> PRF_CRC_SEL_CRC24
#define PRF_CRC                  2
// <i> prf crc select

// <o> prf scramble select   <0=> PRF_SRC_SEL_NOSRC    <1=> PRF_SRC_SEL_EN
#define PRF_SCR                  0
// <i> prf scramble select

// <o> prf mode config select   <1=> PRF_BLE_CONF    <2=> PRF_NRF_CONF    <3=> PRF_G_250k_CONF    <4=> PRF_B_250K_CONF
#define PRF_MODE_CONFIG                  1
// <i> prf mode config select

// <o> prf transfer type select   <0=> PRF_TRF_NORMAL    <1=> PRF_TRF_NRF52    <2=> PRF_TRF_B250K
#define PRF_TRANS_TYPE                  0
// <i> prf transfer type select

// <o> rx timeout
// <i> Default: 0
#define PRF_RX_TIMEOUT             5000

// <o> channel
// <i> Default: 2402
#define PRF_CHANNEL                2410

// <o> tx does not need to receive ack
// <0=> DISABLE
// <1=> ENABLE
#define PRF_TX_NO_REVCV_ACK        0

// <o> rx payload length
// <i> Default: 0
#define PRF_RX_PAYLOAD_LENGTH      0

// <o> sync word length
// <i> Default: 4
#define PRF_SYNC_WORD_LENGTH      4

// <o> crc area include sync word
// <0=> DISABLE
// <1=> ENABLE
#define PRF_CRC_INCLUDE_SYNC       0

// <o> white area include sync word
// <0=> DISABLE
// <1=> ENABLE
#define PRF_WHITE_INCLUDE_SYNC      0

// <o> pid mannual
// <0=> DISABLE
// <1=> ENABLE
#define PRF_PID_MANNUAL_EN      0

// <o> tx power(-40~9)
// <i> Default: 0
#define PRF_TX_POWER      0

// <o> prf pipe num   <0x1=> PRF_PIPE0   <0x2=> PRF_PIPE1  <0x4=> PRF_PIPE2  <0x8=> PRF_PIPE3  <0x10=> PRF_PIPE4  <0x20=> PRF_PIPE5  <0x40=> PRF_PIPE6  <0x80=> PRF_PIPE7  
#define PRF_PIPE_NUM                  (0x1)
// <i> prf pipe num

// </h>

#endif // SAMPLE_CONFIG_H

//*** <<< end of configuration section >>>    ***
