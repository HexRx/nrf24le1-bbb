#include "wiring.h"
#include <string.h>

void
wiring_init() 
{
}

uint8_t
wiring_write_then_read(uint8_t* out, uint16_t out_len, 
	               uint8_t* in, uint16_t in_len)
{
	uint8_t transfer_buf[out_len + in_len];
	unsigned int ret = 0;

	memset(transfer_buf, 0, out_len + in_len);
	
	if (NULL != out) {
		memcpy(transfer_buf, out, out_len);
		ret += out_len;
	}

	if (NULL != in) {
		ret += in_len;
	}

	memcpy(in, &transfer_buf[out_len], in_len);

	return ret;
}

void
wiring_set_gpio_value(uint8_t pin, uint8_t state)
{
}

void
wiring_destroy(void)
{
}
