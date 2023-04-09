#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "asm51.h"
#include "instructions.c"

#define DEBUG

const struct sfr defined_vars[] = {
   {"acc",0xe0,true},
   {"psw",0xd0,true},
   {"p0",0x80,true},
   {"p1",0x90,true},
   {"p2",0xa0,true},
   {"p3",0xb0,true},
   {"ie",0xa8,true},
   {"ip",0xb8,true},
   {"tcon",0x88,true},
   {"scon",0x98,true},
   {"sbuf",0x99,true},
   {"dpl",0x02,false},
   {"dph",0x03,false},
   {"sp",0x81,false},
   {"tl0",0x8a,false},
   {"tl1",0x8b,false},
   {"th0",0x8c,false},
   {"th1",0x8d,false},
   {"pcon",0x87,false},
   {"tmod",0x89,false},
   {"b",0xf0,false}
};

const char * all_operands[] = {
	"a","@r0","@r1","r0","r1","r2","r3","r4","r5","r6","r7",
	"c","@a+dptr","ab","@a+pc","dptr","@dptr","","addr11",
	"addr16","offset","bit","direct","immed","label",
};

const char * all_mnemonics[] = {
	"nop","ajmp","ljmp","rr","inc","jbc","acall","lcall",
	"rrc","dec","jb","ret","rl","add","jnb","reti","rlc",
	"addc","jc","orl","jnc","anl","jz","xrl","jnz","jmp",
	"mov","sjmp","movc","div","subb","mul","cpl","cjne",
	"push","clr","swap","xch","pop","setb","da","djnz",
	"xchd","movx",""
};

enum mnemonic_type get_mnemonic_enum(char *word){
	// check through all possible mnemonics
    for(int i=0;i<mn_invalid;i++){
		if(!strcmp(all_mnemonics[i],word)){
            return i;
		}
	}
    return mn_invalid; // return invalid if no matching mnemonic
}

int checklabel(char *label){
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

int checksfr(char *word,struct operand *out){
	// check through all sfr
    for(int i=0;i<20;i++){
		if(!strcmp(defined_vars[i].name,word)){
			out->type = op_direct;
			out->value = defined_vars[i].addr;
			return 1;
		}
	}
	return 0;
}

// base - 2,10,16 only
int strtoint(int *out_val,char *str,int start,int end,int base){
	for(int i=end,j=1;i>=start;i--,j*=base){
		char w = str[i];
		int iw = w-'0';	// char to int (0-9)
		bool is_digit = isdigit(w);
		if(base==16){
			if(w<='f' && w>='a'){
				iw = w-87;	// hex char to int (a-f)
			}
			else if(!is_digit){
				return 0;
			}
		}
		else if(base<=10){
			if(!is_digit || (base==2 && iw>1)){
				return 0;
			}
		}
		*out_val += iw*j;
	}
	return 1;
}

struct operand get_operand_mnemonic(char *word){
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
	if(checksfr(word,&out)){
		return out;
	}

	// check for value based operands - label,direct,bit,addr,offset,immed
	if(checklabel(word)){
		out.type = op_label;
		return out;
	}

	int base = 10;
	int start = 0;
	int end = strlen(word) - 1;
	if(word[0]=='#'){
		out.type = op_immed;
		start = 1;
	}
	if(word[end]=='h'){
		base = 16;
		end--;
	}
	if(word[start]=='0' && word[start+1]=='x'){
		base = 16;
		start += 2;
	}
	else if(word[end]=='b'){
		base = 2;
		end--;
	}
	if(strtoint(&out.value,word,start,end,base)){
		if(out.value>0xff){
			out.type = op_addr16;
		}
		else if(out.type!=op_immed){
			out.type = op_direct;
		}
	}
	else{
		out.type = op_invalid;
	}

	return out;
}

char get_token(FILE *file,char *out_str,char end_char){
	char ch;
	int i = 0;
	while(1){
		ch = fgetc(file);
		if(ch==';'){
			// if comment skip to eol
			while(fgetc(file)!='\n'){ }
			ch = '\n';
			break;
		}
		if(ch==end_char || ch==':' || ch=='\n' || feof(file)){
			// if : break (check last char outside the func to detect label)
			// else break if eol/eof
			break;
		}
		if((ch==' ' || ch=='\t')){
			if(end_char==',' || i==0){
				// skip spaces and tabs if scanning for operands or if mnemonic not found
				continue;
			}
		}
		out_str[i++] = tolower(ch);
	}
	out_str[i]='\0'; // end string
	return ch;	// return last char
}

void assemble(char *mnemonic,char operands[3][16],int *addr){
	enum mnemonic_type mn = get_mnemonic_enum(mnemonic);
	struct operand op[] = {{op_invalid,0},{op_invalid,0},{op_invalid,0},{op_invalid,0}};
	int data[2] = {-1,-1};

	for(int i=0,j=0;i<3;i++){
		op[i] = get_operand_mnemonic(operands[i]);
		if(op[i].value>-1){
			data[j++] = op[i].value;
		}
	}


	int instr = findinstruction(mn,op);

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


#ifdef DEBUG
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
#endif
}

void pushlabelsrc(struct labels *lb,char *label,int addr,int *array_size){
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
		printf("%s: file does not exist\n",file_loc);
		return 0;
	}

	int line_count = 0;
	int addr = 0x0000;

	struct labels all_labels[50];
	int label_pos[50];
	label_pos[0] = 0; //first element is size of array
	int label_count = 0;

	char label[16];
	char mnemonic[16];
	char operands[3][16];
	int operand_count;

	char ch;
	int errors[20];

	while(1){
		// clear all previous value
		label[0] = '\0';
		mnemonic[0] = '\0';
		for(int i=0;i<3;i++){
			operands[i][0] = '\0';
		}
		operand_count = 0;

		// get mnemonic or label
		ch = get_token(asm_file,mnemonic,' ');
		if(ch==':'){	// if label is found
			pushlabelsrc(all_labels,mnemonic,addr,&label_count);
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

	printf("\n");
	fclose(asm_file);
	return 0;
}
