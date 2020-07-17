/**
 * Based on code from the Texas Instruments PRU Hands-On Labs and
 * Mark A. Yoder's PRU Cookbook
 */

#include <stdint.h>
#include <stdio.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include <rsc_types.h>
#include <pru_rpmsg.h>

#include "resource_table_empty.h"
#include "pru_timer.h"
#include "../pru_common/shared_buffer.h"


volatile register uint32_t __R30;
volatile register uint32_t __R31;


/* PRU0-to-PRU1 interrupt */
#define PRU0_PRU1_EVT     (20)
#define PRU0_PRU1_TRIGGER (__R31 = (PRU0_PRU1_EVT - 16) | (1 << 5))

// SPI registers
#define MISO_REG __R31 //P9_29
#define MOSI_REG __R30 //P9_30
#define SCLK_REG __R30 //P9_31
#define CS_REG __R30 //P9_28

#define MISO_MASK ((uint32_t)(1 << 1))
#define MOSI_MASK ((uint32_t)(1 << 2))
#define SCLK_MASK ((uint32_t)(1 << 0))
#define CS_MASK ((uint32_t)(1 << 3))

static inline void mosi_set() {
  MOSI_REG |= MOSI_MASK;
}

static inline void mosi_clr() {
	MOSI_REG &= ~MOSI_MASK;
}

static inline void sclk_set() {
	SCLK_REG |= SCLK_MASK;
}

static inline void sclk_clr() {
	SCLK_REG &= ~SCLK_MASK;
}

static inline void cs_set() {
	CS_REG |= CS_MASK;
}

static inline void cs_clr() {
	CS_REG &= ~CS_MASK;
}

static inline uint8_t miso_rd() {
	if (MISO_REG & MISO_MASK)
		return 1;
	else
		return 0;
}

uint16_t convert(char CH) { // ch -> number of channels

	int ch = CH - '0';

	sclk_clr(); // Initialize clock
	cs_clr(); // Set CS to low (active)
	mosi_set(); // Set MOSI to HIGH (start bit)
	__delay_cycles(30); // 30 cycles = 150ns

	sclk_set(); // cycle clock (data transfer)
	__delay_cycles(100); // 100 cycles = 500ns

	// SGL / ~DIFF
		sclk_clr();
		mosi_set(); // Set SGL = 1 (single ended)
		__delay_cycles(100); // 100 cycles = 500ns
		sclk_set();
		__delay_cycles(100); // 100 cycles = 500ns

	// Channel Selection D2, D1, D0 configure
	sclk_clr();
	if(ch == 1 || ch == 2 || ch == 3 || ch == 4)  // D2
		mosi_clr();
	else
		mosi_set();
	__delay_cycles(100); // 100 cycles = 500ns
		sclk_set();
		__delay_cycles(100); // 100 cycles = 500ns

	if(ch == 1 || ch == 2 || ch == 5 || ch == 6)  // D1
		mosi_clr();
	else
		mosi_set();
	__delay_cycles(100); // 100 cycles = 500ns
		sclk_set();
		__delay_cycles(100); // 100 cycles = 500ns

	if(ch ==1 || ch%2 != 0)  //D0
		mosi_clr();
	else
		mosi_set();
	__delay_cycles(100); // 100 cycles = 500ns
        sclk_set();
        __delay_cycles(100); // 100 cycles = 500ns

	// Wait while device samples channel
	sclk_clr();
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_set();
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_clr(); // Sampling completed

	 uint16_t result = 0;

	// Read null bit
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_set();
	int null_bit = miso_rd();
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_clr();

	// Read b11
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_set();
	result |= miso_rd();
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_clr();

	// Read b10
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_set();
	result <<= 1;
	result |= miso_rd();
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_clr();

	// Read b9
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_set();
	result <<= 1;
	result |= miso_rd();
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_clr();

	// Read b8
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_set();
	result <<= 1;
	result |= miso_rd();
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_clr();

	// Read b7
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_set();
	result <<= 1;
	result |= miso_rd();
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_clr();

	// Read b6
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_set();
	result <<= 1;
	result |= miso_rd();
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_clr();

	// Read b5
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_set();
	result <<= 1;
	result |= miso_rd();
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_clr();

	// Read b4
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_set();
	result <<= 1;
	result |= miso_rd();
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_clr();

	// Read b3
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_set();
	result <<= 1;
	result |= miso_rd();
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_clr();

	// Read b2
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_set();
	result <<= 1;
	result |= miso_rd();
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_clr();

	// Read b1
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_set();
	result <<= 1;
	result |= miso_rd();
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_clr();

	// Read b0
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_set();
	result <<= 1;
	result |= miso_rd();
	__delay_cycles(100); // 100 cycles = 500ns
	sclk_clr();

	cs_set(); // Release SPI bus

	return result;
}

void main() {
  // Clear any pending PRU-generated events
  __R31 = 0x00000000;

  // Initialize interupts so the PRUs can be syncronized.
  CT_INTC.CMR5_bit.CH_MAP_20 = 0;     // Map event 20 to channel 0
  CT_INTC.HMR0_bit.HINT_MAP_0 = 0;    // Map channel 0 to host 0
  CT_INTC.SICR = PRU0_PRU1_EVT;       // Ensure event 20 is cleared
  CT_INTC.EISR = PRU0_PRU1_EVT;       // Enable event 20
  CT_INTC.HIEISR = 0;                 // Enable Host interrupt 0
  CT_INTC.GER = 1;                    // Globally enable host interrupts

  // Initialize buffer index
  *buffer_index = 0;
  
  // Start timer
  start_pru0_timer();
  
  int i = 0;
  for (;;) {
    volatile Buffer *curr_buff = buffer + *buffer_index;
    curr_buff->timestamp_ns = clock_ns();
    curr_buff->data[0] = ++i;
    curr_buff->data[1] = -i;
    curr_buff->data[2] = 2*i;
    curr_buff->data[3] = 4*i;

    // Switch buffers
    *buffer_index ^= 1;
    
    // Trigger event
    PRU0_PRU1_TRIGGER;
    
    __delay_cycles(480000 - 2640/5 - 350);
  }
  
  __halt();
}
