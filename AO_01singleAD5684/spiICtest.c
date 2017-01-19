/** SPI C Transfer Example, Written by Derek Molloy (www.derekmolloy.ie)
* for the book Exploring BeagleBone. Based on the spidev_test.c code
* example at www.kernel.org */

#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<stdint.h>
#include<linux/spi/spidev.h>

#define SPI_PATH "/dev/spidev1.0"
void sendCommand(uint8_t comand, uint8_t address, uint16_t DACregister, uint8_t payload[]);
int transfer(int fd, unsigned char send[], unsigned char receive[], int length, uint32_t speed);

void printArray (int arrayLen, uint8_t array[]);

int main()
{
unsigned int fd; // file handle and loop counter
uint8_t bits = 24, mode = 3; // 8-bits per word, SPI mode 3
uint32_t speed = 100000; // Speed is Hz

// The following calls set up the SPI bus properties
if ((fd = open(SPI_PATH, O_RDWR))<0){
	perror("SPI Error: Can't open device.");
	return -1;
}
if (ioctl(fd, SPI_IOC_WR_MODE, &mode)==-1){
	perror("SPI: Can't set SPI mode.");
	return -1;
}
if (ioctl(fd, SPI_IOC_RD_MODE, &mode)==-1){
	perror("SPI: Can't get SPI mode.");
	return -1;
}
if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits)==-1){
	perror("SPI: Can't set bits per word.");
	return -1;
}
if (ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits)==-1){
	perror("SPI: Can't get bits per word.");
	return -1;
}
if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed)==-1){
	perror("SPI: Can't set max speed HZ");
	return -1;
}
if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed)==-1){
	perror("SPI: Can't get max speed HZ.");
	return -1;
}
// Check that the properties have been set
printf("SPI Mode is: %d\n", mode);
printf("SPI Bits is: %d\n", bits);
printf("SPI Speed is: %d\n", speed);
uint8_t txA[] = {0x3F, 0x7F, 0xFF};
uint8_t rxA[3];
int length = 3;
uint16_t valuei = 0;

sendCommand( 3,  2, 255, txA);
printArray (length, txA);
while (1)
{
	valuei = (valuei > 4095) ? 0 : (valuei + 100);
	sendCommand( 1,  1, valuei, txA);
	if (transfer(fd, (unsigned char*) &txA, (unsigned char*) &rxA, length, speed)==-1){
		perror("Failed to update the display");
		return -1;
	}
	
	printArray (length, txA);
	printArray (length, rxA);
	fflush(stdout); // need to flush the output, as no \n
	usleep(1000); // sleep for 100ms each loop

	sendCommand( 2,  1, valuei, txA);
	if (transfer(fd, (unsigned char*) &txA, (unsigned char*) &rxA, length, speed)==-1){
		perror("Failed to update the display");
		return -1;
	}
	

	usleep(10000); // sleep for 100ms each loop
}

close(fd); // close the file
return 0;
}


int transfer(int fd, unsigned char send[], unsigned char receive[], int length, uint32_t speed)
{
	struct spi_ioc_transfer transfer; // the transfer structure

	transfer.tx_buf = (unsigned long) send; // the buffer for sending data
	transfer.rx_buf = (unsigned long) receive; // the buffer for receiving data
	transfer.len = length; // the length of buffer
	transfer.speed_hz = speed; // the speed in Hz
	transfer.bits_per_word = 8; // bits per word
	transfer.delay_usecs = 0; // delay in us
	// send the SPI message (all of the above fields, inc. buffers)
	int status = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);

	if (status < 0) {
		perror("SPI: SPI_IOC_MESSAGE Failed");
		return -1;
	}
	return status;
}

void printArray (int arrayLen, uint8_t array[])
{
	int i;
	for (i = 0; i < arrayLen; i++){
		if ((!i % 6))
			puts("");
		printf("%.2X ", array[i]);
	}
	puts("");
}

void sendCommand(uint8_t comand, uint8_t address, uint16_t DACregister, uint8_t payload[])
{
	uint8_t ln, /* low half-byte - low nibble */ 
		hn; /* high half-byte - high nibble */
	uint16_t dac;

	hn = (comand & 0x0F) << 4;
	ln = (address & 0x0F);

	payload[0] = hn | ln;

	dac = (DACregister > 4095) ? 4095 : DACregister; /* 12 bit DAC */
	payload[1] = (uint8_t)(dac >> 4);	
	payload[2] = (uint8_t)(dac << 4);
}
