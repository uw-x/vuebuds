#ifndef CAMERA_H_
#define CAMERA_H_

void cameraInit(void);
void cameraDeInit(void);
void cameraCaptureFrame(void);
void cameraStartStream(void);
uint32_t cameraGetFrameBuffer(uint8_t **frame);
uint32_t cameraGetLines(uint8_t **lines);
uint32_t cameraGetBytesReceived(void);
void cameraReadyForMoreData(void);
void cameraReadyNextFrame(void);
void cameraEnableStandbyMode(bool);

#endif