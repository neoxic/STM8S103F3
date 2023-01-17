/*
** Copyright (C) 2022-2023 Arseny Vakhrushev <arseny.vakhrushev@me.com>
**
** This firmware is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This firmware is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this firmware. If not, see <http://www.gnu.org/licenses/>.
*/

#include "common.h"
#include "serial.h"

uint16_t chv[14];

void initserial(void) {
	UART_BRR2 = 0x05;
	UART_BRR1 = 0x04; // 115200 baud @ 8Mhz clock
	UART_CR2 = 0x2c; // REN=1, TEN=1, RIEN=1 (enable RX/TX/interrupts)
	NESTED_IRQ(UART_RXIRQ); // Enable nested IRQ

	TIM4_PSCR = 0x07; // 62.5kHz
	TIM4_ARR = 0xe0; // 3.6ms
	TIM4_EGR = 0x01; // UG=1 (force update)
	TIM4_SR = 0x00; // Clear UEV after UG
	TIM4_IER = 0x01; // UIE=1 (enable interrupts)
}

int putchar(int c) { // STDOUT -> UART_TX (blocking)
	while (!(UART_SR & 0x80)); // TXE=0 (TX in progress)
	UART_DR = c;
	return 0;
}

// Single UART is used both for iBUS servo and telemetry data exchange in the following way:
// 1) Initially, UART is in full-duplex mode and is listening for servo data on the RX pin.
// 2) Upon receiving a servo update, UART goes into half-duplex mode and starts listening
//    for sensor requests on the TX pin.
// 3) Upon receiving a sensor request (and if a response is required), RX is turned off
//    because it is designed to receive what is being trasmitted by TX in half-duplex mode.
// 4) Upon transmitting the last byte, the TX handler waits for the transmission to complete (TC=1)
//    before turning RX back on.
// 5) UART reverts back to full-duplex mode after 3.6ms, and the cycle repeats.

static uint8_t tx[8], txp, txq;

static void send2(uint8_t p, uint16_t x) {
	uint8_t a = x, b = x >> 8;
	uint16_t u = 0xfff9 - p - a - b;
	tx[0] = 0x06;
	tx[1] = p;
	tx[2] = a;
	tx[3] = b;
	tx[4] = u;
	tx[5] = u >> 8;
	txp = 0;
	txq = 6;
}

static void send4(uint8_t p, uint32_t x) {
	uint8_t a = x, b = x >> 8, c = x >> 16, d = x >> 24;
	uint16_t u = 0xfff7 - p - a - b - c - d;
	tx[0] = 0x08;
	tx[1] = p;
	tx[2] = a;
	tx[3] = b;
	tx[4] = c;
	tx[5] = d;
	tx[6] = u;
	tx[7] = u >> 8;
	txp = 0;
	txq = 8;
}

void UART_TXE(void) __interrupt(UART_TXIRQ) {
	if (UART_CR2 & 0x40) { // TCIEN=1
		UART_CR2 = 0x2c; // REN=1, TEN=1, RIEN=1
		return;
	}
	UART_SR, UART_DR = tx[txp++]; // Clear TXE+TC
	if (txp != txq) return;
	UART_CR2 = 0x48; // TEN=1, TCIEN=1
}

void UART_RXNE(void) __interrupt(UART_RXIRQ) {
	static uint8_t a, b, c, d, m, n = 30;
	static uint16_t u;
	a = b;
	b = UART_DR; // Clear RXNE
	if (UART_CR5 & 0x08) { // iBUS sens
		if (m == 4 || ++m & 1) return;
		if (m == 4) { // End of chunk
			if (c != 4 || u != (a | b << 8)) return; // Sync lost
			m = 0;
			u = 0xffff;
			uint8_t i = (d & 0x0f) - 1; // Sensor index
			uint16_t t = sensortype(i);
			if (!t) return;
			switch (d & 0xf0) {
				case 0x80: // Probe
					tx[0] = c;
					tx[1] = d;
					tx[2] = a;
					tx[3] = b;
					txp = 0;
					txq = 4;
					break;
				case 0x90: // Type
					send2(d, t);
					break;
				case 0xa0: { // Value
					uint32_t v;
					if (!sensordata(i, &v)) return;
					switch (t >> 8) {
						case 0x02: // 2 bytes
							send2(d, v);
							break;
						case 0x04: // 4 bytes
							send4(d, v);
							break;
						default:
							return;
					}
					break;
				}
				default:
					return;
			}
			UART_CR2 = 0x88; // TEN=1, TIEN=1
			return;
		}
		c = a;
		d = b;
	} else { // iBUS servo
		if (a == 0x20 && b == 0x40) { // Sync
			n = 0;
			u = 0xff9f;
			return;
		}
		if (n == 30 || ++n & 1) return;
		uint16_t v = a | b << 8;
		if (n == 30) { // End of chunk
			if (u != v) return; // Sync lost
			update();
#ifndef DEBUG
			m = 0;
			u = 0xffff;
			UART_CR5 = 0x08; // HDSEL=1 (enable half-duplex)
			TIM4_CR1 = 0x09; // CEN=1, OPM=1 (enable counter, one-pulse mode)
#endif
			return;
		}
		chv[(n >> 1) - 1] = v & 0x0fff;
	}
	u -= a + b;
}

void TIM4_UIF(void) __interrupt(TIM4_UIRQ) {
	TIM4_SR = 0x00; // Clear interrupts
	UART_CR5 = 0x00; // Disable half-duplex
}
