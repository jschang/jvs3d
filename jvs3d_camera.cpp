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

//////////////////////
// CAMERA FUNCTIONS //
//////////////////////
jvs3d_camera::jvs3d_camera()
{ 
	m_position.set(0,0,0);			// initial camera position origin
	m_rotation.set(0,0,0);			// initialize rotation
	m_lookAt.set(0,0,0);
	m_lastLookAt.set(0,0,0);

	m_FOVDegrees=60;
	m_FOVRadians=jvs3d_math::DegreesToRadians(m_FOVDegrees);
	m_frustumEnd[0]=75.0;
	m_frustumEnd[1]=115.0;
	m_frustumEnd[2]=225.0;
};
jvs3d_camera::~jvs3d_camera(){}

//////////////////////
// CAMERA ACCESSORS //
//////////////////////

double jvs3d_camera::getFOVDegrees(void)
	{return m_FOVDegrees;}
double jvs3d_camera::getFOVRadians(void)
	{return m_FOVRadians;}
bool jvs3d_camera::setFOVDegrees(double d)
{
	m_FOVRadians = jvs3d_math::DegreesToRadians(d); 
	m_FOVDegrees=d; 
	return true;
}
bool jvs3d_camera::setFOVRadians(double r)
{
	m_FOVDegrees = jvs3d_math::RadiansToDegrees(r); 
	m_FOVRadians=r; 
	return true;
}
double jvs3d_camera::getFollowDistance(void)
	{return m_followDistance;}
void jvs3d_camera::setFollowDistance(double d)
	{m_followDistance = d;}
double jvs3d_camera::getFrustumEnd(int idx)
	{return m_frustumEnd[idx];}
bool jvs3d_camera::setFrustumEnd(int idx, double fe)
	{m_frustumEnd[idx] = fe;return true;}
bool jvs3d_camera::getPosition(jvs3d_vertice &pos) 
{ 
	pos.set(&m_position); 
	return true; 
};
bool jvs3d_camera::getRotation(jvs3d_vertice &pos)
{
	pos.set(&m_rotation);
	return true;
}
bool jvs3d_camera::setPosition(jvs3d_vertice pos)
// used to set the position directly,
// primarily the function updateCamera should be used,
// as it _will_ adjust it's position to take into account:
//   objects behind it
//   m_followDistance
{ 
	m_lastPosition.set(&m_position);
	m_position.set(&pos);
	return true; 
};
bool jvs3d_camera::getLookAt(jvs3d_vertice &lookAt) 
{ 
	lookAt.set(&m_lookAt); 
	return true; 
};
bool jvs3d_camera::updateLookAt(jvs3d_vertice &pos)
{
	updateLookAt(pos.m_data[0],pos.m_data[1],pos.m_data[2]);
	return true;
}
bool jvs3d_camera::updateLookAt(double pitch, double yaw, double roll)
{
	double rot[3];
	int i=0;
	jvs3d_vertice pos;
	jvs3d_matrix m;

	rot[0]=pitch;rot[1]=yaw;rot[2]=roll;
	for(i=0;i<3;i++)
	{
		m_rotation.m_data[i]-=rot[i];
		if(m_rotation.m_data[i]>360)
			m_rotation.m_data[i]-=360;
		if(m_rotation.m_data[i]<0)
			m_rotation.m_data[i]+=360;
		rot[i]=m_rotation.m_data[i];
	}
	if( m_rotation.m_data[0]>60 && m_rotation.m_data[0]<180 )
		m_rotation.m_data[0]=60;
	if( m_rotation.m_data[0]>180 && m_rotation.m_data[0]<280 )
		m_rotation.m_data[0]=280;

	pos.set(0,0,-1);
	rotate(-1,pos);
	m_lookAt.set(&pos);

	return true;
}

//////////////////////////////////////
// CAMERA COORDINATE TRANSFORMATION //
//////////////////////////////////////

bool jvs3d_camera::rotate(char dir, jvs3d_vertice &pos)
// rotate coordinate to/from camera coordinates
{
	jvs3d_vertice r;
	bool boolNewMatrixData=false;
	static jvs3d_matrix mx,my,mz;		// current working matrix
	static jvs3d_matrix imx,imy,imz;	// current working matrix
	static jvs3d_vertice lastRot;	// lastRot[ations]

	for(int i=0;i<3;i++)
		if(m_rotation.m_data[i]!=lastRot.m_data[i])
			boolNewMatrixData=true;
	if(boolNewMatrixData)
	{
		// rotate world to camera coords
		r.set(-m_rotation.m_data[0],0,0);
		mx.initialize(JVS3D_MATRIX_ROT,r.m_data);
		r.set(0,-m_rotation.m_data[1],0);
		my.initialize(JVS3D_MATRIX_ROT,r.m_data);
		r.set(0,0,-m_rotation.m_data[2]);
		mz.initialize(JVS3D_MATRIX_ROT,r.m_data);

		// rotate camera to world coord
		r.set(m_rotation.m_data[0],0,0);
		imx.initialize(JVS3D_MATRIX_ROT,r.m_data);
		r.set(0,m_rotation.m_data[1],0);
		imy.initialize(JVS3D_MATRIX_ROT,r.m_data);
		r.set(0,0,m_rotation.m_data[2]);
		imz.initialize(JVS3D_MATRIX_ROT,r.m_data);
	}

	switch(dir)
	{
	case 1:
		pos*=mz;pos*=my;pos*=mx;
		break;
	case -1:
		pos*=imx;pos*=imy;pos*=imz;
		break;
	default:
		return false;
		break;
	}

	lastRot.set(&m_rotation);

	return true;
}
bool jvs3d_camera::translate(char dir, jvs3d_vertice &pos)
// translate coordinate to/from camera coordinates
{
	switch(dir)
	{
	case 1:pos-=m_position;break;
	case -1:pos+=m_position;break;
	default:return false;break;
	}
	return true;
}
bool jvs3d_camera::transform(jvs3d_vertice &pos)
// transform a coordinate to camera coordinates
{
	translate(1,pos);
	rotate(1,pos);
	return true;
}
bool jvs3d_camera::untransform(jvs3d_vertice &pos)
// transform a coordinate from camera coordinates to world coordinates
{
	rotate(-1,pos);
	translate(-1,pos);
	return true;
}

bool jvs3d_camera::forward(double units)
{
	jvs3d_vertice pos, npos;
	double height=m_position.m_data[1];		// preserve height
	npos.set(&m_lookAt);
	pos.set(&m_position);
	npos*=units;	// multiply by what should be a unit vector
	pos+=npos;		// add scaled unit vector to current position
	m_position.set(&pos);
	m_position.m_data[1]=height;			// restore height value
	return true;
}

bool jvs3d_camera::backward(double units)
{
	jvs3d_vertice pos, npos;
	double height=m_position.m_data[1];
	npos.set(&m_lookAt);
	pos.set(&m_position);
	npos*=units;
	pos-=npos;
	m_position.set(&pos);
	m_position.m_data[1]=height;
	return true;
}

bool jvs3d_camera::drawAxis(void)
{	
	jvs3d_vertice apos,bpos,lookAt;
	if(glIsEnabled(GL_TEXTURE_2D))
		glDisable(GL_TEXTURE_2D);
	glLineWidth(5);

	// draw the three axis

	// RED X AXIS
	glColor3d(1,0,0);
	apos.set(-10,0,0);transform(apos);
	bpos.set(10,0,0);transform(bpos);
	glBegin(GL_LINES);
		glVertex3dv(apos.m_data);
		glVertex3dv(bpos.m_data);
	glEnd();
	
	// GREEN Y AXIS
	glColor3d(0,1,0);
	apos.set(0,-10,0);transform(apos);
	bpos.set(0,10,0);transform(bpos);
	glBegin(GL_LINES);
		glVertex3dv(apos.m_data);
		glVertex3dv(bpos.m_data);
	glEnd();

	// BLUE Z AXIS
	glColor3d(0,0,1);
	apos.set(0,0,-10);transform(apos);
	bpos.set(0,0,10);transform(bpos);
	glBegin(GL_LINES);
		glVertex3dv(apos.m_data);
		glVertex3dv(bpos.m_data);
	glEnd();

	// draw the view arrow
	glColor3d(1,1,1);
	apos.set(0,0,0);
	getLookAt(lookAt);
	bpos.set(&lookAt);
	bpos*=20;
	glBegin(GL_LINES);
		glVertex3dv(apos.m_data);
		glVertex3dv(bpos.m_data);
	glEnd();

	return true;
}



//int getWidth();
//int getHeight();
//bool m_boolMapLoaded;