#ifdef _MSC_VER
	//#include <afxwin.h>         // MFC core and standard components
	//#include <afxext.h>         // MFC extensions
	#include <windows.h>
#else*/
	#include <lcms.h>	// for _stricmp
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <string>

#ifdef _MSC_VER
	#include <GL/gl.h> 			//OPENGL
	#include <GL/glu.h>			//OPENGL
	#include <GL/glaux.h>		//OPENGL
#endif

#include "jvslib.h"
#include "jvs3dlib.h"

/***************************************************************
 * store - XML serialization - all classes
 ***************************************************************/

bool jvs3d_material::storeXMLFileList(FILE *fp)
{
	jvs3d_material *mptr=(jvs3d_material*)getTop();
	fprintf(fp,"<material_list>");
	while(mptr)
	{
		mptr->storeXMLFile(fp);
		mptr=(jvs3d_material*)mptr->getNext();
	}
	fprintf(fp,"</material_list>");
	return true;
}
bool jvs3d_material::storeXMLFile(FILE *fp)
{
	fprintf(fp,"<material>");
	switch(m_materialType)
	{
	case JVS3D_MAT_COLOR:
		fprintf(fp,"<name><![CDATA[%s]]></name>",getName());
		fprintf(fp,"<type>color</type>");
		fprintf(fp,"<color>%lf,%lf,%lf</color>");
		break;
	case JVS3D_MAT_TEXTURE:
		fprintf(fp,"<name><![CDATA[%s]]></name>",getName());
		fprintf(fp,"<type>image</type>");
		fprintf(fp,"<file><![CDATA[%s]]></file>",m_filepath);
		break;
	}
	fprintf(fp,"</material>");
	return true;
}
bool jvs3d_armature::storeXMLFileList(FILE *fp)
{
	jvs3d_armature *parm = (jvs3d_armature*)getTop();
	fprintf(fp,"<armature_list>");
	while(parm)
	{
		parm->storeXMLFile(fp);
		parm=(jvs3d_armature*)parm->getNext();
	}
	fprintf(fp,"</armature_list>");
	return true;
}
bool jvs3d_armature::storeXMLFile(FILE *fp)
{
	unsigned int index=0;
	bool last=false;
	fprintf(fp,"<armature>");
	fprintf(fp,"<name><![CDATA[%s]]></name>",getName());
	fprintf(fp,"<pivot>%lf,%lf,%lf</pivot>",m_pivot.m_data[0],m_pivot.m_data[1],m_pivot.m_data[2]);
	if(m_parentArmature)
		fprintf(fp,"<parent><![CDATA[%s]]></parent>",m_parentArmature->getName());
	fprintf(fp,"</armature>");
	return true;
}
bool jvs3d_armupdate::storeXMLFileList(FILE *fp)
{
	jvs3d_armupdate *pau=(jvs3d_armupdate *)getTop();
	while(pau)
	{
		pau->storeXMLFile(fp);
		pau=(jvs3d_armupdate*)pau->getNext();
	}
	return true;
}
bool jvs3d_armupdate::storeXMLFile(FILE *fp)
{
	fprintf(fp,"<armupdate>");
	if(m_armature)
		fprintf(fp,"<armature><![CDATA[%s]]></armature>",m_armature->getName());
	fprintf(fp,"<rotation>%lf,%lf,%lf</rotation>",m_rotation[0],m_rotation[1],m_rotation[2]);
	fprintf(fp,"</armupdate>");
	return true;
}
bool jvs3d_cycle::storeXMLFileList(FILE *fp)
{
	jvs3d_cycle *pcyc=(jvs3d_cycle*)getTop();
	fprintf(fp,"<cycle_list>");
	while(pcyc)
	{
		pcyc->storeXMLFile(fp);
		pcyc=(jvs3d_cycle*)pcyc->getNext();
	}
	fprintf(fp,"</cycle_list>");
	return true;
}
bool jvs3d_cycle::storeXMLFile(FILE *fp)
{
	fprintf(fp,"<cycle>");
	fprintf(fp,"<name><![CDATA[%s]]></name>",getName());
	if(m_frames)
		m_frames->storeXMLFileList(fp);
	fprintf(fp,"</cycle>");
	return true;
}
bool jvs3d_frame::storeXMLFileList(FILE *fp)
{
	jvs3d_frame *pfr=(jvs3d_frame*)getTop();
	fprintf(fp,"<frame_list>");
	while(pfr)
	{
		pfr->storeXMLFile(fp);
		pfr=(jvs3d_frame*)pfr->getNext();
	}
	fprintf(fp,"</frame_list>");
	return true;
}
bool jvs3d_frame::storeXMLFile(FILE *fp)
{
	fprintf(fp,"<frame>");
	if(m_armUpdates)
		m_armUpdates->storeXMLFileList(fp);
	fprintf(fp,"</frame>");
	return true;
}

bool jvs3d_object::storeXMLFileList(FILE *fp)
{
	jvs3d_object* pobj=(jvs3d_object*)getTop();
	fprintf(fp,"<object_list>");
	while(pobj)
	{
		pobj->storeXMLFile(fp);
		pobj=(jvs3d_object*)pobj->getNext();
	}
	fprintf(fp,"</object_list>");
	return true;
}
bool jvs3d_object::storeXMLFile(FILE *fp)
{
	fprintf(fp,"<object>");
	fprintf(fp,"<name><![CDATA[%s]]></name>",getName());
	fprintf(fp,"<rotate>%lf,%lf,%lf</rotate>",m_rotate.m_data[0],m_rotate.m_data[1],m_rotate.m_data[2]);
	fprintf(fp,"<translate>%lf,%lf,%lf</translate>",m_translate.m_data[0],m_translate.m_data[1],m_translate.m_data[2]);
	if(mp_vertices) mp_vertices->storeXMLFileList(fp);
	if(mp_polygons) mp_polygons->storeXMLFileList(fp);
	fprintf(fp,"</object>");
	return true;
}
bool jvs3d_polygon::storeXMLFileList(FILE *fp)
{
	jvs3d_polygon *ppoly=(jvs3d_polygon*)getTop();
	fprintf(fp,"<polygon_list>");
	while(ppoly)
	{
		ppoly->storeXMLFile(fp);
		ppoly=(jvs3d_polygon*)ppoly->getNext();
	}
	fprintf(fp,"</polygon_list>");
	return true;
}
bool jvs3d_polygon::storeXMLFile(FILE *fp)
{
	bool last=false;
	int i;
	fprintf(fp,"<polygon>");
	if(m_index)
	{
		fprintf(fp,"<vertices>");
		for(i=0;i<m_num;i++)
		{
			if(last) fputc(',',fp);
			fprintf(fp,"%u",m_index[i]);
			last=true;
		}
		fprintf(fp,"</vertices>");
	}
	fprintf(fp,"<texture_coords>");
	last=false;
	for(i=0;i<m_num;i++)
		fprintf(fp,"%lf,%lf;",m_txtrX[i],m_txtrY[i]);
	fprintf(fp,"</texture_coords>");
	if(m_materialPtr)
	{
		fprintf(fp,"<material><![CDATA[%s]]></material>",m_materialPtr->getName());
	}
	fprintf(fp,"</polygon>");
	return true;
}

/***************************************************************
 * load - XML serialization - all classes
 *
 * each routine returns the last character processed or null
 * and will be positioned at the beginning of the next tag
 ***************************************************************/

char jvs3d_item::loadXMLFile(FILE*fp)
{
/* 
between the leading tags
	<jvs3dAnimatedItemXML>...</jvs3dAnimatedItemXML>
there may be one each of four possible root nodes:
		<material_list>...</material_list>
		<cycle_list>...</cycle_list>
		<object_list>...</object_list>
		<armature_list>...</armature_list>
on opening tag, unless otherwise specified is followed by another
yielding the node tree of the XML document
text outside the tags will be ignored
*/
	int error=0;
	int stop=0;
	int inXML=0;
	char *tag=null;
	jvs3d_object *pobj=null;
	jvs3d_polygon *ppoly=null;
	jvs3d_vertice *pvert=null;
	jvs3d_armature *parm=null;
	jvs3d_cycle *pcyc=null;
	jvs3d_frame *pfra=null;
	jvs3d_armupdate *pau=null;

	while( !feof(fp) )
	{
		tag=null;
		tag=jvs_textparser::getNextTag(fp);
		if(tag)
		{
			if(_stricmp(tag,"jvs3dAnimatedItemXML")==0)
				inXML=1;
			if(_stricmp(tag,"/jvs3dAnimatedItemXML")==0)
			{
				stop=1;
				inXML=0;
			}
			if(inXML)
			{
				if(!m_objects && _stricmp(tag,"object_list")==0)
				{
					m_objects=new jvs3d_object;
					m_objects->loadXMLFileList(fp);
				}
				if(!m_materials && _stricmp(tag,"material_list")==0)
				{
					m_materials=new jvs3d_material;
					m_materials->loadXMLFileList(fp);
				}	
				if(!m_cycles && _stricmp(tag,"cycle_list")==0)
				{
					m_cycles=new jvs3d_cycle;
					m_cycles->loadXMLFileList(fp);
				}
				if(!m_armatures && _stricmp(tag,"armature_list")==0)
				{
					m_armatures=new jvs3d_armature;
					m_armatures->loadXMLFileList(fp);
				}
			}
			delete tag;
		}
	}
	if(m_objects)
	{
		pobj=m_objects;
		while(pobj)
		{
			ppoly=pobj->mp_polygons;
			if(m_materials)
				while(ppoly)
				{
					ppoly->m_materialPtr=(jvs3d_material*)m_materials->getByName(ppoly->getName());
					ppoly=(jvs3d_polygon*)ppoly->getNext();
				}
			if(m_armatures)
			{
				pvert=pobj->mp_vertices;
				while(pvert)
				{
					pvert->m_armature=(jvs3d_armature*)m_armatures->getByName(pvert->getName());
					pvert=(jvs3d_vertice*)pvert->getNext();
				}
			}
			pobj=(jvs3d_object*)pobj->getNext();
		}
	}
	if(m_armatures)
	{
		parm=m_armatures;
		while(parm)
		{
			if(parm->m_parentName)
				parm->m_parentArmature=(jvs3d_armature*)m_armatures->getByName(parm->m_parentName);
			parm=(jvs3d_armature*)parm->getNext();
		}
	}
	if(m_cycles && m_armatures)
	{
		pcyc=m_cycles;
		while(pcyc)
		{
			pfra=pcyc->m_frames;
			while(pfra)
			{
				pau=pfra->m_armUpdates;
				while(pau)
				{
					pau->m_armature=(jvs3d_armature*)m_armatures->getByName(pau->getName());
					pau=(jvs3d_armupdate*)pau->getNext();
				}
				pfra->m_parentCycle=pcyc;
				pfra=(jvs3d_frame*)pfra->getNext();
			}
			pcyc=(jvs3d_cycle*)pcyc->getNext();
		}
	}
	if(error) return false;
	else return true;
}
char jvs3d_object::loadXMLFileList(FILE *fp)
{
	// assumes <object_list> has already been read
	bool endTag=false;
	char *tag=null;
	jvs3d_object *ptr=null;
	while(! (feof(fp) || endTag) )
	{
		tag = jvs_textparser::getNextTag(fp);
		if(tag)
		{
			if(_stricmp(tag,"/object_list")==0)
				endTag=true;
			else if(_stricmp(tag,"object")==0)
			{
				/* object_list tag is composed of <object> tags only */
				if(!this->mp_vertices)
					ptr=this;
				else ptr=new jvs3d_object;
				if(ptr)
				{
					ptr->loadXMLFile(fp);

					appendEnd(ptr);
				}
			}
			delete tag;
		} else return false;
	}
	if(feof(fp)) return false;
	return true;
}
char jvs3d_object::loadXMLFile(FILE *fp)
{
	// assumes leading <object> tag has been read
	char *tag=null;
	char *name=null;
	bool endTag=false;	// master switch, true to end object
	bool cont=true;		// continue for inner loops
	jvs3d_vertice *pvert=null;
	char c[3];
	int ret=0;
	while( ! (feof(fp) || endTag) )
	{
		tag = jvs_textparser::getNextTag(fp);
		if(tag)
		{
			// if the object has a name
			if(_stricmp(tag,"name")==0)
			{
				name=jvs_textparser::getCDATATag(fp);
				setName( name );delete name;
				delete tag;tag=jvs_textparser::getNextTag(fp);
				if(_stricmp(tag,"/name")!=0) { delete tag; return false; }
			}
			// the translation of the object in item space
			else if(_stricmp(tag,"translate")==0)
			{
				fscanf(fp,"%lf,%lf,%lf",&m_translate.m_data[0],&m_translate.m_data[1],&m_translate.m_data[2]);
				tag = jvs_textparser::getNextTag(fp);
				if(_stricmp(tag,"/translate")!=0) { delete tag; return false;}
			} 
			// the translation of the object in item space
			else if(_stricmp(tag,"rotate")==0)
			{
				fscanf(fp,"%lf,%lf,%lf",&m_rotate.m_data[0],&m_rotate.m_data[1],&m_rotate.m_data[2]);
				tag = jvs_textparser::getNextTag(fp);
				if(_stricmp(tag,"/rotate")!=0) { delete tag; return false;}
			} 
			// the vertice list
			else if(_stricmp(tag,"vertice_list")==0)
			{
				cont=true;
				while(cont)
				{
					fread(c,sizeof(char),2,fp);
					c[2]=0;
					if(_stricmp(c,"</")==0)
					{
						fseek(fp,-2,SEEK_CUR);
						cont=false;
					} else
					{
						pvert=new jvs3d_vertice;
						if(_stricmp(c,"<!")==0)
						{
							name=jvs_textparser::getCDATATag(fp);
							pvert->setName(name);delete name;
						}
						ret=fscanf(fp,",%lf,%lf,%lf;",&pvert->m_data[0],&pvert->m_data[1],&pvert->m_data[2]);
						if(mp_vertices) mp_vertices->appendEnd(pvert);
						else mp_vertices=pvert;
					}
				}
				delete tag;tag=jvs_textparser::getNextTag(fp);
			} 
			// the polygon list
			else if(_stricmp(tag,"polygon_list")==0)
			{
				mp_polygons=new jvs3d_polygon;
				mp_polygons->loadXMLFileList(fp);
			} 
			// the end of the object
			else if(_stricmp(tag,"/object")==0)
			{ delete tag; return true; }
			delete tag;
		}
	}
	if(feof(fp)) return false;
	return true;
}

char jvs3d_polygon::loadXMLFileList(FILE *fp)
{
	char *tag=null;
	bool endTag=false;
	jvs3d_polygon *ppoly=null;
	// assumes <polygon_list> has already been read
	while( !(feof(fp) || endTag) )
	{
		tag = jvs_textparser::getNextTag(fp);
		if(_stricmp(tag, "/polygon_list")==0)
			endTag=true;
		else if (_stricmp(tag, "polygon")==0)
		{
			if( !m_index )
				ppoly=this;
			else ppoly=new jvs3d_polygon;
			if(ppoly)
			{
				if(ppoly->loadXMLFile(fp))
					appendEnd(ppoly);
				else return false;
			}
		}
		delete tag;
	}
	return true;
}
char jvs3d_polygon::loadXMLFile(FILE *fp)
{
	// assumes <polygon> has already been read
	char *tag=null;
	char *name=null;
	bool endTag=false;
	bool cont=true;
	int index;
	double txtrX, txtrY;
	char c;
	while(!(feof(fp) || endTag))
	{
		tag = jvs_textparser::getNextTag(fp);
		if(_stricmp(tag,"/polygon")==0)
			endTag=true;
		else if(_stricmp(tag,"vertices")==0)
		{
			m_num=0;
			m_index=0;
			cont=true;
			while(cont)
			{
				index=-1;
				fscanf(fp,"%d",&index);
				if(index>-1)
				{
					m_num++;
					m_index=(unsigned int *)realloc(m_index,(1+m_num)*sizeof(int));
					m_index[m_num-1]=index;
				}
				c=fgetc(fp);
				if(c=='<')
				{
					cont=false;
					fseek(fp,-1,SEEK_CUR);
				}
			}
		}
		else if(_stricmp(tag,"material")==0)
		{
			name=jvs_textparser::getCDATATag(fp);
			setName(name);
			delete name;
		} else if(_stricmp(tag,"texture_coords")==0)
		{
			index=0;
			cont=true;
			m_txtrX=(double *)realloc(m_txtrX,(1+m_num)*sizeof(double));
			m_txtrY=(double *)realloc(m_txtrY,(1+m_num)*sizeof(double));
			while(cont)
			{
				c=fscanf(fp,"%lf,%lf;",&txtrX,&txtrY);
				if(c)
				{
					m_txtrX[index]=txtrX;
					m_txtrY[index]=txtrY;
				}
				index++;
				c=fgetc(fp);
				fseek(fp,-1,SEEK_CUR);
				if(c=='<')
					cont=false;
			}
		}
		delete tag;
	}
	return true;
}

char jvs3d_material::loadXMLFileList(FILE *fp)
{
	// assumes openning material_list tag has been read
	char *tag=null;
	char *name=null;
	bool endTag=false;
	jvs3d_material *pmat=null;
	while(!endTag)
	{
		tag = jvs_textparser::getNextTag(fp);
		if( _stricmp(tag,"/material_list")==0 )
			endTag=true;
		if( _stricmp(tag,"material")==0 )
		{
			if(m_materialType)
				pmat=new jvs3d_material;
			else pmat=this;
			pmat->loadXMLFile(fp);
			appendEnd(pmat);
		}
		delete tag;
	}
	return true;
}
char jvs3d_material::loadXMLFile(FILE *fp)
{
	// assumes material tag has been read
	// name, type, color|file
	char *tag=null;
	char *name=null;
	bool endTag=false;
	while(! (feof(fp) || endTag) )
	{
		tag = jvs_textparser::getNextTag(fp);
		if( _stricmp(tag,"/material")==0 )
			endTag=true;
		else if( _stricmp(tag,"name")==0 )
		{
			name=jvs_textparser::getCDATATag(fp);
			setName(name);delete name;
			delete tag;tag = jvs_textparser::getNextTag(fp);
		}
		else if( _stricmp(tag,"type")==0 )
		{
			name=jvs_textparser::getTagContent(fp);
			if( _stricmp(name,"color") )
				m_materialType=JVS3D_MAT_TEXTURE;
			else if( _stricmp(name,"image") )
				m_materialType=JVS3D_MAT_COLOR;
			delete name;
			delete tag;tag = jvs_textparser::getNextTag(fp);
		}
		else if( _stricmp(tag,"file")==0 )
		{
			name=jvs_textparser::getCDATATag(fp);
			m_filepath=(char*)realloc(m_filepath,strlen(name)+1);
			memcpy(m_filepath,name,strlen(name)+1);
			initializeGL("tga",m_filepath);
			delete name;
			delete tag;tag = jvs_textparser::getNextTag(fp);
		}
		else if( _stricmp(tag,"color")==0 )
		{
			fscanf(fp,"%lf,%lf,%lf",&m_color[0],&m_color[1],&m_color[2]);
			delete tag;tag = jvs_textparser::getNextTag(fp);
		}
		delete tag;
	}
	return true;
}

///////////////////////
// BELOW IS UNTESTED //
///////////////////////

/*
loadXMLFileList
	char *tag=null
	bool endTag=false;
	while(!(feof(fp) || endTag))
	{
		tag = jvs_textparser::getNextTag(fp);
		if(_stricmp(tag,"/armature_list")==0)
			endTag=true;
		delete tag;
	}
	return true;
*/
char jvs3d_armature::loadXMLFileList(FILE *fp)
{
	// assumes leading armature_list has been read
	char *tag=null;
	bool endTag=false;
	jvs3d_armature *parm=null;
	while(!(feof(fp) || endTag))
	{
		tag = jvs_textparser::getNextTag(fp);
		if(_stricmp(tag,"/armature_list")==0)
			endTag=true;
		else if(_stricmp(tag,"armature")==0)
		{
			if(m_loaded) parm=new jvs3d_armature;
			else { parm=this; m_loaded=true; }
			parm->loadXMLFile(fp);
			appendEnd(parm);
		}
		delete tag;
	}
	return true;
}
char jvs3d_armature::loadXMLFile(FILE *fp)
{
	char *tag=null;
	bool endTag=false;
	char *name=null;
	while(! ( feof(fp) || endTag ) )
	{
		tag = jvs_textparser::getNextTag(fp);
		if(_stricmp(tag,"/armature")==0)
			endTag=true;
		else if(_stricmp(tag,"name")==0)
		{
			name=jvs_textparser::getCDATATag(fp);
			setName(name);
			delete name;
		} else if (_stricmp(tag,"pivot")==0)
			fscanf(fp,"%lf,%lf,%lf",&m_pivot.m_data[0],&m_pivot.m_data[1],&m_pivot.m_data[2]);
		else if(_stricmp(tag,"parent")==0)
			m_parentName=jvs_textparser::getCDATATag(fp);
		delete tag;
	}
	return true;
}

char jvs3d_cycle::loadXMLFileList(FILE *fp)
{
	char *tag=null;
	bool endTag=false;
	jvs3d_cycle *parm=null;
	char *name=null;
	while(!(feof(fp) || endTag))
	{
		tag = jvs_textparser::getNextTag(fp);
		if(_stricmp(tag,"/cycle_list")==0)
			endTag=true;
		else if(_stricmp(tag,"cycle")==0)
		{
			if(m_loaded) parm=new jvs3d_cycle;
			else { parm=this; m_loaded=true; }
			parm->loadXMLFile(fp);
			appendEnd(parm);
		}
		delete tag;
	}
	return true;
}
char jvs3d_cycle::loadXMLFile(FILE *fp)
{
	char *tag=null;
	bool endTag=false;
	char *name=null;

	while(! ( feof(fp) || endTag ) )
	{
		tag = jvs_textparser::getNextTag(fp);
		if(_stricmp(tag,"/cycle")==0)
			endTag=true;
		else if(_stricmp(tag,"name")==0)
		{
			name=jvs_textparser::getCDATATag(fp);
			setName(name);
			delete name;
		} else if(_stricmp(tag,"frame_list")==0)
		{
			if(!m_frames)
			{
				m_frames=new jvs3d_frame;
				m_frames->loadXMLFileList(fp);
			}
		}
		delete tag;
	}
	return true;
}

char jvs3d_frame::loadXMLFileList(FILE *fp)
{
	char *tag=null;
	bool endTag=false;
	char *name=null;
	jvs3d_frame *parm=null;

	while(!(feof(fp) || endTag))
	{
		tag = jvs_textparser::getNextTag(fp);
		if(_stricmp(tag,"/frame_list")==0)
			endTag=true;
		else if(_stricmp(tag,"frame")==0)
		{
			if(m_loaded) parm=new jvs3d_frame;
			else { parm=this; m_loaded=true; }
			parm->loadXMLFile(fp);
			appendEnd(parm);
		}
		delete tag;
	}
	return true;
}
char jvs3d_frame::loadXMLFile(FILE *fp)
{
	char *tag=null;
	bool endTag=false;
	jvs3d_armupdate *parm=null;
	while(!(feof(fp) || endTag))
	{
		tag = jvs_textparser::getNextTag(fp);
		if(_stricmp(tag,"/frame")==0)
			endTag=true;
		else if(_stricmp(tag,"armupdate")==0)
		{
			parm=new jvs3d_armupdate;

			parm->loadXMLFile(fp);

			if(m_armUpdates) m_armUpdates->appendEnd(parm);
			else m_armUpdates=parm;
		}
		delete tag;
	}
	return true;
}

char jvs3d_armupdate::loadXMLFile(FILE *fp)
{
	char *tag=null;
	bool endTag=false;
	char *parent=null;;
	while(!(feof(fp) || endTag))
	{
		tag = jvs_textparser::getNextTag(fp);
		if(_stricmp(tag,"/armupdate")==0)
			endTag=true;
		else if(_stricmp(tag,"armature")==0)
		{
			parent=jvs_textparser::getCDATATag(fp);
			setName(parent);
			delete parent;
		}			
		else if(_stricmp(tag,"rotation")==0)
			fscanf(fp,"%lf,%lf,%lf",&m_rotation[0],&m_rotation[1],&m_rotation[2]);
		delete tag;
	}
	return true;
}

jvs3d_item::jvs3d_item()
{
	m_parentItem=null;
	m_objects=null;
	m_materials=null;
	m_cycles=null;
	m_armatures=null;
};

jvs3d_item::~jvs3d_item()
{
	jvs3d_cycle * pcyc=null;
	jvs3d_material * pmat=null;
	jvs3d_object * pobj=null;
	jvs3d_armature * parm=null;
	void *ptr=null;

	if(m_cycles)
	{
		pcyc=(jvs3d_cycle*)m_cycles->getTop();
		while(pcyc)
		{
			if(pcyc->getNext())
			{
				pcyc=(jvs3d_cycle*)pcyc->getNext();
				delete (jvs3d_cycle*)pcyc->getPrev();
			} else { delete (jvs3d_cycle*)pcyc; pcyc=null; }
		}
		m_cycles=null;
	}
	if(m_objects)
	{
		pobj=(jvs3d_object*)m_objects->getTop();
		while(pobj)
		{
			if(pobj->getNext())
			{
				pobj=(jvs3d_object*)pobj->getNext();
				delete (jvs3d_object*)pobj->getPrev();
			} else { delete pobj; pobj=null; }
		}
		m_objects=null;
	}
	if(m_materials)
	{
		pmat=(jvs3d_material*)m_materials->getTop();
		while(pmat)
		{
			if(pmat->getNext())
			{
				pmat=(jvs3d_material*)pmat->getNext();
				delete (jvs3d_material*)pmat->getPrev();
			} else { delete pmat; pmat=null; }
		}
		m_materials=null;
	}
	if(m_armatures)
	{
		parm=(jvs3d_armature*)m_armatures->getTop();
		while(parm)
		{
			ptr=(void*)parm;
			parm=(jvs3d_armature*)parm->getNext();
			delete (jvs3d_armature*)ptr;
		}
		m_armatures=null;
	}
}
