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

void initsensor(void) {
	ADC_TDR = 0x18; // Disable Schmitt trigger for channels 3,4
	ADC_CR1 = 0x01; // ADON=1 (power on)
	ADC_CR2 = 0x08; // ALIGN=1 (right alignment)
}

uint16_t sensortype(uint8_t i) {
	if (i > 1) return 0;
	static const uint16_t types[] = {0x201, 0x203};
	return types[i];
}

uint8_t sensordata(uint8_t i, uint32_t *v) {
	if (i > 1) return 0;
	static const uint8_t chnums[] = {3, 4};
	static uint16_t b[2][64];
	static uint8_t p[2], q[2];
	uint8_t ch = chnums[i];
	uint16_t s = 0;
	for (uint8_t j = 0; j < 64; ++j) {
		s += b[i][j];
		if (j < 48) continue;
		ADC_CSR = ch; // EOC=0, CH=ch (reset channel)
		ADC_CR1 = 0x01; // ADON=1 (start conversion)
		while (!(ADC_CSR & 0x80)); // EOC=0 (conversion in progress)
	}
	b[i][p[i]++ & 63] = ADC_DR; // Ignore first 15 conversions (increase sampling time)
	uint8_t n = q[i];
	if (n < 64) n = q[i] = p[i];
	*v = sensor(i, s / n);
	return 1;
}
