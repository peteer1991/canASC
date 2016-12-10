/*
 * UsbHost.h
 *
 *  Created on: May 14, 2009
 *      Author: dsk
 */

#ifndef USBHOST_H_
#define USBHOST_H_

void vUsbTask(void);
void detect_device(void);
void enumerate_device(void);
BYTE Send_Packet(BYTE token,BYTE endpoint);
void Reset_Host(void);
BYTE CTL_Read(BYTE *pSUD);
BYTE IN_Transfer(BYTE endpoint,WORD INbytes);
BYTE CTL_Write_ND(BYTE *pSUD);
void initialize_ARM_Interrupts(void);
void wait_for_disconnect(void);
BYTE IN_Transfer(BYTE endpoint,WORD INbytes);
void INT3421(void);
void tc0(void);
void tc1(void);
void vStartUSBTasks( unsigned portBASE_TYPE uxPriority );




#endif /* USBHOST_H_ */
