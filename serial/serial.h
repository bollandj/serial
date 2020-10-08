#ifndef SERIAL_H_
#define SERIAL_H_

#include <avr/io.h>
#include <avr/interrupt.h>

#define SER_BUF_SIZE  64 // must be a power of 2 and not greater than 256
#define ECHO 1

extern void ser_init(uint32_t baud);
extern uint8_t ser_bytes_available();
extern void ser_put_byte(uint8_t b);
extern void ser_get_byte(uint8_t *b);
extern uint8_t ser_get_line(uint8_t *line_buf);
extern void ser_put_str(char *str);

#endif /* SERIAL_H_ */