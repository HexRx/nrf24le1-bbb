
#include "wiring.h"

#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <linux/spi/spidev.h>

static int fd = -1;

bool
wiring_init(const char *device)
{
	int speed = 4000000;
	uint8_t mode = SPI_MODE_0;

	fd = open(device, O_RDWR);
	if (fd < 0) {
		printf("Failed to open the %s\n", device);
		return 0;
	}

	if (ioctl(fd, SPI_IOC_WR_MODE, &mode)<0)   {
		perror("can't set spi mode");
		goto exit;
	}

	if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed)<0) {
		perror("can't set max speed hz");
		goto exit;
	}

	return 1;

exit:
	close(fd);
	return 0;
}

// ugly.

uint8_t
wiring_write_then_read(uint8_t* out, uint16_t out_len, 
	               uint8_t* in, uint16_t in_len)
{
	uint8_t tx_buf[out_len + in_len];
	uint8_t rx_buf[out_len + in_len];

	unsigned int ret = 0;

	memset(tx_buf, 0, out_len + in_len);
	memset(rx_buf, 0, out_len + in_len);

	if (NULL != out) {
		memcpy(tx_buf, out, out_len);
		ret += out_len;
	}

	if (NULL != in) {
		ret += in_len;
	}

	struct spi_ioc_transfer xfer[2];
	memset(xfer, 0, sizeof(xfer));

	xfer[0].tx_buf = (unsigned long)&tx_buf[0];
	xfer[0].rx_buf = (unsigned long)&rx_buf[0];
	xfer[0].len = ret;
	xfer[0].speed_hz = 2500000;
	xfer[0].bits_per_word = 8;

	xfer[1].rx_buf = (unsigned long)in;
	xfer[1].len = in_len;
	xfer[1].speed_hz = 2500000;
	xfer[1].bits_per_word = 8;

	//int status = ioctl(fd, SPI_IOC_MESSAGE(in_len > 0 ? 2 : 1), &xfer[0]);
	int status = ioctl(fd, SPI_IOC_MESSAGE(1), &xfer[0]);
	if (status < 0) {
		perror("SPI_IOC_MESSAGE");
		return 0;
	}

	memcpy(in, &rx_buf[out_len], in_len);

	return ret;
}

void
wiring_set_gpio_value(uint8_t pin, uint8_t state)
{
}

void
wiring_destroy(void)
{
	close(fd);
}
