/**************************************************************************/
/*!
  @file serial.c
  @mainpage SSD1306 Library
  @section intro Introduction
  
  @section author Author
  
  @section license License
  
 */
/**************************************************************************/

#define F_CPU 8000000UL

#include "serial.h"

static volatile uint8_t tx_buf[SER_BUF_SIZE];
static volatile uint8_t tx_read_ptr, tx_write_ptr, tx_cur_size;

static volatile uint8_t rx_buf[SER_BUF_SIZE];
static volatile uint8_t rx_read_ptr, rx_write_ptr, rx_cur_size;

void ser_init(uint32_t baud)
{	
	uint16_t ubrr_val = (F_CPU + 4*baud)/(8*baud) - 1; // calculate UBRR at 2x with rounding

	UCSR0B = 0;
	UBRR0H = (uint8_t) ubrr_val >> 8;
	UBRR0L = (uint8_t) ubrr_val;
	UCSR0A = 1<<U2X0;						  // Double Speed mode
	UCSR0B = 1<<RXCIE0 | 1<<RXEN0 | 1<<TXEN0; // enable RX + TX, enable RX interrupt
	UCSR0C = 1<<UCSZ01 | 1<<UCSZ00;			  // 8 bits, no parity, 1 stop bit
	
	DDRD |= 1<<PD1; // TXD on ATmega 48/88/168/328
}

uint8_t ser_bytes_available()
{
	return rx_cur_size;
}

void ser_put_byte(uint8_t b)
{
// 	while (!(UCSR0A & 1<<UDRE0));
// 	UDR0 = b;

	while (tx_cur_size >= SER_BUF_SIZE); // wait for space in TX buffer
	
	tx_buf[tx_write_ptr] = b;
	cli();
	tx_cur_size++;
	UCSR0B = 1<<RXCIE0 | 1<<UDRIE0 | 1<<RXEN0 | 1<<TXEN0; // enable UDRE interrupt
	sei();
	tx_write_ptr = (tx_write_ptr + 1) & (SER_BUF_SIZE - 1);
}

void ser_get_byte(uint8_t *b)
{
// 	while(!(UCSR0A & 1<<RXC0));
// 	return UDR0;

	while (rx_cur_size == 0); // wait for data in RX buffer
	
	*b = rx_buf[rx_read_ptr];	
	cli();
	rx_cur_size--;
	sei();
	rx_read_ptr = (rx_read_ptr + 1) & (SER_BUF_SIZE - 1);
}

uint8_t ser_get_line(uint8_t *line_buf)
{
	uint8_t b;
	uint8_t line_ptr = 0;
	
	while (1)
	{
		ser_get_byte(&b);
		
		if (b == '\r' || b == '\n') // return/newline 
		{
			break;
		}
		else if (b == '\b' && line_ptr > 0)	// backspace
		{
			line_ptr--;
			ser_put_byte(b);	
		}
		else 
		{
			line_buf[line_ptr++] = b;
			if (ECHO) ser_put_byte(b);	
		}		
	}
	
	return line_ptr;	
}

void ser_put_str(char *str)
{
	while (*str++) ser_put_byte(*str);	
}

ISR(USART_UDRE_vect) // USART Data Register Empty interrupt - ready to send new byte
{
	if (tx_cur_size > 0) // TX buffer isn't empty
	{
		tx_cur_size--;										  // decrement size counter
		UDR0 = tx_buf[tx_read_ptr];							  // start sending new byte
		tx_read_ptr = (tx_read_ptr + 1) & (SER_BUF_SIZE - 1); // increment read pointer (wraparound)
	}
	else
	{
		UCSR0B = 1<<RXCIE0 | 1<<RXEN0 | 1<<TXEN0; // disable UDRE interrupt
	}
}

ISR(USART_RX_vect) // USART RX interrupt - new byte received
{
	uint8_t b = UDR0; // get byte immediately
	
	if (rx_cur_size < SER_BUF_SIZE) // RX buffer isn't full
	{
		rx_cur_size++;											// increment size counter
		rx_buf[rx_write_ptr] = b;								// write new byte into buffer
		rx_write_ptr = (rx_write_ptr + 1) & (SER_BUF_SIZE - 1); // increment write pointer (wraparound)		
	}	
}
