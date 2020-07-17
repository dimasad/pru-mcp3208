/**
 * Definition of the data buffer defined in the shared PRU memory.
 */

#include <stdint.h>

#define NUM_SCAN_ELEMENTS  4
#define NUM_SCANS          60
#define DATA_BUFFER_LEN    (NUM_SCAN_ELEMENTS * NUM_SCANS)
#define BUFFER_WORD_LEN    2 + DATA_BUFFER_LEN/2


typedef struct __attribute__((__packed__)) {
  uint64_t timestamp_ns;
  uint16_t data[DATA_BUFFER_LEN];
} Buffer;

#define PRU_SHARED_RAM 0x10000
static volatile uint8_t *buffer_index = (uint8_t *) (PRU_SHARED_RAM);
static volatile Buffer *buffer = (Buffer*) (PRU_SHARED_RAM + 1);
