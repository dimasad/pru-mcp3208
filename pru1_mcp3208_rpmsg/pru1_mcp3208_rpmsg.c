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
 * PRU0 uses system event 16 (To ARM) and 17 (From ARM).
 * PRU1 uses system event 18 (To ARM) and 19 (From ARM).
 * Our application used event 20 to notify PRU1 from PRU0.
 */
#define TO_ARM_HOST			18
#define FROM_ARM_HOST			19
#define PRU0_PRU1_EVT                   20

#define CHAN_NAME			"rpmsg-pru"
#define CHAN_DESC			"Channel 1"
#define CHAN_PORT			1

#define PRU1_RPMSG_SRC                  1024

/*
 * Used to make sure the Linux drivers are ready for RPMsg communication
 * Found at linux-x.y.z/include/uapi/linux/virtio_config.h
 */
#define VIRTIO_CONFIG_S_DRIVER_OK	4


// Define the output queue
#define MAX_QUEUE_SIZE                  4
Buffer queue[MAX_QUEUE_SIZE];
uint8_t queue_head;
uint8_t queue_size;

// Incoming message payload
uint8_t in_payload[RPMSG_MESSAGE_SIZE];

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
  while (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, CHAN_NAME,
                           CHAN_DESC, CHAN_PORT) != PRU_RPMSG_SUCCESS);
  
  for (;;) {
    // Check for new data from PRU0
    if (__R31 & HOST0_INT) {
      // Clear the event status
      CT_INTC.SICR = PRU0_PRU1_EVT;

      uint8_t completed_buffer_index = *buffer_index ? 0 : 1;
      void *completed_buffer = buffer + completed_buffer_index;

      uint8_t queue_tail = (queue_head + queue_size) % MAX_QUEUE_SIZE;
      memcpy(queue + queue_tail, completed_buffer, sizeof(Buffer));
      if (queue_size < MAX_QUEUE_SIZE)
        queue_size++;
      else
        queue_head = (queue_head + 1) % MAX_QUEUE_SIZE;
    }

    // Check for a data request from the ARM host, and if it can be fulfilled
    if (__R31 & HOST1_INT && queue_size) {
      // Get the message
      int16_t status;
      status = pru_rpmsg_receive(&transport, &src, &dst, in_payload, &len);
      
      // If there are no more messages, then clear the interrupt event status
      if (status == PRU_RPMSG_NO_BUF_AVAILABLE)
        CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;
      
      // If a message was received, then send the data from the queue
      if (status == PRU_RPMSG_SUCCESS) {
        // Send the data back
        void *out_buffer = queue + queue_head;
        pru_rpmsg_send(&transport, dst, src, out_buffer, sizeof(Buffer));

        // Update the queue
        queue_size--;
        queue_head = (queue_head + 1) % MAX_QUEUE_SIZE;
      }
    }
  }
}
