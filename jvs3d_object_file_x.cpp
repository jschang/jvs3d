//#include <mysql\mysql.h>
#include <stdio.h>

#include "jvslib.h"
#include "jvs3dlib.h"

bool jvs3d_file_x::readList(int elem_per)
{
	unsigned long verts;
	int ret;
	readNext();
	if(isNumeric(s_jvs_cur))
	{
		verts = convertToNumber(s_jvs_cur);
		printf("%u",(int)verts);
		while(verts)
		{
			for(int x = 0; x<elem_per; x++)
			{
				ret = readNext();
				if( ret!=JVS_OP_SEMICOLON ) 
				{ errorSet(JVSERR_PARSE,"Expecting a semicolon."); return false; }
				printf("%s,",s_jvs_cur);
			}
			ret = readNext();
			if( ret != JVS_OP_COMMA ) return false;
			verts--;
		}
	} else return false;
	return true;
}
