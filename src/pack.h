#ifndef PACK_H
#define PACK_H

#include <stdio.h>

void pack_ihex(char *file_name, unsigned char *hex, int hex_size, int org_addr);

#endif
