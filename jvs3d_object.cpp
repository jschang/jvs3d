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

///////////////////////////////////////////
// All drawgl functions - except vertice //
///////////////////////////////////////////

bool jvs3d_polygon::drawGL(jvs3d_object *pobj, jvs3d_vertice *vlist)
{
	jvs3d_vertice *pv=(jvs3d_vertice*)vlist->getTop();
	const unsigned char * ret=null;

	#ifdef _MSC_VER 
		if( ! 
				(  
					(
						m_materialPtr && 
						m_materialPtr->m_glTextureObject 
					) 
				) || 
				(m_drawMode==GL_LINE_LOOP) 
			)
		{
			glPolygonMode(m_polyFace,m_polyMode); 
			glColor3f(m_colorR,m_colorG,m_colorB);
			glBegin(m_drawMode);
		} else if( m_materialPtr && m_drawMode!=GL_LINE_LOOP )
		{
			m_materialPtr->bindGL();
			glPolygonMode(GL_FRONT,GL_FILL);
			glBegin(GL_POLYGON);			
		}		
	#else
		if(DBGOBJDISP)
			cout << "Polygon: " << m_num << " vertices" << endl;
	#endif	
	for(unsigned int i=0; i<m_num; i++)
	{ 
		if(DBGVERT_MATH && m_txtrX && m_txtrY)
			printf( "texture coord %f, %f\n",m_txtrX[i],m_txtrY[i]);

		pv=(jvs3d_vertice*)vlist->getAt(m_index[i]);
		if(pv) 
		{
			#ifdef _MSC_VER
				if(m_materialPtr && 
					m_txtrX && m_txtrY &&
					m_materialPtr->m_materialType==JVS3D_MAT_TEXTURE)
				{
						glTexCoord2d(
							(double)m_txtrX[i],
							(double)m_txtrY[i]);	//verified
				}
			#endif
			pv->drawGL(pobj);
		}
	}

	#ifdef _MSC_VER
		glEnd();
		glDisable(GL_TEXTURE_2D);
	#endif

	return true;
}

bool jvs3d_object::drawGL(void)
{
#ifdef _MSC_VER
	return drawGL(m_polyMode,m_polyFace);
#else
	return true;
#endif
}
 bool jvs3d_object::drawGL(unsigned long mode, unsigned long face)
{
	jvs3d_polygon *ppoly=null;
	if(DBGOBJDISP) printf("Attempting to display object\n");
	if(mp_polygons)
	{
		if(DBGOBJDISP)
			printf("%u polygons to process\n",mp_polygons->countList());

		ppoly=(jvs3d_polygon*)mp_polygons->getTop();

		// cycle through each polygon, drawing each
		while(ppoly)
		{
			ppoly->drawMode(m_drawMode);
			ppoly->polygonMode(mode,face);
			ppoly->drawGL(this,mp_vertices);
			ppoly=(jvs3d_polygon*)ppoly->getNext();
		}
	}

	return true;
}

/***************************************************************
 * class jvs3d_object - start
 ***************************************************************/

jvs3d_object::jvs3d_object()
{
	mp_vertices=null;
	mp_polygons=null;
	m_drawMode=0;
	m_polyMode=GL_FRONT;
	m_polyFace=GL_FILL;
	m_translate.set(0,0,0);
	m_rotate.set(0,0,0);
}
jvs3d_object::~jvs3d_object()
{
	void * last=null;
	if(DBGDEL) printf("killing object\n");
	if(mp_vertices)
	{
		last=mp_vertices->getTop();
		mp_vertices=(jvs3d_vertice*)last;
		while( mp_vertices=(jvs3d_vertice*)mp_vertices->getNext() )
		{
			delete (jvs3d_vertice*)last;
			last = (void *)mp_vertices;
		}
	}
	if(mp_polygons)
	{
		last=mp_polygons->getTop();
		mp_polygons=(jvs3d_polygon*)last;
		while( mp_polygons=(jvs3d_polygon*)mp_polygons->getNext() )
		{
			if(last) delete (jvs3d_polygon*)last;
			last = (void *)mp_polygons;
		}
	}
}

bool jvs3d_object::addVertice(float x, float y, float z)
{
	jvs3d_vertice *ptr = new jvs3d_vertice;
	if(ptr)
	{
		ptr->set(x,y,z);
		ptr->m_object=this;
		if(DBGOBJLOAD) printf("vertice set\n");
		if(mp_vertices==null)
		{
			mp_vertices=ptr;
			return true;
		} else if( mp_vertices->appendEnd(ptr)==null )
		{ printf( "error: a vertice could not be tacked onto vertice list\n"); exit(1); }
	} else { printf("error: allocation for an object vertice failed\n"); exit(1); }
	return true;
}

bool jvs3d_object::addPolygon(float txo, float tyo, int length, uint *idx_list)
{
	jvs3d_polygon *ptr=new jvs3d_polygon;
	ptr->m_num=length;
	ptr->m_index = new uint[length];

	if(DBGPOLY) 
		printf("adding polygon definition\n" );
	
	for(int i=0;i<length;i++)
	{
		if(DBGPOLY) printf(" idx: $u", idx_list[i]);
		ptr->m_index[i]=idx_list[i];
	}
	ptr->prepTextureCoords(*mp_vertices);
	if(!mp_polygons)
		mp_polygons=ptr;
	else mp_polygons->appendEnd(ptr);
	return true;
}

unsigned int jvs3d_object::verticeCount(void)
{
	jvs_baseclass *ptr = (jvs_baseclass *) this->mp_vertices;
	if(ptr) return ptr->countList();
	else return 0;
}
bool jvs3d_object::applyFrame(jvs3d_frame *frame, jvs3d_armature *exclude)
{
	jvs3d_armupdate *pau=null; // current arm update
	pau=frame->m_armUpdates;
	while(pau)
	{
		//if(pau->m_

		pau=(jvs3d_armupdate*)pau->getNext();
	} // while(pau)
	return true;
}

///////////////////
// file handling //
///////////////////

bool jvs3d_object::loadXFile(const char *pathfile)
// load x formatted file
{
	int verts=0, faces=0;
	//unsigned long ret;
	jvs3d_file_x parser;

	if(!parser.openFile(pathfile)) return false;
	while(parser.readNext())
		if(!strcmp(parser.s_jvs_cur,"Mesh"))
		{printf("FIRST FOUND!");break;}

	parser.readNext();
	parser.readNext();

	return true;
}

bool jvs3d_object::storeXFile(const char *pathfile)
// store x formatted file
{
	return true;
}

bool jvs3d_object::loadAC3DFile(const char *pathfile)
{
	jvs3d_file_ac3d fil;
	if(fil.loadFile(pathfile,this)) return true;
	else return false;
}
bool jvs3d_object::storeAC3DFile(const char *pathfile)
{return false;}


/////////////////////////
// Internal Adjustment //
/////////////////////////

bool jvs3d_object::newCenter(void)
{
	if(!mp_vertices) return true;

	double mx=0, my=0, mz=0;
	unsigned int n;
	
	n = mp_vertices->countList();

	jvs3d_vertice *v=(jvs3d_vertice*)mp_vertices->getTop();
	while(v)
	{
		mx+=v->m_data[0];
		my+=v->m_data[1];
		mz+=v->m_data[2];
		v=(jvs3d_vertice*)v->getNext();
	}
	mx = mx / n;
	my = my / n;
	mz = mz / n;

	v=(jvs3d_vertice*)mp_vertices->getTop();
	while(v)
	{
		v->m_data[0]-=mx;
		v->m_data[1]-=my;
		v->m_data[2]-=mz;
		v=(jvs3d_vertice*)v->getNext();
	}
	return true;
}


//////////////////////
// Display settings //
//////////////////////

void jvs3d_object::polygonMode(unsigned long face, unsigned long mode)
{m_polyMode=mode; m_polyFace=face;}

void jvs3d_object::drawMode(unsigned long mode)
{m_drawMode=mode;}

/***************************************************************
 * class jvs3d_polygon - start
 ***************************************************************/

jvs3d_polygon::jvs3d_polygon()
{
#ifdef _MSC_VER
	m_polyFace=GL_FRONT;
	m_polyMode=GL_FILL;
	m_drawMode=GL_POLYGON;
#else
	m_polyFace=0;
	m_polyMode=0;
	m_drawMode=0;
#endif
	m_colorR=0.5f;
	m_colorG=0.5f;
	m_colorB=0.5f;
	m_materialPtr=0;
	m_txtrX=0;
	m_txtrY=0;
	m_index=0;
	m_num=0;
}
jvs3d_polygon::~jvs3d_polygon()
{
	if(DBGDEL) printf("killing polygon\n");
	if(m_index) 
	{
		if(DBGDEL) printf("killing index array\n");
		delete m_index;
	}
	if(m_txtrX) 
	{
		if(DBGDEL) printf("killing x coord array\n");
		delete m_txtrX;
	}
	if(m_txtrY) 
	{
		if(DBGDEL) printf("killing y coord array\n" );
		delete m_txtrY;
	}
	if(DBGDEL) printf("polygon killed.\n" );
}

void jvs3d_polygon::polygonMode(unsigned long face, unsigned long mode)
{m_polyMode=mode; m_polyFace=face;}

void jvs3d_polygon::drawMode(unsigned long mode)
{m_drawMode=mode;}

double * jvs3d_polygon::getLenAng(jvs3d_vertice &verts)
{
	//return false;
	// not even a triangle!? return that bitch; we ain doin shit!  fer rizza
	if(m_num<3) return false;
	if(verts.countList()<3) return false;
	// determine the angle at each point of the polygon;
	// line texture up with idx edge on polygon
	double *toret = new double[m_num*2];
	jvs3d_vertice a,b,c;
	unsigned int j=0;
	if(toret) 
	{
		toret[0]=0;toret[m_num]=0;
		a.set((jvs3d_vertice *)verts[m_index[0]]);
		b.set((jvs3d_vertice *)verts[m_index[1]]);
		b-=a;
		toret[1]=0; // cosine
		toret[m_num+1]=b.length();
		for(int i=2; i < m_num; i++)
		{
			a.set((jvs3d_vertice *)verts[m_index[1]]);
			b.set((jvs3d_vertice *)verts[m_index[0]]);
			c.set((jvs3d_vertice *)verts[m_index[i]]);
			a-=b;
			c-=b;
			toret[i]=fabs(a.dotcos(c));
			toret[i+m_num]=c.length();
			if(DBGVERT_MATH)
				printf("this index %u has cosine %f and length %f from v[0] and v[1] \n" ,i ,toret[i] ,toret[i+m_num]);

		}

		return toret;
	} else return false;
}

void jvs3d_polygon::textureCenter(double *x, double *y)
{
	int i=0;
	*x=0;
	*y=0;
	if(m_materialPtr && m_num>0)
	{
		for(i=0;i<m_num;i++)
		{
			*x+=m_txtrX[i];
			*y+=m_txtrY[i];
		}
		*x/=(double)m_num;*y/=(double)m_num;
		for(i=0;i<m_num;i++)
		{
			m_txtrX[i]-=*x;
			m_txtrY[i]-=*y;
		}
	}
}

void jvs3d_polygon::textureTranslate(double x, double y)
{
	unsigned int i=0;
	if(m_materialPtr && m_num>0)
	{
		for(i=0;i < m_num;i++)
		{
			m_txtrX[i]+=x;
			m_txtrY[i]+=y;
		}
	}
}

void jvs3d_polygon::textureRotate(double theta)
{
	double x=0, y=0;
	double *oldY=new double;
	double *oldX=new double;
	double rad=(JVS_PI/180.00)*theta;
	int i=0;

	if(m_materialPtr && m_num>0)
	{
		textureCenter(oldX,oldY);

		for(i=0;i<m_num;i++)
		{
			m_txtrX[i]=m_txtrX[i]*cos(rad)-m_txtrY[i]*sin(rad);
			m_txtrY[i]=m_txtrY[i]*cos(rad)+m_txtrX[i]*sin(rad);
		}

		textureTranslate(*oldX,*oldY);
	}
}

void jvs3d_polygon::textureScale(double x, double y)
{
	double *oldY=new double;
	double *oldX=new double;
	int i=0;
	if(m_materialPtr && m_num>0)
	{
		textureCenter(oldX,oldY);
		for(i=0;i<m_num;i++)
		{
			m_txtrX[i]*=x;
			m_txtrY[i]*=y;
		}
		textureTranslate(*oldX,*oldY);
	}
}

bool jvs3d_polygon::prepTextureCoords(jvs3d_vertice &verts)
{
	double *lenangs=getLenAng(verts);
	double s;
	if(!lenangs) return false;
	jvs3d_vertice *thisvert;
	double width, height;
	if(m_txtrX) {delete m_txtrX;m_txtrX=0;}
	if(m_txtrY) {delete m_txtrY;m_txtrY=0;}
	if(!(m_txtrX && m_txtrY))
	{
		m_txtrX=new double[m_num];
		m_txtrY=new double[m_num];
	}
	if(m_txtrY && m_txtrX)
	{
		if(DBGVERT_MATH) printf("successful allocate: %d, %d\n", m_txtrX, m_txtrY);
		m_txtrX[0]=0;
		m_txtrY[0]=0;
		m_txtrX[1]=lenangs[1+m_num];
		m_txtrY[1]=0;
		if(DBGVERT_MATH)
		{
			printf("%d,%d\n",m_txtrX[0],m_txtrY[0]);
			printf("%d,%d\n",m_txtrX[1],m_txtrY[1]);
		}
		for(int idx=2; idx<m_num; idx++)
		{

			m_txtrX[idx]=lenangs[idx+m_num]*lenangs[idx];
			m_txtrY[idx]=lenangs[idx+m_num]*sin(acos(lenangs[idx]));

			if(DBGVERT_MATH)
				printf("%d, %d\n",m_txtrX[idx],m_txtrY[idx]);
		}

	} else return false;
}

/***************************************************************
 * class jvs3d_armature - start
 ***************************************************************/

jvs3d_armature::jvs3d_armature()
{
	m_armatures=0;
//	m_vertices=0;
	m_parentArmature=0;
	m_pivot.set(0,0,0);
	m_matrix.initialize(JVS3D_MATRIX_IDENT,null);
	m_parentName=null;
};
jvs3d_armature::~jvs3d_armature()
{
//	if(m_vertices)
//		delete m_vertices;
	if(m_armatures) 
		delete m_armatures;
};

///////////////
/*
bool jvs3d_armature::addVertice(jvs3d_vertice *v)
{
	jvs_baseclass *ptr=null;

	if(hasVertice(v))
		return false;

	if(m_vertices)
	{
		ptr = new jvs_baseclass;
		ptr->m_ptr=v;
		m_vertices->appendEnd(ptr);
	} else 
	{
		m_vertices = new jvs_baseclass;
		m_vertices->m_ptr=v;
	}
	return true;
}
jvs_baseclass * jvs3d_armature::hasVertice(jvs3d_vertice *v)
{
	jvs_baseclass *ptr=null;
	if(m_vertices)
	{
		ptr=(jvs_baseclass*)m_vertices->getTop();
		while(ptr)
		{
			if(v==ptr->m_ptr)
				return ptr;
			ptr=(jvs_baseclass*)ptr->getNext();
		}
		return false;
	} else return false;
}
bool jvs3d_armature::remVertice(jvs3d_vertice *v)
{
	jvs_baseclass *ptr=null;
	if(ptr=hasVertice(v))
	{
		if(m_vertices->countList()>=1)
		{
			if(m_vertices==ptr)
			{
				if( m_vertices->getNext() )
					m_vertices=(jvs_baseclass*)m_vertices->getNext();
				else if( m_vertices->getPrev() )
					m_vertices=(jvs_baseclass*)m_vertices->getPrev();
			}
			m_vertices->removeMember(ptr);
		} else m_vertices=null;
		delete ptr;
		return true;
	} else return false;
}*/

///////////////
bool jvs3d_armature::addArmature(jvs3d_armature *v)
{
	jvs_baseclass *ptr=null;

	if(hasArmature(v))
		return false;

	if(m_armatures)
	{
		ptr = new jvs_baseclass;
		ptr->m_ptr=v;
		m_armatures->appendEnd(ptr);
	} else 
	{
		m_armatures = new jvs_baseclass;
		m_armatures->m_ptr=v;
	}
	v->m_parentArmature=(jvs3d_armature*)this;
	return true;
}
jvs_baseclass * jvs3d_armature::hasArmature(jvs3d_armature *v)
{
	jvs_baseclass *ptr=null;
	if(m_armatures)
	{
		ptr=(jvs_baseclass*)m_armatures->getTop();
		while(ptr)
		{
			if(v==ptr->m_ptr)
				return ptr;
			ptr=(jvs_baseclass*)ptr->getNext();
		}
		return false;
	} else return false;
}
bool jvs3d_armature::remArmature(jvs3d_armature *v)
{
	jvs_baseclass *ptr=null;
	if(ptr=hasArmature(v))
	{
		v->m_parentArmature=null;
		if(m_armatures->countList()>=1)
		{
			if(m_armatures==ptr)
			{
				if( m_armatures->getNext() )
					m_armatures=(jvs_baseclass*)m_armatures->getNext();
				else if( m_armatures->getPrev() )
					m_armatures=(jvs_baseclass*)m_armatures->getPrev();
			}
			m_armatures->removeMember(ptr);
		} else m_armatures=null;
		delete ptr;
		return true;
	} else return false;
}
bool jvs3d_armature::setupMatrices(jvs3d_armupdate *udates,jvs3d_armature *parent)
// pass in null, parent is necessary for recursive aspect of function
{
	jvs3d_armature *parm = (jvs3d_armature*)this->getTop();
	jvs3d_armupdate *pudate=null;
	jvs3d_matrix rm;
	int i=0;
	while(parm)
	{
		pudate=udates->findArmature(parm);

		if(pudate)
			parm->m_matrix.initialize(JVS3D_MATRIX_ROT,pudate->m_rotation);
		parm=(jvs3d_armature*)parm->getNext();
	}
	return true;
}

bool jvs3d_armature::clearMatrices(void)
{
	jvs3d_armature *parm = (jvs3d_armature*)this->getTop();
	while(parm)
	{
		parm->m_matrix.initialize(JVS3D_MATRIX_IDENT,null);
		parm=(jvs3d_armature*)parm->getNext();
	}
	return true;
}

/***************************************************************
 * class jvs3d_frame - start
 ***************************************************************/

jvs3d_armupdate::jvs3d_armupdate()
{
	m_armature=null;
	m_rotation[0]=0;
	m_rotation[1]=0;
	m_rotation[2]=0;
}
jvs3d_armupdate* jvs3d_armupdate::findArmature(jvs3d_armature *arm)
{
	jvs3d_armupdate *pudate;
	pudate=(jvs3d_armupdate*)this->getTop();
	while(pudate)
	{
		if(pudate->m_armature==arm)
			return pudate;
		pudate=(jvs3d_armupdate*)pudate->getNext();
	}
	return null;
}
jvs3d_frame::jvs3d_frame()
{
	m_armUpdates=null;
	m_parentCycle=null;
}
jvs3d_frame::~jvs3d_frame()
{
	jvs3d_armupdate *parmupdate=m_armUpdates;
	jvs3d_armupdate *tokill=null;
	while(parmupdate)
	{
		tokill=parmupdate;
		parmupdate=(jvs3d_armupdate*)parmupdate->getNext();
		if(tokill) delete tokill;
	}
}
jvs3d_cycle::jvs3d_cycle() {m_frames=null;m_curFrame=null;}
jvs3d_cycle::~jvs3d_cycle()
{	
	jvs3d_frame *pframe=m_frames;
	jvs3d_frame *tokill=null;
	while(pframe)
	{
		tokill=pframe;
		pframe=(jvs3d_frame*)pframe->getNext();
		if(tokill) delete tokill;
	}
}


