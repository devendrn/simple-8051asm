#include <stdio.h>

#include "pack.h"

// pack bytes into intel hex format with record size of 16
void pack_ihex(char *file_name, unsigned char *hex, int hex_size, unsigned int orgs[][3], int orgs_filled) {

  FILE *file_out = fopen(file_name, "w");
  if (file_out == NULL) {
    printf("Could not create '%s'\n", file_name);
    return;
  }

  // intel ihex format
  // :<byte_count><addr><record_type><bytes><checksum>

  for (int j = 0; j < orgs_filled; j++) {
    unsigned int start = orgs[j][1];
    unsigned int end = orgs[j][2];

    unsigned char byte_count = 0;
    unsigned int addr = orgs[j][0];  // origin address
    unsigned char record_type = 0;
    unsigned char data[16];
    unsigned char checksum = 0;
    
    unsigned int i = start;
    while (i <= end) {
      data[byte_count] = hex[i];
      checksum += data[byte_count];
      byte_count++;
      i++;
      if (byte_count == 16 || i == end + 1) {  // one record complete

        // checksum = 2s complement of sum of all bytes
        checksum += byte_count + (addr >> 8) + (addr & 0x00ff);
        checksum = ~checksum + 1;

        fprintf(file_out, ":%02X%04X%02X", byte_count, addr, record_type);
        for (int j = 0; j < byte_count; j++) {
          fprintf(file_out, "%02X", data[j]);
        }
        fprintf(file_out, "%02hhX\n", checksum);

        addr += byte_count;
        byte_count = 0;
        checksum = 0;
      }
    }
  }
  fprintf(file_out, ":00000001FF");
  fclose(file_out);
}

