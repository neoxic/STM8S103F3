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

#pragma once

#include <stdint.h>
#include <stdio.h>

#define DISABLE_INTERRUPTS() __asm__("sim")
#define ENABLE_INTERRUPTS()  __asm__("rim")
#define WAIT_FOR_INTERRUPT() __asm__("wfi")

#define sfr(x) (*(volatile uint8_t *)(x))
#define wsfr(x) (*(volatile uint16_t *)(x))

#define PA_ODR sfr(0x5000)
#define PA_IDR sfr(0x5001)
#define PA_DDR sfr(0x5002)
#define PA_CR1 sfr(0x5003)
#define PA_CR2 sfr(0x5004)
#define PB_ODR sfr(0x5005)
#define PB_IDR sfr(0x5006)
#define PB_DDR sfr(0x5007)
#define PB_CR1 sfr(0x5008)
#define PB_CR2 sfr(0x5009)
#define PC_ODR sfr(0x500a)
#define PC_IDR sfr(0x500b)
#define PC_DDR sfr(0x500c)
#define PC_CR1 sfr(0x500d)
#define PC_CR2 sfr(0x500e)
#define PD_ODR sfr(0x500f)
#define PD_IDR sfr(0x5010)
#define PD_DDR sfr(0x5011)
#define PD_CR1 sfr(0x5012)
#define PD_CR2 sfr(0x5013)

#define EXTI_CR1 sfr(0x50a0)

#define CLK_CKDIVR   sfr(0x50c6)
#define CLK_HSITRIMR sfr(0x50cc)

#define WWDG_CR sfr(0x50d1)

#define UART_SR   sfr(0x5230)
#define UART_DR   sfr(0x5231)
#define UART_BRR1 sfr(0x5232)
#define UART_BRR2 sfr(0x5233)
#define UART_CR2  sfr(0x5235)
#define UART_CR5  sfr(0x5238)

#define TIM1_CR1   sfr(0x5250)
#define TIM1_IER   sfr(0x5254)
#define TIM1_SR1   sfr(0x5255)
#define TIM1_EGR   sfr(0x5257)
#define TIM1_CCMR1 sfr(0x5258)
#define TIM1_CCMR2 sfr(0x5259)
#define TIM1_CCMR3 sfr(0x525a)
#define TIM1_CCMR4 sfr(0x525b)
#define TIM1_CCER1 sfr(0x525c)
#define TIM1_CCER2 sfr(0x525d)
#define TIM1_PSCRH sfr(0x5260)
#define TIM1_PSCRL sfr(0x5261)
#define TIM1_ARRH  sfr(0x5262)
#define TIM1_ARRL  sfr(0x5263)
#define TIM1_CCR1H sfr(0x5265)
#define TIM1_CCR1L sfr(0x5266)
#define TIM1_CCR2H sfr(0x5267)
#define TIM1_CCR2L sfr(0x5268)
#define TIM1_CCR3H sfr(0x5269)
#define TIM1_CCR3L sfr(0x526a)
#define TIM1_CCR4H sfr(0x526b)
#define TIM1_CCR4L sfr(0x526c)
#define TIM1_BKR   sfr(0x526d)

#define TIM2_CR1   sfr(0x5300)
#define TIM2_IER   sfr(0x5303)
#define TIM2_SR1   sfr(0x5304)
#define TIM2_EGR   sfr(0x5306)
#define TIM2_CCMR1 sfr(0x5307)
#define TIM2_CCMR3 sfr(0x5309)
#define TIM2_CCER1 sfr(0x530a)
#define TIM2_CCER2 sfr(0x530b)
#define TIM2_PSCR  sfr(0x530e)
#define TIM2_ARRH  sfr(0x530f)
#define TIM2_ARRL  sfr(0x5310)

#define TIM4_CR1  sfr(0x5340)
#define TIM4_IER  sfr(0x5343)
#define TIM4_SR   sfr(0x5344)
#define TIM4_EGR  sfr(0x5345)
#define TIM4_PSCR sfr(0x5347)
#define TIM4_ARR  sfr(0x5348)

#define ADC_CSR  sfr(0x5400)
#define ADC_CR1  sfr(0x5401)
#define ADC_CR2  sfr(0x5402)
#define ADC_DR  wsfr(0x5404)
#define ADC_TDR wsfr(0x5406)

#define CFG_GCR sfr(0x7f60)

#define NESTED_IRQ(n) (sfr(0x7f70 + (n) / 4) &= ~(3 << ((n) % 4 * 2))) // Set level 2 priority

#define EXTI_PCIRQ 5
#define EXTI_PDIRQ 6
#define UART_TXIRQ 17
#define UART_RXIRQ 18
#define TIM2_UIRQ  13
#define TIM4_UIRQ  23

// #define DEBUG // Debug mode

extern uint16_t chv[14];

void initserial(void);
void initsensor(void);
void update(void);
uint32_t sensor(uint8_t i, uint16_t v);
uint16_t sensortype(uint8_t i);
uint8_t sensordata(uint8_t i, uint32_t *v);
