#ifndef PACK_H
#define PACK_H

#include <stdio.h>

void pack_ihex(FILE *file_out, unsigned char hex_array[][2], int org_addr);

#endif
