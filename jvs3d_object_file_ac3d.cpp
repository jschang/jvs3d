#ifdef _MSC_VER
	//#include <afxwin.h>         // MFC core and standard components
	//#include <afxext.h>         // MFC extensions
#endif

#include <iostream>
#include <stdio.h>
#include <string.h>

#ifndef _MSC_VER // doesn't exist in windows, don't include it.
	#include <lcms.h>
#endif

#include "jvslib.h"
#include "jvs3dlib.h"

bool jvs3d_file_ac3d::loadFile(const char *pathfile, jvs3d_object *obj)
{
	char cmd[50];
	int version;
	FILE *fd=null;
	int ret=0;
	
	if(obj) m_obj=obj;
	else if(m_obj) obj=m_obj;
	
	if(DBGFAC3D) printf("%s ",pathfile);
	if(	openFile(pathfile) )
	{
		fd=file_jvs_tp_fd;
		if(fscanf(fd,"AC3D%x ",&version))
		{
			if(version==11)
			{
				// you have entered the inner loop!		
				fscanf(fd,"%s ",cmd);
				ret = returnType((char *)cmd);
				if(DBGFAC3D)
					printf("%s ",cmd);
				while( !isEOF() && ret )
				{
					switch(ret)
					{
						case JVS3D_FILE_AC3D_OBJECT:

							if(DBGFAC3D)
								printf("\nadvise: parsing object\n");
							ret = 0;
							ret = loadObject();

							break;

						case JVS3D_FILE_AC3D_MATERIAL:

							if(DBGFAC3D) 
								printf("\nadvise: parsing material\n");
							ret = 0;
							ret = loadMaterial();

							break;

						default:

							if(DBGFAC3D) 
								printf("\nfile done\n");
							return true;

							break;
					} // switch(ret)
				} // while( !isEOF() && ret )
			} else { if(DBGFAC3D) printf("error: version %d, not an ac3d file parsable by this program.\n",version); return false; }
		} else { if(DBGFAC3D) printf("error: may not be an ac3d text file.\n"); return false; }
	} else { if(DBGFAC3D) printf("error: file could not be opened\n"); return false; }
	return true;
}

int jvs3d_file_ac3d::returnType(const char *cmd)
{
	if(stricmp("object",cmd)==0) return JVS3D_FILE_AC3D_OBJECT;
	if(stricmp("material",cmd)==0) return JVS3D_FILE_AC3D_MATERIAL;
	return false;
}

int jvs3d_file_ac3d::loadMaterial(void)
{
	if(!m_obj) return false;
	FILE *fd=file_jvs_tp_fd;
	char name[40];
	char cmd[40];
	float f[4];
	unsigned long d[4];
	unsigned int ret=1;
	fscanf(fd,"%s ",name);
	if(DBGFAC3D) printf("name=%s ",name);
	while( !isEOF() )
	{
		fscanf(fd,"%s ",cmd);
		if(DBGFAC3D) printf("%s ",cmd);
		ret=1;
		if(stricmp("rgb",cmd)==0)	ret=fscanf(fd,"%f %f %f ",&f[0],&f[1],&f[2]);
		if(stricmp("amb",cmd)==0)	ret=fscanf(fd,"%f %f %f ",&f[0],&f[1],&f[2]);
		if(stricmp("shi",cmd)==0)	ret=fscanf(fd,"%d ",&d[0]);
		if(stricmp("spec",cmd)==0)	ret=fscanf(fd,"%f %f %f ",&f[0],&f[1],&f[2]);
		if(stricmp("emis",cmd)==0)	ret=fscanf(fd,"%f %f %f ",&f[0],&f[1],&f[2]);
		if(stricmp("trans",cmd)==0)	ret=fscanf(fd,"%f ",&f[0]);
		if(!ret) return false;
		if(ret=returnType(cmd)) return ret;
	}		
}

int jvs3d_file_ac3d::loadObject(void)
{
	if(!m_obj) return false;
	FILE *fd=file_jvs_tp_fd;
	bool oversucc=false;
	char type[40],cmd[40],name[40],texture[40];
	char url[40];
	float f[10];
	unsigned long d[10];
	unsigned int ret=1;
	uint i=0;
	uint *l=null;
	fscanf(fd,"%s ",type);
	uint polyidxoff = m_obj->verticeCount();
	if(DBGFAC3D) printf("cur vert count=%u ",polyidxoff);
	if(DBGFAC3D) printf("type=%s ",type);
	while( !isEOF() )
	{
		fscanf(fd,"%s ",cmd);
		if(DBGFAC3D) printf("%s ",cmd);
		ret=1;
		if(stricmp("kids",cmd)==0)		ret = fscanf(fd,"%d ",&d[0]);
		if(stricmp("texture",cmd)==0)	ret = fscanf(fd,"%s ",texture);
		if(stricmp("texrep",cmd)==0)	ret = fscanf(fd,"%f %f ",&f[0],&f[1]);
		if(stricmp("crease",cmd)==0)	ret = fscanf(fd,"%d ",&d[0]);
		if(stricmp("rot",cmd)==0)		ret = fscanf(fd,"%f %f %f %f %f %f %f %f %f ",&f[0],&f[1],&f[2],&f[3],&f[4],&f[5],&f[6],&f[7],&f[8]);
		if(stricmp("loc",cmd)==0)		ret = fscanf(fd,"%f %f %f ",&f[0],&f[1],&f[2]);
		if(stricmp("url",cmd)==0)		ret = fscanf(fd,"%s ",url);
		if(stricmp("name",cmd)==0) 		ret = fscanf(fd, "%s ", name);
		if(stricmp("data",cmd)==0) 		ret = fscanf(fd, "%d %s ",&d[0],name);
	
		if(!ret) return false;

		if(stricmp("numvert",cmd)==0)
		{
			fscanf(fd,"%d ",&d[0]);
			while(d[0])
			{
				fscanf(fd,"%f %f %f ",&f[0],&f[1],&f[2]);
				m_obj->addVertice(f[0],f[1],f[2]);
				d[0]--;
			}
		}
		if(stricmp("numsurf",cmd)==0)
		{
			fscanf(fd,"%d ",&d[0]);
			while(d[0])
			{
				fscanf(fd,"SURF 0x%x ",&d[1]);
				fscanf(fd,"mat %d ",&d[2]);
				fscanf(fd,"refs %u ",&d[3]);
				if(DBGPOLY) printf("refs: %u ",d[3]);
				l=new unsigned int[d[3]];
				for(i=0;i<d[3];i++)
				{
					fscanf(fd,"%u %f %f ",(unsigned int *)&l[i],&f[0],&f[1]);
					if(DBGPOLY) printf( "idx: %u : %u ", l[i],i);
				}
				m_obj->addPolygon(0,0,d[3],l);
				delete l;
				d[0]--;
			}
		}
		if(ret=returnType(cmd)) return ret;
	}
}
