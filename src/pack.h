#ifndef PACK_H
#define PACK_H

#include <stdio.h>

// pack bytes into intel hex format with record size of 16
void pack_ihex(FILE *file_out,unsigned char hex_array[][2],int org_addr);

#endif
