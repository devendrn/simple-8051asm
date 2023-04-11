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
   {"dpl",0x02,0},
   {"dph",0x03,0},
   {"sp",0x81,0},
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
				return 0;
			}
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

	for(int i=0;i<20;i++){	// check all sfr
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

// assemble into char array
void assemble(char *mnemonic,char operands[3][16],int *addr){
	enum mnemonic_type mn = get_mnemonic_enum(mnemonic);
	struct operand op[] = {{op_invalid,0},{op_invalid,0},{op_invalid,0},{op_invalid,0}};
	int data[2] = {-1,-1};
	int j = 0;	// num of operands with data
	for(int i=0;i<3;i++){
		op[i] = get_operand_struct(operands[i]);
		if(op[i].value>-1){
			data[j++] = op[i].value;
		}
	}

	int instr = get_opcode(mn,op);
	
	if(instr!=0xA5){	// increment address by instruction size
		*addr += 1+j;
	}

	// for debug
	printf(" %2x",instr);
	if(data[0]>-1){
		printf(" %2x",data[0]);
	}
	else{
		printf("   ");
	}
	if(data[1]>-1){
		printf(" %2x",data[1]);
	}else{
		printf("   ");
	}
	printf(" | ");
	if(mn!=mn_none){
		printf(" m(%6s/%6s)",mnemonic,all_mnemonics[mn]);
		for(int i=0;i<3;i++){
			printf(" o%d(%8s/%8s:%4d)",i,operands[i],all_operands[op[i].type],op[i].value);
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
	for(i=0;i<*array_size;i++){
		if(!strcmp(lb[i].name,label)){
			lb[i].addr = addr;
			return;
		}
	}
	memcpy(lb[i].name,label,strlen(label)+1);
	lb[i].addr = addr;
	*array_size = *array_size + 1;
}

int main(int argc, char **argv){

	char *file_loc;
	char *file_dest;

	if(argc==1){
		printf("No arguments given: Use -h for help\n");
		return 1;
	}
	else if(argc==2){
		if(argv[1][0]=='-'){
			if(argv[1][1]=='h'){
				printf("Usage:\nasm51 [input_file] -o [output_file]\n");
				return 1;
			}
			printf("Invalid arguments: Use -h for help\n");
			return 1;
		}
		 file_loc = argv[1];
	}
	else if(argc==4 && strcmp("-o",argv[2])){
		file_loc = argv[1];
		file_dest = argv[3];
	}
	else{
		printf("Invalid arguments: Use -h for help\n");
		return 1;
	}

	FILE *asm_file = fopen(file_loc,"r");

	if(asm_file == NULL){
		printf("File '%s' not found\n",file_loc);
		return 0;
	}

	int line_count = 0;
	int addr = 0x0000;

	struct labels all_labels[256];	// index and name of all labels 
	int label_pos[256];	// byte pos of all labels
	label_pos[0] = 0; //first element is size of array
	int label_count = 0;

	char label[16];
	char mnemonic[16];
	char operands[3][16];
	int operand_count;
	char out_hex[256];
	char ch;
	int errors[20];

	while(1){
		// clear all previous value
		label[0] = '\0';
		mnemonic[0] = '\0';
		operands[0][0] = '\0';
		operands[1][0] = '\0';
		operands[2][0] = '\0';
		operand_count = 0;

		// get mnemonic or label
		ch = get_token(asm_file,mnemonic,' ');
		if(ch==':'){	// if label is found
			push_label_src(all_labels,mnemonic,addr,&label_count);
			// memcpy(label,mnemonic,strlen(mnemonic)+1);
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
		assemble(mnemonic,operands,&addr);
	}

	printf("\nLabels:");
	for(int i=0;i<label_count;i++){
		printf("\nl%d:%7s | %4x",i,all_labels[i].name,all_labels[i].addr);
	}
	//printf("\nLabel pos");
	//for(int i=0;i<label_count;i++){
	//	printf("%d ",i);
	//}
	printf("\n");
	fclose(asm_file);
	return 0;
}
