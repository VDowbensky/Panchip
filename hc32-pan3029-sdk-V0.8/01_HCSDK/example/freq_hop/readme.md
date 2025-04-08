# freq_hop description

## 1. Functional description
- The chip supports CAD-IRQ interrupt. After turning on the CAD function and entering the Rx mode, the chip will detect whether there is a ChirpIOT™ signal in the channel. If there is, the CAD-IRQ will be set high. The external MCU can determine whether there is a ChirpIOT™ signal in the channel by detecting whether the CAD-IRQ signal is pulled high within a certain period of time.
- Use the CAD function to implement frequency hopping communication through software logic.
- Frequency hopping function implementation logic (2 frequency points)
- The receiving end switches between 2 frequency points. After switching the frequency point, CAD-RX detection is performed. If a useful signal is detected, it continues to receive; if no useful signal is detected, it switches to another frequency point and performs CAD-RX detection again.
- The transmitting end configures the appropriate number of preamble codes according to the actual situation. CAD-TX channel detection is performed before transmission. If the channel is idle, data is transmitted; if the channel is busy, CAD-TX detection is performed again after random delay backoff.

## 2. freq_hop_tx

- The CAD function can be used for channel detection before transmission to ensure that the current channel is idle, and then transmit data to avoid wireless signal collision interference and improve communication success rate.
- Software application reference
- Set the basic parameters of the channel to be detected, such as SF/BW/FREQ, etc.
- Set the appropriate number of preamble codes, and refer to the end of the article for calculation method
- Randomly select frequency points and data packet lengths
- Call rf_cad_detect_start(); detection interface. Perform CAD-TX detection.
- Set cad_tx_detect_flag to MAC_EVT_TX_CAD_NONE in the detection interface, turn on the CAD function, set CAD receive detection, and set the timeout timer
- If the CAD IO interrupt processing function is triggered, set cad_tx_detect_flag to MAC_EVT_TX_CAD_ACTIVE
- If the timer timeout callback function is triggered, set cad_tx_detect_flag to MAC_EVT_TX_CAD_TIMEOUT
- According to the above cad_tx_detect_flag changes, determine the channel idle state and decide whether to transmit
- After completing the CAD-TX detection process, randomly delay, re-randomly select the frequency point and data packet length, and repeat the CAD-TX detection process.

## 3. freq_hop_rx

- The CAD function is used for channel detection before receiving to check whether there is a useful signal in the current channel, and then decide to continue receiving, or turn off receiving, enter standby or sleep state to reduce power consumption.

- Software application reference
- Set the basic parameters of the channel to be detected, such as SF/BW/FREQ, etc.
- Enable the CAD function and set the CAD receiving detection
- Call the check_cad_rx_inactive() interface
- If a useful signal is detected, return LEVEL_ACTIVE and continue to wait for the receiving result. After receiving timeout or receiving is completed, call the check_cad_rx_inactive() interface again for detection
- If no useful signal is detected, enter the STB3 standby state, return to LEVEL_INACTIVE, then randomly switch the frequency point, set the CAD receiving detection, and call the check_cad_rx_inactive() interface again for detection

- TX code flow chart

![image-20240429170449752](picture\image-20240429170449752.png)

- RX code flow chart

![image-20240429170630892](picture\image-20240429170630892.png)

## 4. Special attention

- freq_hop_tx sets the calculation method of the number of leading codes.

When switching between multiple frequencies, the transmitter needs to measure the RX program switching time according to different MCUs and code logic, and then calculate the number of preambles that the TX transmitter should configure based on this time. For example, for the dual-frequency switching scenario, the program switching time is measured to be T1. According to the rf_get_chirp_time function to calculate one_chirp_time, there is a fixed startup time of about 360us when the RF starts RX. The example table of the required preambles in the extreme case of the dual-frequency scenario is as follows:

![image-20240426101019768](picture\image-20240426101019768.png)

When calculating the number of extreme preambles, it is assumed that the current transmitter is transmitting the FREQ1 signal, and the receiver does not detect a useful signal for the first time using FREQ1 (due to incomplete reception), and does not detect a useful signal using FREQ2 (due to incorrect frequency), and can detect a useful signal using FREQ1 again. The maximum scanning time of a CAD-RX is one_chirp_time*7, and it takes 360us for the chip to start RX. The switching time between FREQ1 and FREQ2 requires the actual measurement of T1.

Calculation of the number of additional preamble codes on the TX end: ((one_chirp_time**7+360)*3+T1*2)/one_chirp_time.