void RadioSetTxConfig(RadioModems_t modem, int8_t power, uint32_t fdev,
					  uint32_t bandwidth, uint32_t datarate,
					  uint8_t coderate, uint16_t preambleLen,
					  bool fixLen, bool crcOn, bool FreqHopOn,
					  uint8_t HopPeriod, bool iqInverted, uint32_t timeout);
            
            
void RadioSetRxConfig(RadioModems_t modem, uint32_t bandwidth,
					  uint32_t datarate, uint8_t coderate,
					  uint32_t bandwidthAfc, uint16_t preambleLen,
					  uint16_t symbTimeout, bool fixLen,
					  uint8_t payloadLen,
					  bool crcOn, bool FreqHopOn, uint8_t HopPeriod,
					  bool iqInverted, bool rxContinuous);