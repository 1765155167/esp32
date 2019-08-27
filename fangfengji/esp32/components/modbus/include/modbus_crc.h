#ifndef MODBUS_CRC_H
#define MODBUS_CRC_H

#include "std_common.h"

//
// 		The CRC16 function
// 		pucFrame --> data for CRC calculation
//		usLen 	 --> length of data
 
uint16_t modbus_crc( uint8_t * pucFrame, uint16_t usLen );


#endif