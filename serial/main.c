
#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "serial.h"

uint8_t line_buf[64];
uint8_t line_ptr;

int main(void)
{
	ser_init(9600UL);
    
	sei();
	
    while (1) 
    {
		ser_put_byte('>');
		ser_put_byte('>');
		
		line_ptr = 0;
		ser_get_line(line_buf);
		
		while (line_buf[line_ptr] == ' ') line_ptr++;
		switch (line_buf[line_ptr++])
		{
		case '?':
			ser_put_str("Hello world!\n");
			break;
		default:
			ser_put_str("\n\r");
			break;	
		}				
    }
}
