/**
 * Based on code from the Texas Instruments PRU examples.
 */

#include <stdint.h>
#include <stdio.h>
#include <pru_cfg.h>
#include <pru_ctrl.h>
#include <pru_intc.h>
#include <rsc_types.h>
#include <pru_rpmsg.h>

#include "resource_table_1.h"
#include "../pru_common/shared_buffer.h"

volatile register uint32_t __R30;
volatile register uint32_t __R31;

// Bit mask of the Host-0 and Host-1 interrupts
#define HOST0_INT			((uint32_t) 1 << 30)
#define HOST1_INT			((uint32_t) 1 << 31)

/* The PRU-ICSS system events for RPMsg are defined in the Linux device tree
 * PRU0 uses system event 16 (To ARM) and 17 (From ARM)
 * PRU1 uses system event 18 (To ARM) and 19 (From ARM)
 */
#define TO_ARM_HOST			18
#define FROM_ARM_HOST			19

#define CHAN_NAME			"rpmsg-pru"
#define CHAN_DESC			"Channel 1"
#define CHAN_PORT			1

#define PRU1_RPMSG_SRC                  1024

/*
 * Used to make sure the Linux drivers are ready for RPMsg communication
 * Found at linux-x.y.z/include/uapi/linux/virtio_config.h
 */
#define VIRTIO_CONFIG_S_DRIVER_OK	4

#define NUM_OUT_BUFFERS                 2
Buffer out_buffer[NUM_OUT_BUFFERS];
uint8_t out_buffer_status[NUM_OUT_BUFFERS];
uint8_t out_buffer_index;

enum {
  BUFFER_FREE = 0,
  BUFFER_FULL = 1,
};

void main(void) {
  struct pru_rpmsg_transport transport;
  uint16_t src, dst, len;
  volatile uint8_t *status;
  
  /* AM335x must enable OCP master port access in order for the PRU to
   * read external memories.
   */
  CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;
  
  /* Clear the status of the PRU-ICSS system event that the ARM will
     use to 'kick' us */
  CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

  /* Make sure the Linux drivers are ready for RPMsg communication */
  status = &resourceTable.rpmsg_vdev.status;
  while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK));
        
  /* Initialize the RPMsg transport structure */
  pru_rpmsg_init(&transport, &resourceTable.rpmsg_vring0,
                 &resourceTable.rpmsg_vring1, TO_ARM_HOST, FROM_ARM_HOST);
  
  /* Create the RPMsg channel between the PRU and ARM user space using the
     transport structure. */
  while (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, CHAN_NAME, CHAN_DESC, CHAN_PORT)
         != PRU_RPMSG_SUCCESS);
  
  for (;;) {
    // Wait for PRU 0
    while((__R31 & HOST0_INT) == 0);
    
    // Clear event 20
    CT_INTC.SICR = 20;

    uint8_t completed_buffer_index = *buffer_index ? 0 : 1;
    volatile Buffer *completed_buffer = buffer + completed_buffer_index;
    memcpy(out_buffer, completed_buffer, sizeof(Buffer));
    
    // Send message to ARM
    pru_rpmsg_send(&transport, CHAN_PORT, PRU1_RPMSG_SRC,
                   out_buffer, sizeof(Buffer));
  }
}
