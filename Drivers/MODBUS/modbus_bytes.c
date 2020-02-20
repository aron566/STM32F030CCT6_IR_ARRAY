#include "modbus_bytes.h"
#include <stdint.h>
/*******************************************************************************
* Function Name  : modbus_U16_to_buffer
* Description    : 先发高字节，再发低字节
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void modbus_U16_to_buffer( uint8_t *pBuffer, uint16_t value16 )
{
    pBuffer[ 0 ] = (uint8_t)( value16 >> 8 );
    pBuffer[ 1 ] = (uint8_t)( value16 & 0xFF );
}

/*******************************************************************************
* Function Name  : modbus_buffer_to_U16
* Description    : 先传的高字节，后传的低字节
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t modbus_buffer_to_U16( uint8_t *pBuffer )
{
    uint16_t return_value = 0;

    return_value = pBuffer[ 0 ];
    return_value <<= 8;
    return_value += pBuffer[ 1 ];

    return return_value;
}

/*******************************************************************************
* Function Name  : modbus_U32_to_buffer
* Description    : 先发高字节，再发低字节
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void modbus_U32_to_buffer( uint8_t *pBuffer, uint32_t value32 )
{
    pBuffer[ 0 ] = (uint8_t)( value32 >> 24 );
    pBuffer[ 1 ] = (uint8_t)( value32 >> 16 );
    pBuffer[ 2 ] = (uint8_t)( value32 >> 8 );
    pBuffer[ 3 ] = (uint8_t)( value32 & 0xFF );
}
/*******************************************************************************
* Function Name  : modbus_buffer_to_U16
* Description    : 先传的高字节，后传的低字节
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint32_t modbus_buffer_to_U32( uint8_t *pBuffer )
{
    uint32_t return_value = 0;

    return_value = pBuffer[ 0 ];
    return_value <<= 8;
    return_value += pBuffer[ 1 ];
    return_value <<= 8;
    return_value += pBuffer[ 2 ];
    return_value <<= 8;
    return_value += pBuffer[ 3 ];

    return return_value;
}


