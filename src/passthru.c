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

#define PC_PIN 0x08 // C3
#define PD_PIN 0x02 // D1

void EXTI_PC(void) __interrupt(EXTI_PCIRQ) {
	if (PC_IDR & PC_PIN) {
		PD_DDR = 0x00; // Dx (input high)
		PD_CR2 = PD_PIN; // Dx (enable interrupts)
	} else {
		PD_CR2 = 0x00; // Dx (disable interrupts)
		PD_DDR = PD_PIN; // Dx (output low)
	}
}

void EXTI_PD(void) __interrupt(EXTI_PDIRQ) {
	if (PD_IDR & PD_PIN) {
		PC_DDR = 0x00; // Cx (input high)
		PC_CR2 = PC_PIN; // Cx (enable interrupts)
	} else {
		PC_CR2 = 0x00; // Cx (disable interrupts)
		PC_DDR = PC_PIN; // Cx (output low)
	}
}

void main(void) {
	CLK_CKDIVR = 0x00; // HSI=16Mhz clock

	PB_ODR = 0x20;
	PB_DDR = 0x20; // B5 (turn off LED)
	PC_CR1 = PC_PIN; // Cx (input/pull-up)
	PC_CR2 = PC_PIN; // Cx (enable interrupts)
	PD_CR1 = PD_PIN; // Dx (input/pull-up)
	PD_CR2 = PD_PIN; // Dx (enable interrupts)
	EXTI_CR1 = 0xf0; // PCIS=11, PDIS=11 (rising and falling edge)

	CFG_GCR = 0x02; // AL=1 (wait forever)
	WAIT_FOR_INTERRUPT();
}
