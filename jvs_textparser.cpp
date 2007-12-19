#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "jvslib.h"

/****************************
 * Construction/Destruction *
 ****************************/

jvs_textparser::jvs_textparser()
{
	ul_jvs_type_id=JVS_CT_TEXTPARSER;
	file_jvs_tp_fd=null;
	s_jvs_cur[0]='\0';
}

jvs_textparser::~jvs_textparser()
{
	if(file_jvs_tp_fd)
		fclose(file_jvs_tp_fd);
}

/*****************
 * Basic File IO *
 *****************/

bool jvs_textparser::openFile(const char *file)
{
	file_jvs_tp_fd = fopen(file,"rt");
	if(!file_jvs_tp_fd)
	{
		file_jvs_tp_fd=0;
		return false;
	} else return true;
}

bool jvs_textparser::closeFile()
{
	if(file_jvs_tp_fd)
		fclose(file_jvs_tp_fd);
	return true;
}

bool jvs_textparser::isEOF(){ if(feof(file_jvs_tp_fd)) return true; else return false; }

/******************
 * Read Next Item *
 ******************/

unsigned long jvs_textparser::readNext(void)
// reads next item in an open file
// if it's a number it attempts to convert and return it
// returns 0 on fails and set jvsError/jvsErrNum
{
	unsigned long bytes=0;
	unsigned long op=0;
	char *tmp=null;
	char *last=null;
	char cc;	// current char

	errorReset();

	memset(s_jvs_cur,0,255);
	s_jvs_cur[0]=' ';
	s_jvs_cur[1]='\0';

	if(!file_jvs_tp_fd)
	{
		errorSet(JVSERR_NOFILE,"No file has been opened.");
		return false;
	}

	if(!isEOF())
	{
		cc = fgetc(file_jvs_tp_fd);
		while( ! feof(file_jvs_tp_fd) ) 
		{
			// if it's an operator, then return that.
			if(op=isOperator(cc)) 
				return op;
			// space or newline?  don't append it; return if we have a string length already
			if( cc == '\n' || cc == ' ' )
			{
				if(trimStrLen(s_jvs_cur))
					return bytes;	
			} else
			{
				s_jvs_cur[bytes]=cc;
				s_jvs_cur[bytes+1]='\0';
				bytes++;
			}
			
			// get the next character
			cc = fgetc(file_jvs_tp_fd);
		} 
	} else { errorSet(JVSERR_EOF,"File end reached, no more input."); return false; }
	return true;
}

bool jvs_textparser::isNumeric(const char *str)
{
	bool neg=false;
	bool dec=false;
	bool num=true;
	// determine if it's negative
	if(str[0]=='-') 
		neg=true;
	// cycle through and verify it as being numeric
	for(int x=(str[0]=='-')?1:0; x <= strlen(str)-1; x++)
		if( (str[x]>='0' && str[x]<='9') || // numbers
			(str[x]=='.' && dec==false) )
		{
			if( str[x]=='.' ) 
				dec=false;
		} else num=false;
	return num;
}
double jvs_textparser::convertToNumber(const char *str)
{return atof(str);}

bool jvs_textparser::isAlphaChar(char c)
{
	if( (c>='A' && c<='Z') || (c>='a' && c<='z') )
		return true;
	else return false;
}

unsigned long jvs_textparser::isOperator(char c)
{
	const unsigned long operators[JVS_OP_LENGTH][2]=
		{	{'(',JVS_OP_OPAR},
			{')',JVS_OP_CPAR},
			{'{',JVS_OP_OBRACE},
			{'}',JVS_OP_CBRACE},
			{'[',JVS_OP_OBRACK},
			{']',JVS_OP_CBRACK},
//			{'<',JVS_OP_LT},
//			{'>',JVS_OP_GT},
			{';',JVS_OP_SEMICOLON},
			{',',JVS_OP_COMMA},
//			{'*',JVS_OP_ASTERICK},
//			{'/',JVS_OP_FSLASH},
//			{'\\',JVS_OP_BSLASH},
//			{'|',JVS_OP_PIPE},
//			{'+',JVS_OP_PLUS},
//			{'-',JVS_OP_MINUS},
//			{'!',JVS_OP_EXCLAIM},
//			{'=',JVS_OP_EQUAL},
//			{'~',JVS_OP_TILDA} 
		};
	for(int x=0; x <= JVS_OP_LENGTH; x++)
		if( ((char)operators[x][0])==c )
			return operators[x][1];
	return 0;
}

int jvs_textparser::trimStrLen(const char *str)
{
	int x=0;
	int initspaces=0, trailspaces=0;
	while( x <= strlen(str)-1 )
	{
		if(str[x]!=' ')
		{
			initspaces=x;
			break;
		} else x++;
	}
//printf("%i",initspaces);
	x=strlen(str)-1;
	while( x >= 0 )
	{
		if(str[x]!=' ')
		{
			trailspaces=x+1;
			break;
		} else x--;
	}
//printf(",%i,",trailspaces);
	return trailspaces-initspaces;
}

char * jvs_textparser::getTagContent(FILE *fp)
{
	char last_char=0;
	char *toret=null;
	int len=0;
	while(last_char!='<')
	{
		last_char=fgetc(fp);
		if(last_char!='<')
		{
			len++;
			toret=(char*)realloc(toret,len+1);
			toret[len-1]=last_char;
			toret[len]=0;
		} else fseek(fp,-1,SEEK_CUR);
	}
	return toret;
}

char * jvs_textparser::getNextTag(FILE *fp)
{
	char *toret=null;
	int size=0;	
	bool inTag=0;
	bool stop=0;
	char last_char=0;
	while( !(feof(fp) || stop ) )
	{
		last_char=fgetc(fp);

		if(last_char=='>')
			stop=true;
		if(inTag && !stop)
		{
			size++;
			toret=(char*)realloc(toret,size+1);
			toret[size-1]=last_char;
			toret[size]=0;
		}
		if(last_char=='<')
			inTag=true;
	}
	return toret;
}

char * jvs_textparser::getCDATATag(FILE *fp)
{
	// file should be positioned right at or after the leading "<!"
	char tag[16];
	int x=0;
	int i=0;
	char last_char=0;
	char *content=null;
	int contentSize=0;
	if(fgetc(fp)=='<')
	{
		fgetc(fp);
		fread(tag,sizeof(char),7,fp);
	} else { fseek(fp,-1,SEEK_CUR); fread(tag,sizeof(char),7,fp); }
	if(stricmp(tag,"[CDATA[")==0)
	{
		// if it's cdata read the rest
		while(stricmp(tag,"]]>")!=0)
		{
			if(last_char)
			{
				contentSize++;
				content=(char*)realloc(content,contentSize+1);
				content[contentSize-1]=last_char;
				content[contentSize]=0;

				if(x<2) x++;
				else for(i=0;i<x;i++)
				tag[i]=tag[i+1];
			}

			last_char=tag[x]=fgetc(fp);	// set current index to current char
			tag[x+1]=0;			// set terminating null

		}
		if(contentSize)
		{
			contentSize-=2;
			content=(char *)realloc(content,contentSize+1);
			content[contentSize]=0;
		}
	} else
	{
		// rewind and return null;
		fseek(fp, -9, SEEK_CUR);
		return null;
	}
	return content;
}

bool jvs_textparser::inStr(const char *needle, const char *haystack)
{
	int m=0, i=0;
	int nlen=strlen(needle);
	while(haystack[i]!='\0')
	{
		if(haystack[i]==needle[m])
			m++;
		if(m==nlen)
			return true;
		i++;
	}
}
