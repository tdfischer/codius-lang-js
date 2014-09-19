#include <stdio.h>
#include <unistd.h>

#include "codius-util.h"

typedef struct codius_rpc_header_s codius_rpc_header_t;

struct codius_rpc_header_s {
  unsigned long magic_bytes;
  unsigned long callback_id;
  unsigned long size;
};


const unsigned long CODIUS_MAGIC_BYTES = 0xC0D105FE;

/* Make synchronous function call outside the sandbox.
   Return the number of characters written to resp_buf if
   buf_size had been sufficiently large (not counting null terminator). */
int codius_sync_call(const char* message, size_t len, char *resp_buf, size_t buf_size) {
  size_t bytes_read;
  const int sync_fd = 3;
  int resp_len;

  codius_rpc_header_t rpc_header;
  rpc_header.magic_bytes = CODIUS_MAGIC_BYTES;
  rpc_header.callback_id = 0;
  rpc_header.size = len;
  
  if (-1==write(sync_fd, &rpc_header, sizeof(rpc_header)) ||
      -1==write(sync_fd, message, len)) {
    perror("write()");
    printf("Error writing to fd %d\n", sync_fd);
    return -1;
  }
  
  bytes_read = read(sync_fd, &rpc_header, sizeof(rpc_header));
  if (bytes_read==-1 || rpc_header.magic_bytes!=CODIUS_MAGIC_BYTES) {
    printf("Error reading from fd %d\n", sync_fd);
    return -1;
  }
  
  // Do not read more than buf_size.
  if (rpc_header.size < buf_size) {
    resp_len = rpc_header.size;  
  } else {
    resp_len = buf_size-1;
  }
  
  bytes_read = read(sync_fd, resp_buf, resp_len);
  if (bytes_read==-1) {
    perror("read()");
    printf("Error reading from fd %d\n", sync_fd);
    fflush(stdout);

    return -1;
  }

  return resp_len;
}