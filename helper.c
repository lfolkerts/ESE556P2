#include <stdio.h>
#include <stdlib.h>
#include<stdint.h>
#include <ctype.h>
#include "node.h"
#include "helper.h"

//local helper function
int to_integer(char * c);

int LogTwo(int i)
{
	int ret;
	assert(i >= 0);
	for(ret = 0; i !=0; ret++){ i = i>>1; }
	return ret;
}


char GetOrientation(FILE* go_file)
{
       int ret, i;
       char s[2], flipped_xor;
	flipped_xor =0;
        while(1)
	{	
		s[0] =(char)fgetc(go_file); //wait for first digit
        	switch(s[0])
		{
			case EOF:
				return 0;
			case '#':
				Endline(go_file);
				break;//endswtich
			case 'N':
			case 'n':
				return (flipped_xor ^ OR_N);	
			case 'S':
                        case 's':
                                return (flipped_xor ^ OR_S);
			case 'E':
                        case 'e':
                                return (flipped_xor ^ OR_E);
			case 'W':
                        case 'w':
                                return (flipped_xor ^ OR_W);
			case 'F':
			case 'f':
				flipped_xor = OR_F;
				continue; //go back to while (skip resetting the flag below)
			default:
				break; //end switch
		}
		flipped_xor = 0; //direction should be immeadiately after next character
					
        }
        for(i=1; i<10 && isdigit(s[i] = (char)fgetc(go_file)); i++);
        s[i] = '\0'; //null terminate
        return to_integer(s);
}

int GetNextInt(FILE* gni_file)
{
	int ret, i;
	char s[10];
	char negative_flag;
	//	while(scanf("%d", &ret)<=0); //scanf f stops working after ~400 integer scans
	negative_flag = 0;
	while(!isdigit((s[0]=(char)fgetc(gni_file)))) //wait for first digit
	{
		if(s[0] == '#') { Endline(gni_file); } //remove file
		if(s[0] == '-') { negative_flag = 1; }
		else { negative_flag = 0; }
	}
	for(i=1; i<10 && isdigit(s[i] = (char)fgetc(gni_file)); i++);
	s[i] = '\0'; //null terminate
	ret = to_integer(s);
	if(negative_flag==0) return ret;
	else{ return -ret; }
}

char GetSymmetry(FILE* gs_file)
{
	char s;
	while(!isalpha((s=(char)fgetc(gs_file)))) //wait for first digit
        {
                if(s == '#') { Endline(gs_file); } //remove file
        }
	return s;
}

/************* get_line ********************
 * Gets one line from infile
 *
 * returns length of line on sucess, negative value on error
 **************************************************/
int  Endline(FILE* el_file)
{
	unsigned char c[1];
	int i, err;

	while(1) //avoid end of file and line
	{
		*c = fgetc(el_file);
		if(*c=='\n' || *c == EOF) break;
	}

	return i;
}


/************** to_integer *********************
 * Converts a string to an integer
 *
 * c - numeric string to conver
 *
 * returns the integer
 ***********************************************/
int to_integer(char * c)
{
	int i, val;
	i= val = 0;
	while(c[val]!='\0' && c[val]<='9' && c[val]>='0')
	{
		i+= (unsigned int) c[val] - '0';
		i*=10;
		val++;
	}
	i/=10;
	return i;
}

/************** help ***************
 * prints out help message
 ***********************************/
void Help()
{
	fprintf(stdout, "request_help");

}

/***********request_help ***********
 * suggests to confused users that the seek help
 ***********************************/

void inline RequestHelp()
{
	fprintf(stdout,  "Use \"./algo -h\" for help");
}
