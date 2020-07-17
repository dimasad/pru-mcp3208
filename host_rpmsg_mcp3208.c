/**
 * Based on examples from Texas Instruments PRU Hands-on Labs.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

uint8_t readBuf[512];
#define DEVICE_NAME  "/dev/rpmsg_pru1"

#define NUM_SCAN_ELEMENTS  4
#define NUM_SCANS          60
#define DATA_BUFFER_LEN    (NUM_SCAN_ELEMENTS * NUM_SCANS)
#define BUFFER_WORD_LEN    2 + DATA_BUFFER_LEN/2

typedef struct {
  uint64_t timestamp_ns;
  uint16_t data[DATA_BUFFER_LEN];
} Buffer;

int main(void) {
  /* Open the rpmsg_pru character device file */
  int fd = open(DEVICE_NAME, O_RDWR);
  if (fd < 0) {
    perror("Failed to open " DEVICE_NAME);
    return -1;
  }

  uint64_t last_ts = 0;
  for(;;) {
    /* Kick the PRU through the RPMsg channel */
    int result = write(fd, 0, 0);
    if (result < 0) {
      perror("Error writing to PRU");
      return -1;
    }
    
    // Read the data from the PRU.
    result = read(fd, readBuf, sizeof readBuf);
    if (result == sizeof(Buffer)) {
      Buffer *b = (Buffer *) readBuf;
      for (int i=0; i<4; i++)
        printf("ch%d=%4" PRIu16 ", ", i, b->data[i]);
      printf("ts=%" PRIu64 ",\t", b->timestamp_ns);
      printf("delta=%" PRIu64 ",\n", b->timestamp_ns - last_ts);

      last_ts = b->timestamp_ns;
    } else if (result < 0) {
      perror("Error reading from device");
      return -1;
    } else {
      unsigned bufsz = sizeof(Buffer);
      printf("[[read only %d bytes, buffer size %u]]\n", result, bufsz);
    }
  }
  
  fsync(0);
  close(fd);
  return 0;
}

