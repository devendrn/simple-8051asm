#include "pack.h"
#include <stdio.h>

// pack bytes into intel hex format with record size of 16
// :<byte_count><addr><record_type><bytes><checksum>
void pack_ihex(FILE *file_out,unsigned char hex_array[][2],int org_addr){
	unsigned char byte_count = 0;
	unsigned int addr = org_addr>=0 ? org_addr : 0;	// origin address
	unsigned char record_type = 0;
	unsigned char data[16];
	unsigned char checksum = 0;

	int i = 0;
	while(hex_array[i][1]!=255){
		data[byte_count] = hex_array[i][0];
		checksum += data[byte_count];
		byte_count++;
		i++;
		if(byte_count==16 || hex_array[i][1]==255){	// one record complete

			// checksum = 2s complement of sum of all bytes
			int addr_msb = addr/256;
			checksum += byte_count + addr_msb + (addr-addr_msb*256);
			checksum = ~checksum + 1;

			fprintf(file_out,":%02X%04X%02X",byte_count,addr,record_type);
			for(int j=0;j<byte_count;j++){
				fprintf(file_out,"%02X",data[j]);
			}
			fprintf(file_out,"%02hhX\n",checksum);

			addr += byte_count;
			byte_count = 0;
			checksum = 0;
		}
	}
	fprintf(file_out,":00000001FF");	// end
}
