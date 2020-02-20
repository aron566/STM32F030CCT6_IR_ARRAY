
#ifndef MODBUS_BYTES_H
#define MODBUS_BYTES_H

#include <stdint.h>

void modbus_U16_to_buffer( uint8_t *pBuffer, uint16_t value16 );
void modbus_U32_to_buffer( uint8_t *pBuffer, uint32_t value32 );

uint16_t modbus_buffer_to_U16( uint8_t *pBuffer );
uint32_t modbus_buffer_to_U32( uint8_t *pBuffer );

#endif