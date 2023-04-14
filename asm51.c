#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "asm51.h"
#include "instructions.c"

// all sfrs (basic 8051)
const struct sfr defined_vars[] = {
   {"acc",0xe0,1},
   {"psw",0xd0,1},
   {"p0",0x80,1},
   {"p1",0x90,1},
   {"p2",0xa0,1},
   {"p3",0xb0,1},
   {"ie",0xa8,1},
   {"ip",0xb8,1},
   {"tcon",0x88,1},
   {"scon",0x98,1},
   {"sbuf",0x99,1},
   {"sp",0x81,0},
   {"dpl",0x82,0},
   {"dph",0x83,0},
   {"tl0",0x8a,0},
   {"tl1",0x8b,0},
   {"th0",0x8c,0},
   {"th1",0x8d,0},
   {"pcon",0x87,0},
   {"tmod",0x89,0},
   {"b",0xf0,0}
};

// all valid operands
const char * all_operands[] = {
	"a","@r0","@r1","r0","r1","r2","r3","r4","r5","r6","r7",
	"c","@a+dptr","ab","@a+pc","dptr","@dptr","","addr11",
	"addr16","offset","bit","direct","immed","label",
};

// all valid mnemonics
const char * all_mnemonics[] = {
	"nop","ajmp","ljmp","rr","inc","jbc","acall","lcall",
	"rrc","dec","jb","ret","rl","add","jnb","reti","rlc",
	"addc","jc","orl","jnc","anl","jz","xrl","jnz","jmp",
	"mov","sjmp","movc","div","subb","mul","cpl","cjne",
	"push","clr","swap","xch","pop","setb","da","djnz",
	"xchd","movx",""
};

// get mnemonic enum from string
enum mnemonic_type get_mnemonic_enum(char *word){
	// check through all possible mnemonics
	for(int i=0;i<mn_invalid;i++){
		if(!strcmp(all_mnemonics[i],word)){
			return i;
		}
	}
	return mn_invalid; // return invalid if no matching mnemonic
}

// checks for valid label
int check_label(char *label){
	if(isalpha(label[0])){	// first char of label must be an alphabet
		for(int i=1;label[i]!='\0';i++){	// other chars must be alphanumeric
			if(!isalnum(label[i])){
				return 0; }
		}
		return 1;
	}
	return 0;
}

int str_cmp(const char *word_l,const char *word_s,char end){
	for(int i=0;word_l[i]!=end && word_l[i]!='\0';i++){
		if(word_l[i]!=word_s[i]){
			return 0;
		}
	}
	return 1;
}

// checks if operand is sfr
int check_sfr(char *word,struct operand *out){
	int l = strlen(word);
	
	if(l>3){
		if(word[l-2]=='.'){	// check bit addressing with index
			char last = word[l-1];
			if(last<='7' && last>='0'){
				for(int i=0;i<20;i++){
					if(str_cmp(defined_vars[i].name,word,'.') && defined_vars[i].bit_addr==1){
						out->type = op_bit;
						out->value = defined_vars[i].addr + word[l-1] - '0' ;
						return 1;
					}
				}
			}
			out->type = op_invalid;
			return 1;
		}
	}

	for(int i=0;i<21;i++){	// check all sfr
		if(!strcmp(defined_vars[i].name,word)){
			out->type = op_direct;
			out->value = defined_vars[i].addr;
			return 1;
		}
	}

	return 0;
}

// converts the following formats of string to int
// 12 12h 0x12 -12 'a' 1110b
int str_to_int(int *out_val,char *str){
	int base = 10;
	int start = 0;
	int end = strlen(str) - 1;
	if(str[end]=='h'){	// hex
		base = 16;
		end--;
	}	
	if(str[start]=='0' && str[start+1]=='x'){	// hex
		base = 16;
		start += 2;
	}
	else if(str[end]=='b'){	// binary 
		base = 2;
		end--;
	}

	if(end - start == 2){	// char to ASCII value
		if(str[start]==str[end] && str[end]=='\''){
			*out_val = (int)str[start+1];
			return 1;		
		}
	}

	for(int i=end,j=1;i>=start;i--,j*=base){
		char w = str[i];
		if(i==start && w=='-' && *out_val<256){	// negative index from 0xff
			*out_val = 256-*out_val;
			return 1;
		}	
		int iw = w-'0';	// char to int (0-9)
		unsigned char is_digit = iw>=0 && iw<=9;
		if(base==16){
			if(w<='f' && w>='a'){
				iw = w-87;	// hex char to int (a-f)
			}
			else if(!is_digit){
				return 0;
			}
		}
		else{
			if(!is_digit || (base==2 && iw>1)){
				return 0;
			}
		}
		*out_val += iw*j;
	}
	return 1;
}

// get operand - todo: add bit operands
struct operand get_operand_struct(char *word){
	struct operand out = {op_invalid,0x00};

	// check through all possible operands
	for(int i=0; i<=op_none;i++){
		if(!strcmp(all_operands[i],word)){
			out.type = i;
			out.value = -1;
			return out;
		}
	}

	// check for special function registers like acc,b,psw etc
	if(check_sfr(word,&out)){
		return out;
	}

	// check for value based operands - label,direct,bit,addr,offset,immed
	if(check_label(word)){
		out.type = op_label;
		return out;
	}

	int start = 0;
	if(word[0]=='#'){	// check for immed operand
		out.type = op_immed;
		start = 1;
	}
	if(str_to_int(&out.value,&word[start])){
		//if(out.value>0xff){
		//	out.type = op_addr16;
		//}
		if(out.type!=op_immed){
			out.type = op_direct;
		}
	}
	else{
		out.type = op_invalid;
	}

	return out;
}

// get words one by one from line
char get_token(FILE *file,char *out_str,char end_char){
	char ch;
	int i = 0;
	char j = 0;
	while(1){
		ch = fgetc(file);
		if(ch==';'){
			// if comment skip to eol
			while(fgetc(file)!='\n'){ }
			ch = '\n';
			break;
		}
		if((ch==' ' || ch=='\t')){
			if(end_char==',' || i==0){
				// skip spaces and tabs if scanning for operands or if mnemonic not found
				continue;
			}
		}
		if(ch==end_char || ch==':' || ch=='\n' || feof(file)){
			// if : break (check last char outside the func to detect label)
			// else break if eol/eof
			break;
		}
		if(ch=='\''){	// make char case sensitive for char to ascii operands
			j = 1-j;
		}
		if(j){
			out_str[i++] = ch;	
		}
		else{
			out_str[i++] = tolower(ch);
		}
	}
	out_str[i]='\0'; // end string
	return ch;	// return last char
}

// find label index
int search_label(struct labels *all_labels,int *count,char *label){
	int i;
	for(i=0;i<*count;i++){
		if(!strcmp(label,all_labels[i].name)){
			return i;	// return matching index
		}
	}
	// push label if not found
	memcpy(all_labels[i].name,label,strlen(label));
	all_labels[i].addr = -1;
	*count += 1;
	return i;
}

// assemble into char array
void assemble(unsigned char out[][2],char *mnemonic,char operands[3][16],int *addr,struct labels *all_labels,int *label_count){
	enum mnemonic_type mn = get_mnemonic_enum(mnemonic);
	struct operand op[4];

	int data[2] = {-1,-1};
	int j = 0;	// num of operands with data
	int k = -1;	// label pos if any
	for(int i=0;i<3;i++){
		op[i] = get_operand_struct(operands[i]);
		if(op[i].value>-1){
			if(op[i].type==op_label){
				k = i;
			}
			data[j++] = op[i].value;
		}
	}

	unsigned char opcode = get_opcode(mn,op);
	
	if(opcode!=0xa5){	// store in hex array 
		// pack op code
		out[*addr][0] = opcode;
		*addr += 1;
		
		// pack data
		if(data[0]>-1){
			if(op[0].type==op_dptr){	// dptr takes 2 bytes
				int msb = data[0]/256;
				out[*addr][0] = msb;
				*addr += 1;
				out[*addr][0] = data[0]-msb*256;
				*addr += 1;
			}
			else{
				out[*addr][0] = data[0];
				*addr += 1;
			}
		}
		if(data[1]>-1){
			out[*addr][0] = data[1];
			*addr += 1;
		}
		
		// pack label details if any
		if(k>-1){
			int pos = search_label(all_labels,label_count,operands[k]);
			out[*addr-1][0] = pos;
			enum operand_type parse_type = all_instructions[opcode].operands[k];
			if(parse_type==op_offset){
				out[*addr-1][1] = 1;
			}
			else if(parse_type==op_addr16){
				out[*addr-1][1] = 2;
				*addr += 1;	// reserve next space for a byte
			}
		}
	}

	// for debug
	printf(" %02x",opcode);
	if(data[0]>-1){
		printf(" %02x",data[0]);
	}
	else{
		printf("   ");
	}
	if(data[1]>-1){
		printf(" %02x",data[1]);
	}else{
		printf("   ");
	}
	printf(" |");
	if(mn!=mn_none){
		printf("%6s >%6s |",mnemonic,all_mnemonics[mn]);
		for(int i=0;i<3;i++){
			printf(" %8s > %8s:%4d |",operands[i],all_operands[op[i].type],op[i].value);
		}
	}
	if(mn==mn_invalid){
		printf(" | Error: Invalid mnemonic '%s'",mnemonic);
	}
	else{
		for(int i=0;i<3;i++){
			if(op[i].type==op_invalid)
				printf(" | Error: Invalid operand: '%s'",operands[i]);
		}
	}
	printf("\n");
}

// push defined labels with their address
void push_label_src(struct labels *lb,char *label,int addr,int *array_size){
	int i;
	for(i=0;i<*array_size;i++){	// check if label already exists
		if(!strcmp(lb[i].name,label)){
			lb[i].addr = addr;
			return;
		}
	}
	memcpy(lb[i].name,label,strlen(label)+1);
	lb[i].addr = addr;
	*array_size = *array_size + 1;
}

// substitute undefined data(labels) with offset or address
void substitute_labels(unsigned char hex_array[][2],struct labels *all_labels){
	int i = -1;
	while(1){
		i++;
		unsigned char *type = &hex_array[i][1];
		if(*type==0xff){	// end of hex array
			break;
		}
		if(*type==0){	// defined data - skip
			continue;
		}
		int addr = all_labels[hex_array[i][0]].addr;
		if(*type==2){	// addr16
			hex_array[i][0] = addr/256;
			hex_array[i+1][0] = addr - hex_array[i][0];
			i++;
		}
		if(*type==1){	// offset
			int offset = addr - i;
			if(offset<0){
				hex_array[i][0] = 0xff + offset;
			}
			else{
				hex_array[i][0] = offset - 1;
			}
		}
	}
}

// pack bytes into intel hex format with record size of 16
// :<byte_count><addr><record_type><bytes><checksum>
void pack_ihex(FILE *file_out,unsigned char hex_array[][2],int org_addr){
	unsigned char byte_count = 0;
	unsigned int addr = org_addr;	// orgin address
	unsigned char record_type = 0;
	unsigned char data[16];
	unsigned char checksum = 0;
	
	int i = 0;
	while(hex_array[i][1]!=0xff){
		data[byte_count] = hex_array[i][0];
		checksum += data[byte_count];
		byte_count++;
		i++;
		if(byte_count==16 || hex_array[i][1]==0xff){	// one record complete
			
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
	else if(argc==4 && strcmp("-o",argv[2])){
		file_in = argv[1];
		file_out = argv[3];
	}
	else{
		printf("Invalid arguments: Use -h for help\n");
		return 0;
	}

	FILE *asm_file = fopen(file_in,"r");

	if(asm_file == NULL){
		printf("File '%s' not found\n",file_in);
		return 0;
	}

	int line_count = 0;	// line position for locating error
	int addr = 0x0000;	// byte address

	struct labels all_labels[256];	// index and name of all labels
	int all_labels_count = 0;

	char label[16];
	char mnemonic[16];
	char operands[3][16];
	int operand_count;
	
	// first column for data
	// second column for label substitute details
	// 0:skip  1:offset  2:addr16  255:end 
	unsigned char out_hex[512][2];	// output hex array

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

		// get mnemonic (it can also be label)
		ch = get_token(asm_file,mnemonic,' ');
		
		// if token was	label, look for mnemonic again
		if(ch==':'){
			// memcpy(label,mnemonic,strlen(mnemonic)+1);
			push_label_src(all_labels,mnemonic,addr,&all_labels_count);
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

		line_count++;

		if(feof(asm_file)){
			break;
		}

		printf("%2d|",line_count);
		assemble(out_hex,mnemonic,operands,&addr,all_labels,&all_labels_count);
	}

	fclose(asm_file);

	printf("\nLabels:");
	for(int i=0;i<all_labels_count;i++){
		printf("\n%2d:%7s | %04x",i,all_labels[i].name,all_labels[i].addr);
	}
	printf("\n");
	
	out_hex[addr][1] = 0xff;
	printf("\nHex array:\n");
	for(int i=0;out_hex[i][1]!=0xff;i++){
		printf(" %02x %02x |",out_hex[i][0],out_hex[i][1]);
		if((1+i)%8==0){
			printf("\n");
		}
	}

	substitute_labels(out_hex,all_labels);

	printf("\n\nFinal hex:\n");
	for(int i=0;out_hex[i][1]!=0xff;i++){
		printf(" %02x",out_hex[i][0]);
		if((1+i)%8==0){
			printf("\n");
		}
	}

	FILE *hex_file = fopen(file_out,"w");
	pack_ihex(hex_file,out_hex,0);	
	return 0;
}
