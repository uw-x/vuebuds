#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H_

void bleAdvertisingStart();
void bleInit(void);
void bleSendData(uint8_t * data, uint32_t length);
bool bleSendPacket(uint8_t * data, uint32_t length);
bool bleCanTransmit(void);
bool bleBufferHasSpace(uint16_t length);
uint32_t bleGetRingBufferBytesAvailable(void);
void blePushSequenceNumber(void);
void send(void);

void bleImuSendData(uint8_t * data, uint16_t length);
void bleImuResetBuffer(void);

void bleSetButtonPressed(bool pressed);

// ble_bytes_sent_counter from prev codebase
void bleSetPixelsSent(uint32_t value);
uint32_t bleGetPixelsSent(void);
ret_code_t bleDisconnect(void);
void bleService(void);

#endif