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

// SPI registers bit masks
#define MISO_MASK (1 << 1)
#define MOSI_MASK (1 << 2)
#define SCLK_MASK (1 << 0)
#define CS_MASK   (1 << 3)

// MCP3208 timing delays, in PRU cycles (200MHz ==> 5ns / cycle)
#define MCP3208_TSUCS_CYC    40  //CS~ Fall to First Rising CLK edge (min 100ns)
#define MCP3208_THI_CYC      100 //Clock High Time (min 250ns)
#define MCP3208_TLO_CYC      100 //Clock Low Time (min 250ns)
#define MCP3208_TCSH_CYC     200 //CS~ Disable Time (min 500ns)

// Control bits of the scan elements
static uint8_t scan_ctrl[NUM_SCAN_ELEMENTS] = {
  0b1000, //Single ended, ch0
  0b1001, //Single ended, ch1
  0b1010, //Single ended, ch2
  0b1011, //Single ended, ch3
};

static inline void mosi_set() {
  __R30 |= MOSI_MASK;
}

static inline void mosi_clr() {
  __R30 &= ~MOSI_MASK;
}

static inline void sclk_set() {
  __R30 |= SCLK_MASK;
}

static inline void sclk_clr() {
  __R30 &= ~SCLK_MASK;
}

static inline void cs_set() {
  __R30 |= CS_MASK;
}

static inline void cs_clr() {
  __R30 &= ~CS_MASK;
}

static inline uint32_t miso_rd() {
  return __R31 & MISO_MASK;
}

uint16_t convert(uint8_t ctrl) {
  cs_clr();   // Set CS to low (active)
  mosi_set(); // Set MOSI to HIGH (start bit)

  sclk_clr();
  __delay_cycles(MCP3208_TSUCS_CYC);
  sclk_set();
  __delay_cycles(MCP3208_THI_CYC);
  
  // Send control bits
  uint8_t mask;
  for (mask=0b1000; mask!=0; mask>>=1) {
    sclk_clr();
    if (ctrl & mask)
      mosi_set();
    else
      mosi_clr();
    
    __delay_cycles(MCP3208_TLO_CYC);
    sclk_set();
    __delay_cycles(MCP3208_THI_CYC);
  }

  // Wait for conversion (part of t_SAMPLE)
  sclk_clr();
  __delay_cycles(MCP3208_TLO_CYC);
  sclk_set();
  __delay_cycles(MCP3208_THI_CYC);

  // Read the null bit and conversion result
  int8_t i;
  uint16_t result = 0;
  for (i=0; i<13; i++){
    sclk_clr();
    __delay_cycles(MCP3208_TLO_CYC);
    sclk_set();

    // Incorporate the incoming data
    result <<= 1;
    if (miso_rd())
      result++;
    
    __delay_cycles(MCP3208_THI_CYC);
  }

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
  for (;;) {
    volatile Buffer *curr_buff = buffer + *buffer_index;
    curr_buff->timestamp_ns = clock_ns();
    
    int i;
    for (i=0; i<DATA_BUFFER_LEN; i++) {
      uint8_t ctrl = scan_ctrl[i % NUM_SCAN_ELEMENTS];
      curr_buff->data[i] = convert(ctrl);
      __delay_cycles(MCP3208_TCSH_CYC);
    }
    
    // Switch buffers
    *buffer_index ^= 1;
    *buffer_index &= 1; // For precaution
    
    // Trigger event
    PRU0_PRU1_TRIGGER;
  }
  
  __halt();
}
