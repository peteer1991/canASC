/*
 * SpiUSBHost.h
 *
 *  Created on: May 13, 2009
 *      Author: dsk
 */

#ifndef SPIUSBHOST_H_
#define SPIUSBHOST_H_




#include "MAX3421E.h"


void Host_SPI_Write(BYTE reg, BYTE data);
unsigned short Host_SPI_Read(BYTE reg);
void Host_Write_N_Bytes(BYTE reg, int Bytes_num, BYTE *pdata);
void Host_Read_N_Bytes(BYTE reg, int Bytes_num, BYTE *prdata);
void Host_wr_HostReg(BYTE reg, BYTE val);

#define ABRT		1 << 3		/* SPI0 interrupt status */
#define MODF		1 << 4
#define ROVR		1 << 5
#define WCOL		1 << 6
#define SPIF		1 << 7


#endif /* SPIUSBHOST_H_ */
