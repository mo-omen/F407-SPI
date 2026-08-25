#include <stdint.h>

volatile uint32_t *RCC_AHB1ENR = (uint32_t *)0x40023830;
volatile uint32_t *RCC_APB2ENR = (uint32_t *)0x40023844;

volatile uint32_t *GPIOA_MODER = (uint32_t *)0x40020000;
volatile uint32_t *GPIOA_AFRL = (uint32_t *)0x40020020;
volatile uint32_t *GPIOA_ODR = (uint32_t *)0x40020014;

volatile uint32_t *SPI1_CR1 = (uint32_t *)0x40013000;
volatile uint32_t *SPI1_SR = (uint32_t *)0x40013008;
volatile uint8_t *SPI1_DR = (uint8_t *)0x4001300C; // has to be 8 bit to match ADXL345's  SPI frame

void spi_drain_rx(void);
void spi_send_dummy(void);

int main(void) {

	*RCC_AHB1ENR |= (1 << 0);
	*RCC_APB2ENR |= (1 << 12);

	*GPIOA_ODR &= ~(1 << 4); 	// ODR RESET
	*GPIOA_MODER &= ~(0xFF << 8);
	*GPIOA_MODER |= ((1 << 8) | (1 << 11) | (1 << 13) | (1 << 15));
	*GPIOA_AFRL &= ~(0xFFF << 20);
	*GPIOA_AFRL |= ((5 << 20) | (5 << 24) | (5 << 28));
	*GPIOA_ODR |= (1 << 4);		// de-select slave

	*SPI1_CR1 |= (1 << 3);	// Clock Speed divider '4'
	*SPI1_CR1 |= (1 << 1);	// CLK polarity
	*SPI1_CR1 |= (1 << 0);	// CLK phase
	*SPI1_CR1 &= ~(1 << 11);	// 8-bit frame selection
	*SPI1_CR1 |= (1 << 8);	// internal slave select
	*SPI1_CR1 |= (1 << 9);	// Software slave management
	*SPI1_CR1 |= (1 << 2);	// Master Config
	*SPI1_CR1 |= (1 << 6);	// SPI enable


	*GPIOA_ODR &= ~(1 << 4);			// select slave
	while(!(*SPI1_SR & (1 << 1))) { }	// wait for TXE
	*SPI1_DR = 0b00101101;				// send command byte (write POWER_CTL)
	spi_drain_rx();
	while(!(*SPI1_SR & (1 << 1))) { }	// wait for TXE
	*SPI1_DR = 0b00001000;				// write a byte (POWER_CTL config)
	spi_drain_rx();
	*GPIOA_ODR |= (1 << 4);

	int16_t X = 0x00;
	int16_t Y= 0x00;
	int16_t Z = 0x00;


	while(1) {
		X = 0x00;
		Y = 0x00;
		Z = 0x00;

		*GPIOA_ODR &= ~(1 << 4);			// select slave

		while(!(*SPI1_SR & (1 << 1))) { }	// wait for TXE
		*SPI1_DR = 0b11110010;				// send command byte (read all consecutive axis addresses)

		spi_drain_rx();
		spi_send_dummy();

		while(!(*SPI1_SR & (1 << 0))) { }
		X |= (*SPI1_DR << 0);
		spi_send_dummy();

		while(!(*SPI1_SR & (1 << 0))) { }
		X |= (*SPI1_DR << 8);
		spi_send_dummy();

		while(!(*SPI1_SR & (1 << 0))) { }
		Y |= (*SPI1_DR << 0);
		spi_send_dummy();

		while(!(*SPI1_SR & (1 << 0))) { }
		Y |= (*SPI1_DR << 8);
		spi_send_dummy();

		while(!(*SPI1_SR & (1 << 0))) { }
		Z |= (*SPI1_DR << 0);
		spi_send_dummy();

		while(!(*SPI1_SR & (1 << 0))) { }
		Z |= (*SPI1_DR << 8);

		*GPIOA_ODR |= (1 << 4);				// de-select slave
	}

}

void spi_drain_rx(void) {
	int buffer_f;
	while(!(*SPI1_SR & (1 << 0))) { }	// wait for RXNE
	buffer_f = *SPI1_DR;

}

void spi_send_dummy(void) {
	while(!(*SPI1_SR & (1 << 1))) { }	// wait for TXE
	*SPI1_DR = 0x00;
}
