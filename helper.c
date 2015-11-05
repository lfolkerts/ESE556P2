#include <unistd.h> //getopt
#include <getopt.h> //optind
#include <stdio.h>
#include  <fcntl.h>
#include <stdlib.h>
#include<stdint.h>
#include <ctype.h>
#include "my_header.h"


static int getint();
static int Endline();
static int to_integer(char * c);
static void help();
static void inline request_help();

static char GetOrientation(FILE* go_file)
{
       int ret, i;
        char s[2];
	//      while(scanf("%d", &ret)<=0); //scanf f stops working after ~400 integer scans
        while(1)
	{	
		s =(char)fgetc(go_file); //wait for first digit
        	switch(s)
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
        for(i=1; i<10 && isdigit(s[i] = (char)fgetc(gni_file)); i++);
        s[i] = '\0'; //null terminate
        return to_integer(s);
}

static int GetNextInt(FILE* gni_file)
{
	int ret, i;
	char s[10];
	//	while(scanf("%d", &ret)<=0); //scanf f stops working after ~400 integer scans
	while(!isdigit((s[0]=(char)fgetc(gni_file)))) //wait for first digit
	{
		if(s[0] == '#') { Endline(gni_file); } //remove file
	}
	for(i=1; i<10 && isdigit(s[i] = (char)fgetc(gni_file)); i++);
	s[i] = '\0'; //null terminate
	return to_integer(s);
}



/************* get_line ********************
 * Gets one line from infile
 *
 * returns length of line on sucess, negative value on error
 **************************************************/
static int  Endline(FILE* el_file)
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
static int to_integer(char * c)
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
static void Help()
{
	fprintf(stdout, "request_help");

}

/***********request_help ***********
 * suggests to confused users that the seek help
 ***********************************/
static void inline RequestHelp()
{
	fprintf(stdout,  "Use \"./algo -h\" for help");
}
