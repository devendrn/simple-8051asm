#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "literals.h"
#include "parser.h"
#include "pack.h"

//#define DEBUG

char label[16];
char mnemonic[16];
char operands[3][16];
int operand_count;

int line_count = 0;	// line position for locating error
int org_addr = -1;	// origin address
int addr = 0;		// byte address

struct labels all_labels[256];	// name of all labels
int all_labels_count = 0;

int error_count = 0;

// first column for data
// second column for label substitute details
// 0:skip  1:offset  2:addr16  255:end 
unsigned char out_hex[512][2];

// assembler errors
void print_error(int line_true,char *error,char *element){
	printf("\n");
	if(line_true){
		printf("Line %-3d | ",line_count);
	}
	printf("error: %s",error);
	if(element[0]!='\0'){
		printf(" '%s'",element);
	}
	error_count++;
}

// find label index or create index if not found
int search_label(char *label){
	int i;
	for(i=0;i<all_labels_count;i++){
		if(!strcmp(label,all_labels[i].name)){
			return i;	// return matching index
		}
	}
	// push label if not found
	memcpy(all_labels[i].name,label,strlen(label));
	all_labels[i].addr = -1;	// address can be replaced when label definition is found
	all_labels_count++;
	return i;
}

// assemble into char array
void assemble(unsigned char out[][2],char *mnemonic,char operands[3][16]){
	int er = 0;
	int data[2] = {-1,-1};
	int j = 0;	// num of operands with data
	int k = -1;	// label pos if any

	enum mnemonic_type mn = get_mnemonic_enum(mnemonic);
	if(mn==mn_invalid){
		print_error(1,"Invalid mnemonic",mnemonic);
		er++;
	}

	struct operand op[4];
	for(int i=0;i<3;i++){
		op[i] = get_operand_struct(operands[i]);
		if(op[i].type==op_invalid){
			print_error(1,"Invalid operand",operands[i]);
			er++;
		}
		if(op[i].value>-1){
			if(op[i].type==op_label){
				k = i;
			}
			data[j++] = op[i].value;
		}
	}

	if(er || mn==mn_none){	// no need to assemble if mn/op is invalid or none
		return;
	}

	if(mn==mn_org ){ 
		if(org_addr<0){	// origin not set 
			if(op[0].type==op_direct && op[1].type==op_none && op[2].type==op_none){
				org_addr = op[0].value;
			}
			else{
				print_error(1,"org requires a valid address","");
			}
		}
		else{	// origin already set
			print_error(1,"org can only be set once","");
		}
		return;
	}

	unsigned char opcode = get_opcode(mn,op);
	
	if(opcode!=0xa5){	// store in hex array 
		// pack op code
		out[addr++][0] = opcode;
		
		// pack data
		if(data[0]>-1){
			if(op[0].type==op_dptr){	// dptr takes 2 bytes
				int msb = data[0]/256;
				out[addr++][0] = msb;
				out[addr++][0] = data[0]-msb*256;
				if(data[0]>0xffff){
					print_error(1,"Value cannot be larger than 65535","");
				}
			}
			else{
				out[addr++][0] = data[0];
				if(data[0]>0xff){
					print_error(1,"Value cannot be larger than 255","");
				}
			}
		}
		if(data[1]>-1){
			out[addr++][0] = data[1];
			if(data[1]>0xff){
				print_error(1,"Value cannot be larger than 255","");
			}
		}
		
		// pack label details if any
		if(k>-1){
			int pos = search_label(operands[k]);
			out[addr-1][0] = pos;
			enum operand_type parse_type = all_instructions[opcode].operands[k];
			if(parse_type==op_offset){
				out[addr-1][1] = 1;
			}
			else if(parse_type==op_addr16){
				out[addr-1][1] = 2;
				addr++;	// reserve next space for a byte
			}
		}
	}
	else{
		print_error(1,"Invalid instruction","");
	}

	#ifdef DEBUG
	printf("\n%2d| %02x",line_count,opcode);
	for(int i=0;i<2;i++){
		if(data[i]>-1){
			printf(" %02x",data[i]);
		}
		else{
			printf("   ");
		}
	}
	if(mn!=mn_none){
		printf(" |%6s >%6s |",mnemonic,all_mnemonics[mn]);
		for(int i=0;i<3;i++){
			printf(" %8s > %8s:%4d |",operands[i],all_operands[op[i].type],op[i].value);
		}
	}
	#endif
}

// push defined labels with their address
void push_label_src(char *label,int addr){
	if(!check_label(label)){
		print_error(1,"Invalid label name",label);
		return;
	}
	int i;
	for(i=0;i<all_labels_count;i++){	// check if label already exists
		if(!strcmp(all_labels[i].name,label)){
			if(all_labels[i].addr>=0){
				print_error(1,"Label already defined previously",label);
			}
			all_labels[i].addr = addr;
			return;
		}
	}
	memcpy(all_labels[i].name,label,strlen(label)+1);
	all_labels[i].addr = addr;
	all_labels_count++;
}

// substitute undefined data(labels) with offset or address
void substitute_labels(unsigned char hex_array[][2]){
	int i = -1;
	while(1){
		i++;
		unsigned char *type = &hex_array[i][1];
		if(*type==255){	// end of hex array
			break;
		}
		if(*type==0){	// defined data - skip
			continue;
		}
		int loc = all_labels[hex_array[i][0]].addr;
		if(loc<0){
			print_error(0,"Label not defined",all_labels[hex_array[i][0]].name);
		}
		if(*type==2){	// addr16
			hex_array[i][0] = loc/256;
			hex_array[i+1][0] = loc - 256*hex_array[i][0];
			i++;
		}
		else if(*type==1){	// offset
			int offset = loc - i;
			if(offset>127 || offset<-127){	// todo: find actual bound
				print_error(0,"Label offset out of range",all_labels[hex_array[i][0]].name);
			}
			hex_array[i][0] = offset<0 ? 255+offset : offset-1;
		}
	}
}

int main(int argc, char **argv){

	char *file_in;
	char *file_out = "a.hex";
	
	if(argc==2 && argv[1][0]=='-' && argv[1][1]=='h'){
		printf("Usage:\nasm51 [input_file] -o [output_file]\n");
		return 1;
	}
	else if(argc==2 && argv[1][0]!='-'){
		printf("%s",argv[1]);
		file_in = argv[1];
	}
	else if(argc==4 && !strcmp("-o",argv[2])){
		file_in = argv[1];
		file_out = argv[3];
	}
	else{
		printf("Invalid arguments: Use -h for help\n");
		return 0;
	}

	FILE *asm_file = fopen(file_in,"r");

	if(asm_file==NULL){
		printf("File '%s' not found\n",file_in);
		return 0;
	}

	// process instructions line by line
	char ch;
	while(1){
		// set all empty 
		label[0] = '\0';
		mnemonic[0] = '\0';
		operands[0][0] = '\0';
		operands[1][0] = '\0';
		operands[2][0] = '\0';
		operand_count = 0;

		line_count++;

		// get mnemonic (it can also be label)
		ch = get_token(asm_file,mnemonic,' ');
		
		// if token was	label, look for mnemonic again
		if(ch==':'){
			push_label_src(mnemonic,addr);
			ch = get_token(asm_file,mnemonic,' ');
		}

		// get operand if not eol
		if(ch!='\n'){
			while(1){
				ch = get_token(asm_file,operands[operand_count],',');
				if(ch==','){
					operand_count++;
					continue;
				}
				if(ch=='\n' || feof(asm_file)){
					break;
				}
			}
		}

		if(feof(asm_file)){
			break;
		}

		assemble(out_hex,mnemonic,operands);
	}

	fclose(asm_file);

	out_hex[addr][1] = 0xff;

	#ifdef DEBUG
	printf("\n\nLabels:");
	for(int i=0;i<all_labels_count;i++){
		printf("\n%2d:%7s | %04x",i,all_labels[i].name,all_labels[i].addr);
	}
	printf("\n\nHex array:\n");
	for(int i=0;out_hex[i][1]!=0xff;i++){
		printf(" %02x %01x |",out_hex[i][0],out_hex[i][1]);
		if((1+i)%8==0){
			printf("\n");
		}
	}
	#endif

	substitute_labels(out_hex);

	#ifdef DEBUG
	printf("\n\nFinal hex:\n");
	for(int i=0;out_hex[i][1]!=0xff;i++){
		printf(" %02x",out_hex[i][0]);
		if((1+i)%8==0){
			printf("\n");
		}
	}
	printf("\n");
	#endif

	if(error_count){	// dont pack ihex if there are errors
		return 0;
	}

	FILE *hex_file = fopen(file_out,"w");
	if(hex_file==NULL){
		printf("Could not create '%s'\n",file_out);
		return 0;
	}
	pack_ihex(hex_file,out_hex,org_addr);
	fclose(hex_file);
	return 0;
}
