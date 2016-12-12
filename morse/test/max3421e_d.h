
/*
 * max3421e.h
 *
 * Created: 2016-10-07 23:56:59
 *  Author: peter
 */ 
#define  byte uint8_t
void USBH_SpiInit(void);

/**
 * Write data to a register over SPIF
 * @param reg uint8_t  - the register you want to write to
 * @param data uint8_t - the register you want to read from
 * @return void
 */
void USBH_SpiWrite(uint8_t reg, uint8_t data);
/**
 * Read a byte from a register.
 * @param reg uint8_t  - the register you want to write to
 * @return char - date read from SPIF.DATA
 */
char USBH_SpiRead(uint8_t reg);
void USBH_powerOn();
byte Task( void );
byte IntHandler();
void Usb_Task();
