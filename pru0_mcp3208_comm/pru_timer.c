#include "pru_timer.h"

/// Holds the excess cycle count which would have overflowed the CYCLE counter
static uint64_t pru0_extended_cycles = 0;


static inline void stop_pru0_timer() {
  PRU0_CTRL.CTRL_bit.CTR_EN = 0;  // Disable cycle counter
}


void manage_pru0_cycle_overflow() {
  unsigned cycle = PRU0_CTRL.CYCLE;
  if (cycle & (1 << 31)) {
    // Reset counter to the approximate cycle count between the register is
    // read and the timer is restarted.
    stop_pru0_timer();
    PRU0_CTRL.CYCLE = 10;
    start_pru0_timer();
    
    // Save elapsed count
    pru0_extended_cycles += cycle;
  }
}


uint64_t clock_ns() {
  manage_pru0_cycle_overflow();
  uint64_t total_cycles = PRU0_CTRL.CYCLE + pru0_extended_cycles;
  return total_cycles * 5;
}
