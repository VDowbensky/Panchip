# deepsleep description

## 1. Functional description

- The chip supports entering sleep mode.

## 2. Quick start

- sleep_tx demonstrates that the chip periodically transmits data. After the chip transmits data, it enters sleep mode. After waking up, it can directly transmit data.

- sleep_rx demonstrates chip receiving companion test.

## 3. Interface description

- rf_sleep();

Set the chip to enter sleep mode. After entering sleep mode, the chip register configuration is saved.

- rf_sleep_wakeup();

Set the chip to wake up from sleep mode.

## 4. Special attention

- None
