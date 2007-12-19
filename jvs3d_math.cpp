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
 * class jvs3d_math - start
 ***************************************************************/

double jvs3d_math::RadiansToDegrees(double rad)
{
	double ret=rad*(double)(180.0f/JVS_PI);
	if(DBGMATH) 
		printf("RadiansToDegrees : %f=%f(%f/%f)\n",
			ret,rad,180.0f,JVS_PI);
	return ret;
}
double jvs3d_math::DegreesToRadians(double deg)
{
	double ret = deg*(double)(JVS_PI/180.0f);
	if(DBGMATH) 
		printf("DegreesToRadians : %f=%f(%f/%f)\n",
			ret,deg,JVS_PI,180.0f);
	return ret;
}
double jvs3d_math::getDistance(double x1, double y1, double x2, double y2)
{
	if(DBGMATH) printf("%f, %f, %f, %f\n",x1,y1,x2,y2);
	return 
		sqrt( (double)
				(
					(x2-x1)*(x2-x1) +
					(y2-y1)*(y2-y1)
				)
			);
}

/***************************************************************
 * class jvs3d_vertice - start
 ***************************************************************/

jvs3d_vertice::jvs3d_vertice()
{
	m_armature=null;
	m_object=null;
	for(int i=0;i<4;i++)
		m_data[i]=m_temp[i]=0;
}

jvs3d_vertice::~jvs3d_vertice()
{
	if(DBGDEL) printf("killing vertice\n");
}

/*void jvs3d_vertice::operator=(jvs3d_vertice *a)
{
#ifndef _MSC_VER
	cout << "BLAH!" << endl;
#endif
	set(a->m_data[0],a->m_data[1],a->m_data[2]);
};*/

void jvs3d_vertice::calculateNormal(jvs3d_vertice one, jvs3d_vertice two, jvs3d_vertice three)
{
	jvs3d_vertice normal;
	jvs3d_vertice *toret=null;
	normal.crossProduct(one,two,three);
	normal.normalize();
	this->set(&normal);
}
void jvs3d_vertice::normalize(void)
{
	double len=length();
	for(int i=0; i<3; i++)
		m_data[i]=(m_data[i]/len);
}
void jvs3d_vertice::crossProduct(jvs3d_vertice one, jvs3d_vertice two, jvs3d_vertice three)
{
	jvs3d_vertice v1, v2;
	v1.set(&one);
	v1-=two;
	v2.set(&two);
	v2-=three;
	this->set( v1.m_data[1]*v2.m_data[2] - v1.m_data[2]*v2.m_data[1],
		v1.m_data[2]*v2.m_data[0] - v1.m_data[0]*v2.m_data[2],
		v1.m_data[0]*v2.m_data[1] - v1.m_data[1]*v2.m_data[0] );
}

/*`void jvs3d_vertice::normalize(void)
{
	double l=this.length();
	for(int i=0;i<3;i++)
		m_data[i]/=length;
}*/
bool jvs3d_vertice::setTemp(float x, float y, float z)
{
	if(DBGVERT) 
		printf("setting vertice: %d,%d,%d\n",x,y,z);
	m_temp[0]=x;
	m_temp[1]=y;
	m_temp[2]=z;
	m_temp[3]=1;
	return true;
}
bool jvs3d_vertice::setTemp(jvs3d_vertice *v)
{
	if(DBGVERT) 
		printf("setting vertice: %d, %d, %d\n", v->m_temp[0],v->m_temp[1],v->m_temp[2]);
	if(!v) return false;
	m_temp[0]=v->m_temp[0];
	m_temp[1]=v->m_temp[1];
	m_temp[2]=v->m_temp[2];
	m_temp[3]=v->m_temp[3];
	return true;
}
bool jvs3d_vertice::set(float x, float y, float z)
{
	if(DBGVERT) 
		printf("setting vertice: %d,%d,%d\n",x,y,z);
	m_data[0]=x;
	m_data[1]=y;
	m_data[2]=z;
	m_data[3]=1;
	return true;
}
bool jvs3d_vertice::set(jvs3d_vertice *v)
{
	if(DBGVERT) 
		printf("setting vertice: %d, %d, %d\n", v->m_data[0],v->m_data[1],v->m_data[2]);
	if(!v) return false;
	m_data[0]=v->m_data[0];
	m_data[1]=v->m_data[1];
	m_data[2]=v->m_data[2];
	m_data[3]=v->m_data[3];
	return true;
}
double jvs3d_vertice::length(void)
{
	double acc=0; // accumulator
	for(int i=0; i<3; i++)
		acc+=m_data[i]*m_data[i];
	return sqrt(acc);
}
void jvs3d_vertice::swap(jvs3d_vertice &one, jvs3d_vertice &two)
{
	jvs3d_vertice tmp;
	void *ptr;
	ptr=one.m_ptr;
	one.m_ptr=two.m_ptr;
	two.m_ptr=ptr;
	tmp.set(&one);
	tmp.setTemp(&one);
	one.set(&two);
	one.setTemp(&two);
	two.set(&tmp);
	two.setTemp(&tmp);
}
double jvs3d_vertice::dotcos(jvs3d_vertice a)
{
	jvs3d_vertice pa;
	pa.set(this);
	pa.normalize();
	jvs3d_vertice pb;
	pb.set(&a);
	pb.normalize();

	double ret=0;
	if(DBGVERT_MATH) printf("dotcos: ");
	for(int i=0;i<3;i++)
	{
		ret+=pa.m_data[i]*pb.m_data[i];
		if(DBGVERT_MATH) printf("%d->",ret);
	}
	if(DBGVERT_MATH) printf("\n");
	return ret;
}
void jvs3d_vertice::subtract(jvs3d_vertice a)
{
	for(int i=0; i<3; i++)
		m_data[i]-=a.m_data[i];
}
void jvs3d_vertice::add(jvs3d_vertice a)
{
	for(int i=0; i<3; i++)
		m_data[i]+=a.m_data[i];
}
void jvs3d_vertice::scale(double s)
{
	for(int i=0; i<3; i++)
		m_data[i]*=s;
}
void jvs3d_vertice::operator*=(jvs3d_matrix m)
{
	int r=0,c=0;
	double nv[4];
	memset(nv,0,sizeof(nv));
	
//NewX = X * Matrix[0][0] + Y * Matrix[1][0] + Z * Matrix[2][0] + Matrix[3][0];
//NewY = X * Matrix[0][1] + Y * Matrix[1][1] + Z * Matrix[2][1] + Matrix[3][1];
//NewZ = X * Matrix[0][2] + Y * Matrix[1][2] + Z * Matrix[2][2] + Matrix[3][2];
	m_data[3]=1;
	for(c=0;c<4;c++)
	{
		nv[c]=0;
		for(r=0;r<4;r++)
			nv[c]+=m_data[r]*m.m_data[r][c];
	}
	memcpy(m_data,nv,sizeof(nv));
}

void jvs3d_vertice::drawGL(jvs3d_object * pobj)
{	
#ifdef _MSC_VER
	jvs3d_armature *parm=null;
	jvs3d_armature *parmTop=null;
	jvs3d_vertice accum;
	if(!m_armature)
		glVertex3dv(m_data);	//verified
	else
	{
		parm=m_armature;
		parmTop=m_armature;
		parmTop->m_pivotXForm.set(&parmTop->m_pivot);
		memcpy(accum.m_data,m_data,sizeof(m_data));

		accum+=pobj->m_translate;
		parmTop->m_pivotXForm+=pobj->m_translate;
		while(parm)
		{
			accum-=parm->m_pivot;
			parmTop->m_pivotXForm-=parm->m_pivot;

			accum*=(jvs3d_matrix)parm->m_matrix;
			parmTop->m_pivotXForm*=(jvs3d_matrix)parm->m_matrix;

			accum+=parm->m_pivot;
			parmTop->m_pivotXForm+=parm->m_pivot;

			parm=parm->m_parentArmature;
		}
		parmTop->m_pivotXForm-=pobj->m_translate;
		accum-=pobj->m_translate;

		memcpy(m_temp,accum.m_data,sizeof(m_temp));
		glVertex3dv(m_temp);	//verified
	}
#endif
	if(DBGOBJDISP || DBGVERT_MATH) 
		printf("%d,%d,%d\n",m_data[0],m_data[1],m_data[2]);
}

bool jvs3d_vertice::storeXMLFileList(FILE *fp)
{
	jvs3d_vertice * pvert=(jvs3d_vertice*)getTop();
	fprintf(fp,"<vertice_list>");
	while(pvert)
	{
		pvert->storeXMLFile(fp);
		pvert=(jvs3d_vertice*)pvert->getNext();
	}
	fprintf(fp,"</vertice_list>");
	return true;
}

bool jvs3d_vertice::storeXMLFile(FILE *fp)
{
	if(m_armature)
		fprintf(fp,"<![CDATA[%s]]>,%lf,%lf,%lf;",m_armature->getName(),m_data[0],m_data[1],m_data[2]);
	else fprintf(fp,"na,%lf,%lf,%lf;",m_data[0],m_data[1],m_data[2]);
	return true;
}

int jvs3d_vertice::leastPoint(jvs3d_vertice verts[],int numVertices,int axis)
{
	int i,ret=0;
	ret=0;
	for(i=0;i<numVertices;i++)
	{
		if( verts[i].m_data[axis] < verts[ret].m_data[axis] )
			ret=i;
	}
	return ret;
}
int jvs3d_vertice::greatestPoint(jvs3d_vertice verts[],int numVertices,int axis)
{
	int i,ret=0;
	ret=0;
	for(i=0;i<numVertices;i++)
	{
		if( verts[i].m_data[axis] > verts[ret].m_data[axis] )
			ret=i;
	}
	return ret;
}

/***************************************************************
 * class jvs3d_matrix - start
 ***************************************************************/

jvs3d_matrix::jvs3d_matrix()
{memset(m_data,0,sizeof(m_data));}
jvs3d_matrix::~jvs3d_matrix()
{
}
void jvs3d_matrix::multiply(jvs3d_matrix a)
{
	int r=0,c=0,i=0;
	double nm[4][4];
	memset(nm,0,sizeof(nm));
	for(i=0;i<4;i++)
	{
		//  NewMatrix[i][0] = MatrixA[i][0] * MatrixB[0][0] +	MatrixA[i][1] * MatrixB[1][0] + MatrixA[i][2] * MatrixB[2][0];
		//  NewMatrix[i][1] = MatrixA[i][0] * MatrixB[0][1] +	MatrixA[i][1] * MatrixB[1][1] + MatrixA[i][2] * MatrixB[2][1];
		//  NewMatrix[i][2] = MatrixA[i][0] * MatrixB[0][2] +	MatrixA[i][1] * MatrixB[1][2] + MatrixA[i][2] * MatrixB[2][2];
		for(r=0;r<4;r++)
		{
			nm[i][r]=0;
			for(c=0;c<4;c++)
			{
				nm[i][r]+=m_data[i][c]*a.m_data[c][r];
			}
		}
	}
	memcpy(m_data,nm,sizeof(nm));
}
void jvs3d_matrix::operator*=(jvs3d_matrix a)
{this->multiply(a);}
void jvs3d_matrix::operator*=(double s)
{
	int r=0, c=0;
	for(r=0;r<4;r++)
		for(c=0;c<4;c++)
			m_data[r][c]*=s;
}
bool jvs3d_matrix::initialize(unsigned int type, void *inp)
{
	int c=0;
	int r=0;
	jvs3d_matrix temp;
	jvs3d_vertice *data=null;
	double ddata[3];
	double *scaler=null;
	switch(type)
	{
	case JVS3D_MATRIX_IDENT:
		memset(m_data,0,sizeof(m_data));
		for(r=0;r<4;r++)
			m_data[r][r]=1;
		break;
	case JVS3D_MATRIX_ROT:
		memcpy(ddata,inp,sizeof(ddata));
		if(!ddata) return false;
		this->initialize(JVS3D_MATRIX_IDENT,null);
		if( ddata[0])
		{
			temp.initialize(JVS3D_MATRIX_IDENT,null);
			temp.m_data[1][1]=cos( ((JVS_PI/180)*ddata[0]) );
			temp.m_data[1][2]=-sin( ((JVS_PI/180)*ddata[0]) );
			temp.m_data[2][1]=sin( ((JVS_PI/180)*ddata[0]) );
			temp.m_data[2][2]=cos( ((JVS_PI/180)*ddata[0]) );
			this->multiply(temp);
		}
		if( ddata[1])
		{
			temp.initialize(JVS3D_MATRIX_IDENT,null);
			temp.m_data[0][0]=cos( ((JVS_PI/180)*ddata[1]) );
			temp.m_data[0][2]=-sin( ((JVS_PI/180)*ddata[1]) );
			//temp.m_data[2][1]=sin( ((JVS_PI/180)*ddata[1]) );
			temp.m_data[2][0]=sin( ((JVS_PI/180)*ddata[1]) );
			temp.m_data[2][2]=cos( ((JVS_PI/180)*ddata[1]) );
			this->multiply(temp);
		}
		if( ddata[2])
		{
			temp.initialize(JVS3D_MATRIX_IDENT,null);
			temp.m_data[0][0]=cos( ((JVS_PI/180)*ddata[2]) );
			temp.m_data[0][1]=-sin( ((JVS_PI/180)*ddata[2]) );
			temp.m_data[1][0]=sin( ((JVS_PI/180)*ddata[2]) );
			temp.m_data[1][1]=cos( ((JVS_PI/180)*ddata[2]) );
			this->multiply(temp);
		}
		break;
	case JVS3D_MATRIX_TRANS:
		data=(jvs3d_vertice*)inp;
		this->initialize(JVS3D_MATRIX_IDENT,null);
		for(r=0;r<4;r++)
			m_data[r][3]=data->m_data[r];
		break;
	case JVS3D_MATRIX_SCALE:
		scaler=(double*)inp;
		this->initialize(JVS3D_MATRIX_IDENT,null);
		for(r=0;r<4;r++)
			m_data[r][r]=*scaler;
		break;
	}
	return true;
}

bool jvs3d_math::orderArray2dx4(int upORdown, int idx, int d1, int arr[][4])
{
	int d2=4;
	int *temp = new int[d2];
	if(!temp) 
		return false;

	if( upORdown==0) 
	{
		delete temp;
		return false;
	}

	for(int i=0; i<d1; i++)
	{
		arr[i][1]=i;
		for(int e=i; e<d1; e++)
		{
			if( (upORdown<0 && arr[e][idx]<arr[i][idx]) ||  
				(upORdown>0 && arr[e][idx]>arr[i][idx]) )
			{
				for(int a=0;a<d2;a++)
				{
					temp[a]=arr[e][a];
					arr[e][a]=arr[i][a];
					arr[i][a]=temp[a];
				}
				
			}
		}
	}

	delete temp;
	return true;
}