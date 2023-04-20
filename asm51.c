#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "asm51.h"
#include "instructions.c"

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

// all sfrs (basic 8051)
const struct sfr defined_vars[] = {
	// bit addressable (0-10)
	{"acc",0xe0},
	{"psw",0xd0},
	{"p0",0x80},
	{"p1",0x90},
	{"p2",0xa0},
	{"p3",0xb0},
	{"ie",0xa8},
	{"ip",0xb8},
	{"tcon",0x88},
	{"scon",0x98},
	{"sbuf",0x99},

	// not bit addressable (11-20)
	{"sp",0x81},
	{"dpl",0x82},
	{"dph",0x83},
	{"tl0",0x8a},
	{"tl1",0x8b},
	{"th0",0x8c},
	{"th1",0x8d},
	{"pcon",0x87},
	{"tmod",0x89},
	{"b",0xf0}
};

// all valid operands
const char * all_operands[] = {
	"a","@r0","@r1","r0","r1","r2","r3","r4","r5","r6","r7",
	"c","@a+dptr","ab","@a+pc","dptr","@dptr","","addr11",
	"addr16","offset","bit","direct","immed","label",
};

// all valid mnemonics (org and empty char is not part of actual mnemonics)
const char * all_mnemonics[] = {
	"nop","ajmp","ljmp","rr","inc","jbc","acall","lcall",
	"rrc","dec","jb","ret","rl","add","jnb","reti","rlc",
	"addc","jc","orl","jnc","anl","jz","xrl","jnz","jmp",
	"mov","sjmp","movc","div","subb","mul","cpl","cjne",
	"push","clr","swap","xch","pop","setb","da","djnz",
	"xchd","movx","","org"
};

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

// check if string is a valid label
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

// compare string up to specified char
int str_cmp(const char *word_l,const char *word_s,char end){
	for(int i=0;word_l[i]!=end && word_l[i]!='\0';i++){
		if(word_l[i]!=word_s[i]){
			return 0;
		}
	}
	return 1;
}

// check if operand is a sfr
int check_sfr(char *word,struct operand *out){
	// check for sfr bit - bit.index
	int l = strlen(word);
	char last = l>3 ? word[l-1] : 0;
	if(word[l-2]=='.' && last<='7' && last>='0'){
		for(int i=0;i<11;i++){	// 0-10 defined_vars are bit addressable
			if(str_cmp(defined_vars[i].name,word,'.')){
				out->type = op_bit;
				out->value = defined_vars[i].addr + last - '0';
				return 1;
			}
		}
		out->type = op_invalid;
		return 1;
	}
	
	// check for sfr
	for(int i=0;i<21;i++){	
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
	
	// ascii - 'a' 
	if(end - start == 2 && str[start]==str[end] && str[end]=='\''){
		*out_val = (int)str[start+1];
		return 1;		
	}
	
	// hex - 0xab, 12h
	if(str[end]=='h'){
		base = 16;
		end--;
	}	
	else if(str[start]=='0' && str[start+1]=='x'){
		base = 16;
		start += 2;
	}

	// binary - 10110b
	else if(str[end]=='b'){	// binary 
		base = 2;
		end--;
	}

	for(int i=end,j=1;i>=start;i--,j*=base){
		char w = str[i];
		if(i==start && w=='-' && *out_val<256){	// negative index from 0xff
			*out_val = 256-*out_val;
			return 1;
		}	
		int iw = w-'0';	// char to int (0-9)
		if(base==16 && w<='f' && w>='a'){
			iw = w-87;	// hex char to int (a-f)
		}
		else if(iw<0 || iw>9 || (base==2 && iw>1)){	// invalid char
			return 0;
		}
		*out_val += iw*j;
	}
	return 1;
}

// get a word from line
char get_token(FILE *file,char *out_str,char end_char){
	char ch;
	int i = 0;
	char case_sensitive = 0;
	while(1){
		ch = fgetc(file);
		if(ch==';'){
			// if comment skip to eol
			while(ch!='\n'){
				ch = fgetc(file);
			}
			break;
		}
		else if((ch==' ' || ch=='\t') && (end_char==',' || i==0)){
			// skip spaces and tabs if scanning for operands or if mnemonic not found
			continue;
		}
		else if(ch==end_char || ch==':' || ch=='\n' || feof(file)){
			// break (check last char outside the func to detect label)
			// break if eol/eof
			break;
		}
		else if(ch=='\''){
			// toggle case sensitive for '' operands
			case_sensitive = 1 - case_sensitive;
		}
		out_str[i++] = case_sensitive ? ch : tolower(ch);
	}
	out_str[i]='\0'; // end string
	return ch;	// return last char
}

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

// get operand 
struct operand get_operand_struct(char *word){
	struct operand out = {op_invalid,0x00};

	// check through all possible operands
	for(int i=0;i<=op_none;i++){
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

	// check for label
	if(check_label(word)){
		out.type = op_label;
		return out;
	}
	
	// check for value based operands
	int start = 0;
	if(word[0]=='#'){	// immed operand
		out.type = op_immed;
		start = 1;
	}
	if(str_to_int(&out.value,&word[start])){
		if(out.type!=op_immed){
			out.type = op_direct;
		}
		return out;
	}
	
	out.type = op_invalid;
	return out;
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

// pack bytes into intel hex format with record size of 16
// :<byte_count><addr><record_type><bytes><checksum>
void pack_ihex(FILE *file_out,unsigned char hex_array[][2]){
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
	pack_ihex(hex_file,out_hex);	
	fclose(hex_file);
	return 0;
}
