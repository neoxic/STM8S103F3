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

#define CH1_TRIM -50 // Bucket
#define CH2_TRIM 50 // Boom
#define CH3_TRIM -80 // Stick

#define VALVE_MIN 220 // Still closed
#define VALVE_MAX 280 // Fully open
#define VALVE_MUL 100 // Input multiplier (%)

#define PUMP_MIN 60 // Minimum duty
#define PUMP_MAX 340 // Maximum duty
#define PUMP_LIM 20 // Acceleration limit

#define VOLT1 3329 // mV
#define VOLT2 3662 // xx.xxV = VOLT1*(R1+R2)/R2

static int16_t input1(uint16_t t, uint16_t *u, int16_t x) {
	*u = t + x;
	t = t < 1500 ? 1500 - t : t - 1500;
	if (t < VALVE_MIN) return 0;
	if (t < VALVE_MAX) return VALVE_MUL * (t - VALVE_MIN) / 200;
	return VALVE_MUL * (t - (VALVE_MIN + VALVE_MAX) / 2) / 100;
}

static uint8_t input2(uint16_t t) {
	if (t < 1450) return 0;
	if (t > 1550) return 2;
	return 1;
}

static uint16_t output1(int16_t t, uint8_t *s) {
	if (!(*s = !!t)) return 1500;
	if ((t += PUMP_MIN) > PUMP_MAX) return 1500 + PUMP_MAX;
	return 1500 + t;
}

static uint16_t ramp(uint16_t t, uint16_t u, uint16_t x) {
	if (!u || !x) return t;
	if (t < 1500) {
		u -= x;
		if (u > 1450) u = 1450;
		if (t < u) return u;
	} else {
		u += x;
		if (u < 1550) u = 1550;
		if (t > u) return u;
	}
	return t;
}

static uint16_t u1, u2, u3, u4;
static int16_t i1, i2, i3;
static uint8_t s1;

void update(void) {
	s1 = input2(chv[6]);

	i1 = input1(chv[0], &u1, CH1_TRIM);
	i2 = input1(chv[1], &u2, CH2_TRIM);
	i3 = input1(chv[2], &u3, CH3_TRIM);

	uint8_t sl;
	u4 = ramp(output1(i1 + i2 + i3, &sl), u4, PUMP_LIM);

	TIM1_CCR1H = u1 >> 8;
	TIM1_CCR1L = u1;
	TIM1_CCR2H = u2 >> 8;
	TIM1_CCR2L = u2;
	TIM1_CCR3H = u3 >> 8;
	TIM1_CCR3L = u3;
	TIM1_CCR4H = u4 >> 8;
	TIM1_CCR4L = u4;

	PC_ODR = s1 ? 0x20 : 0x00; // C5
	PB_ODR = sl ? 0x00 : 0x20; // B5

	WWDG_CR = 0xff; // Reset watchdog
#ifdef DEBUG
	static uint8_t n;
	if (++n < 26) return; // 130Hz -> 5Hz
	CFG_GCR = 0x00; // Resume main loop
	n = 0;
#endif
}

uint32_t sensor(uint8_t i, uint16_t v) {
	switch (i) {
		case 0: // TMP36 sensor
			return (((uint32_t)v * VOLT1) >> 10) - 100;
		case 1: // Voltage divider
			return ((uint32_t)v * VOLT2) >> 10;
	}
	return 0;
}

void main(void) {
	CLK_CKDIVR = 0x08; // HSI/2=8Mhz clock
	CLK_HSITRIMR = 0x01;

	PB_ODR = 0x20;
	PB_DDR = 0x20; // B5 (active low)
	PC_DDR = 0x20; // C5
	PA_CR1 = 0xff;
	PB_CR1 = 0xff;
	PC_CR1 = 0xff;
	PD_CR1 = 0xf3; // D2,D3 floating

	TIM1_PSCRH = 0x00;
	TIM1_PSCRL = 0x07; // 1Mhz
	TIM1_ARRH = 0x0f;
	TIM1_ARRL = 0x9f; // 250Hz
	TIM1_EGR = 0x01; // UG=1 (force update)
	TIM1_CR1 = 0x01; // CEN=1 (enable counter)
	TIM1_BKR = 0x80; // MOE=1 (enable main output)
	TIM1_CCMR1 = 0x68; // CC1S=00, OC1PE=1, OC1M=110 (CC1 as output, buffered CCR1, PWM mode 1)
	TIM1_CCMR2 = 0x68; // CC2S=00, OC2PE=1, OC2M=110 (CC2 as output, buffered CCR2, PWM mode 1)
	TIM1_CCMR3 = 0x68; // CC3S=00, OC3PE=1, OC3M=110 (CC3 as output, buffered CCR3, PWM mode 1)
	TIM1_CCMR4 = 0x68; // CC4S=00, OC4PE=1, OC4M=110 (CC4 as output, buffered CCR4, PWM mode 1)
	TIM1_CCER1 = 0x11; // CC1E=1, CC2E=1 (enable OC1, OC2)
	TIM1_CCER2 = 0x11; // CC3E=1, CC4E=1 (enable OC3, OC4)

	initsensor();
	initserial();
#ifdef DEBUG
	printf("\n");
	printf("  U1   U2   U3   U4      I1   I2   I3    SW\n");
#endif
	for (;;) {
		CFG_GCR = 0x02; // AL=1 (suspend main loop)
		WAIT_FOR_INTERRUPT();
#ifdef DEBUG
		DISABLE_INTERRUPTS();
		uint16_t _u1 = u1, _u2 = u2, _u3 = u3, _u4 = u4;
		int16_t _i1 = i1, _i2 = i2, _i3 = i3;
		ENABLE_INTERRUPTS();
		printf("%4u %4u %4u %4u    %4d %4d %4d    %d\n",
			_u1, _u2, _u3, _u4, _i1, _i2, _i3, s1);
#endif
	}
}
