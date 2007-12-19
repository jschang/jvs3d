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

#include "jvslib.h"
#include "jvs3dlib.h"

jvs3d_terrain::~jvs3d_terrain(){delete m_pTextureCoords;}
jvs3d_terrain::jvs3d_terrain()
{
	setInitialValues();
	initializeMultiTextureSupport();
}
jvs3d_terrain::jvs3d_terrain(const char *fileName)
{
	setInitialValues();
	initializeMultiTextureSupport();
	m_boolMapLoaded=loadHeightMap(fileName);
}

void jvs3d_terrain::setInitialValues(void)
{
	m_scale=1.0f;
	m_boolDrawGL=true;
	m_boolMapLoaded=false;
	m_blurRadius=.75f;
	m_translate.set(0,0,0);
	m_pCamera=null;
	m_scaleHeightCorrect=.1f;
	m_pTextureCoords=null;
	m_pMaterials=null;
	m_pTextureMappings=null;
}

bool jvs3d_terrain::loadHeightMap(const char *fileName)
{
	if(!m_boolMapLoaded)
	{
		if(m_heightMap.loadTargaFile(fileName))
		{
			m_boolMapLoaded=true;
			return (calculateTextureCoords() && m_boolMapLoaded);
		}
	}
	return false;
}
bool jvs3d_terrain::loadTextureMapping(const char *fileName, unsigned long *pks)
// pks should be a 4 count array
// fileName should be the fullpath to the file
{
	jvs3d_terrain_texture_mapping *pTT=new jvs3d_terrain_texture_mapping;
	if(!pks) { delete pTT; return false; }
	if(pTT->loadTargaFile(fileName))
	{
		if(m_pTextureMappings)
			m_pTextureMappings->appendEnd(pTT);
		else m_pTextureMappings=pTT;
		pTT->setTextureIds(pks[0],pks[1],pks[2],pks[3]);
		return pTT;
	} 
	delete pTT;
	return false;
}

bool jvs3d_terrain::setMaterials(jvs3d_material *pMaterials)
{
	if(pMaterials) 
	{
		m_pMaterials=pMaterials;
		return true;
	} else return false;
}

bool jvs3d_terrain::calculateTextureCoords(void)
{
	jvs3d_vertice *pThisVert=null;
	double quadT[4][2];		// resultant texture coordinates
	int quadI[4][2];		// the x,y location on the height map
	char quadB[4];			// fetch of quadI into quadT was good?
	double a, b, c;			// some temporary variables
	double scaleX=5;
	double scaleY=5;
	jvs3d_vertice tA, tB, tC;	// some temporary vertices
	jvs3d_vertice quadV[4];

	if(m_boolMapLoaded && !m_pTextureCoords)
	{
		// allocate texture coords
		long size=m_heightMap.m_width*m_heightMap.m_height*3;
		m_pTextureCoords=new double[size];
		if(!m_pTextureCoords)
			return false;

		// zero out all texture coords
		memset(m_pTextureCoords,0,size*sizeof(double));	
		for(int y=1;y<m_heightMap.m_height;y++)
		{
			for(int x=1;x<m_heightMap.m_width;x++)
			{
				a=b=c=0;
				// set the height map coordinates we're looking at 
				quadI[0][0]=x;quadI[0][1]=y;
				quadI[2][0]=x;quadI[2][1]=y-1;
				quadI[1][0]=x-1;quadI[1][1]=y;
				quadI[3][0]=x-1;quadI[3][1]=y-1;
				// 3 2
				// 1 0

				// get each position in world coordinates from the height map
				// also check to see if texture coordinates already exist for the vertex
				for(int i=0;i<4;i++)
				{
					getPosition(quadV[i],quadI[i][0],quadI[i][1]);
					quadB[i] = getTextureCoord(
						quadT[i][0],quadT[i][1],
						quadI[i][0],quadI[i][1]);
				}
				
				// triangle one is composed of indexes 0 to 2
				// triangle two is composed of indexes 3 to 1

				if( quadB[1] && quadB[2] && quadB[3] ) 
				// then it follows that index 3 is calculated as well
				// so all we need to calculate is 2 and 1
				{
					// quad is 
					// 3 2 
					// 1 0
					setTextureCoord(
						(quadB[2]?quadT[2][0]:0),
						(quadB[1]?quadT[1][1]:0),
						quadI[0][0],quadI[0][1]);
				} else if( quadB[1] && quadB[3] ) 
				// then it's just index 1 and 3 that are calculated
				// so we need to calculate 0 and 2
				{
					// quad is 
					// 3 2 
					// 1 0

					// calculate 3rd texture coord (idx 2)
					tA.set(&quadV[2]);
					tA-=quadV[3];
					a=(quadB[3]?quadT[3][0]:0)+(tA.length()/scaleX);
					b=(quadB[3]?quadT[3][1]:0);
					setTextureCoord(a,b,
						quadI[2][0],quadI[2][1]);

					// calculate 1st texture coord (lower right corner, per screen coords)
					setTextureCoord(a,quadT[1][1],
						quadI[0][0],quadI[0][1]);
					
				}
				else // none of the quad has been caculated
				// so we must calculate all four
				// but it's easier, because 1 can be (0,0)
				{
					// quad is 
					// 3 2 
					// 1 0
					setTextureCoord(0,0,quadI[3][0],quadI[3][1]);
					// calculate 3rd texture coord
					tA.set(&quadV[2]);
					tA-=quadV[3];
					a=(quadB[3]?quadT[3][0]:0)+(tA.length()/scaleX);
					setTextureCoord(a,0,
						quadI[2][0],quadI[2][1]);

					// calculate 2nd texture coord
					tB.set(&quadV[1]);
					tB-=quadV[3];
					b=(quadB[3]?quadT[3][1]:0)+(tB.length()/scaleY);
					setTextureCoord(0,b,
						quadI[1][0],quadI[1][1]);

					// calculate 1st texture coord (lower right corner, per screen coords)
					setTextureCoord(a,b,
						quadI[0][0],quadI[0][1]);
				}
			} // for(int x=1;x<m_heightMap.m_width;x++)
		} // for(int y=1;y<m_heightMap.m_height;y++)
		return true;
	} // if(m_boolMapLoaded && !m_pTextureCoords)
	return false;
}

///////////////////////
// TERRAIN ACCESSORS //
///////////////////////

bool jvs3d_terrain::setTranslate(jvs3d_vertice &pos)
{
	setTranslate(pos.m_data[0],pos.m_data[1],pos.m_data[2]);
	return true;
}
bool jvs3d_terrain::setTranslate(double x, double y, double z) 
{
	m_translate.set(x,y,z);
	return true;
}

bool jvs3d_terrain::setCamera(jvs3d_camera *pcam)
{
	m_pCamera = pcam;
	return true;
}
jvs3d_camera * jvs3d_terrain::getCamera(void)
{
	return m_pCamera;
}
bool jvs3d_terrain::setTextureCoord(double tX, double tY, int x, int y)
{
	if(!m_pTextureCoords)
		return false;
	if(x<0 || y<0 || x>m_heightMap.m_width || y>m_heightMap.m_height)
		return false;
	long offset=(x+(m_heightMap.m_width*y))*3;
	double *txtr=&m_pTextureCoords[offset];
	txtr[0]=tX;		// texture X
	txtr[1]=tY;		// texture Y
	txtr[2]=1.0f;	// texture coord set flag
	return true;
}
bool jvs3d_terrain::getTextureCoord(double &tX, double &tY, int x, int y)
{

	if( !m_pTextureCoords || 
		!(x>=0 && y>=0 && x<m_heightMap.m_width && y<m_heightMap.m_height) )
		return false;
	long offset=(x+(m_heightMap.m_width*y))*3;
	double *txtr=&m_pTextureCoords[offset];
	if( m_pTextureCoords && txtr[2] && x>=0 && y>=0)
	{
		tX=txtr[0];
		tY=txtr[1];
		return true;
	}
	return false;
}

bool jvs3d_terrain::getPosition(jvs3d_vertice &pos)
{
	pos.set(&m_position);
	return true;
}

bool jvs3d_terrain::getPosition(int *x, int *z, jvs3d_vertice m_pos)
{
	jvs3d_vertice pos;
	pos.set(&m_pos);
	untransform(pos);
	*x = pos.m_data[0];
	*z = pos.m_data[2];
	if( *x>=0 && *x<m_heightMap.m_width && *z>=0 && *z<m_heightMap.m_height )
		return true;
	else return false;
}

double jvs3d_terrain::getHeightAt(double x, double z)
{
	int ax, ay;
	jvs3d_vertice pos;
	pos.set(x,0,z);
	getPosition(&ax, &ay, pos);
	getPosition(pos,ax,ay);
	return pos.m_data[1];
}

bool jvs3d_terrain::getPosition(jvs3d_vertice &pos, int x, int z)
{
	unsigned char pel[3];
	
	if( x<m_heightMap.m_width &&
		z<m_heightMap.m_height &&
		m_boolMapLoaded && 
		m_heightMap.getPixel((char*)&pel,x,z) && 
		m_pCamera )
	{
		if(1==0) 
		{
			printf("pel=>%u, %u, %u;", pel[0], pel[1], pel[2] );
			printf("pos=>%u, %u, %u\n", x, z, pel[2]);
		}
		pos.set(
			x, 
			( 
				(double)(pel[2]?pel[2]:1) * 
				(double)(pel[1]?pel[1]:1) * 
				(double)(pel[0]?pel[0]:1) 
			), 
			z );
		transform(pos);				// transform to world coordinates
		return true;
	} else
	{
		pos.set(x,0,z);
		transform(pos);				// transform to world coordinates
		return false;
	}
}

void * jvs3d_terrain::getTextureIds(int x, int z, int &num)
{
	unsigned char pel[sizeof(unsigned long)];

	unsigned long *textureIds=null;
	unsigned char *alphaValues=null;
	//void *ptr;
	unsigned char **lists=null;
	unsigned long tid;
	unsigned long idx=0;

	jvs3d_terrain_texture_mapping *pTT=null;
	memset(pel,0,sizeof(unsigned long));
	if( m_pTextureMappings && 
		x<m_heightMap.m_width &&
		z<m_heightMap.m_height &&
		m_pCamera )
	{
		num=0;
		pTT=(jvs3d_terrain_texture_mapping*)m_pTextureMappings->getTop();
		while(pTT)
		{
			if( pTT->getPixel((char *)&pel,x,z) )
			{
				for(int i=0;i<4;i++)
				{
					tid=pTT->getTextureId(i);
					if(tid && pel[i])
					{
						num++;
						if(lists==null)
							lists=(unsigned char **)malloc(sizeof(unsigned long *)*2);
						//ptr=(void *)textureIds;
						textureIds=(unsigned long *)realloc(textureIds,sizeof(unsigned long)*num);
						//if(ptr!=textureIds) free(ptr);
						//ptr=(void *)alphaValues;
						alphaValues=(unsigned char *)realloc(alphaValues,sizeof(unsigned char)*num);
						//if(ptr!=alphaValues)free(ptr);
						if(textureIds && alphaValues)
						{
							idx=(num-1);
							textureIds[idx]=tid;
							alphaValues[idx]=pel[i];
						} else { free(textureIds); free(alphaValues); return false; }
					} // if(tid && pel[i])
				} // for(i=0;i<4;i++)
			} // if( pTT->getPixel(pel,x,z) )
			pTT=(jvs3d_terrain_texture_mapping*)pTT->getNext();
		} // while(pTT)
		// sort, maybe inefficiently (?) so that the highest alpha values are drawn first
		/*for(int i=0; i<num; i++)
		{
			for(int e=i;e<num;e++)
			{
				if(alphaValues[e]>alphaValues[i])
				{
					b=alphaValues[e];
					alphaValues[e]=alphaValues[i];
					alphaValues[i]=b;
					a=textureIds[e];
					textureIds[e]=textureIds[i];
					textureIds[i]=a;
				}
			}
		}*/
		if(num)
		{
			lists[0]=(unsigned char *)textureIds;
			lists[1]=alphaValues;
		}
		return (unsigned char *)lists;
	} else { num=0; return false; }
}

///////////////////////////////
// Coordinate transformation //
///////////////////////////////

bool jvs3d_terrain::transform(jvs3d_vertice &t)
{
	t*=m_scale;
	t+=m_translate;
	t-=m_position;
	t.m_data[1]*=m_scaleHeightCorrect;

	return true;
}
bool jvs3d_terrain::untransform(jvs3d_vertice &t)
{
	t.m_data[1]/=.1;
	t+=m_position;
	t-=m_translate;
	t*=1.0f/m_scale;
	t.m_data[1]*=1.0f/m_scaleHeightCorrect;
	return true;
}

//////////////////////////
// Debug visualizations //
//////////////////////////

bool jvs3d_terrain::drawAllTerrain(void)
{
	double h,l;
	jvs3d_vertice t;
	
	for(h=m_heightMap.m_height;h>0;h--)
	{
		glBegin(GL_LINE_STRIP);
		for(l=0;l<m_heightMap.m_width;l++)
		{
			getPosition(t,l,h);			// fetch from map in world coordinates
			if(t.m_data[0]>0 && t.m_data[2]>0)
				glColor3f(.5,0,0);
			else if(t.m_data[0]<0 && t.m_data[2]<0)
				glColor3f(.5
				,.5,0);
			else glColor3f(0,.5,0);
			m_pCamera->transform(t);	// transform to camera coordinates
			t.drawGL(null);
		}
		glEnd();
	}
	return true;
}

bool jvs3d_terrain::drawGrid(void)
{
	double h;
	jvs3d_vertice t;
	
	for(h=m_heightMap.m_height;h>0;h-=10)
	{
		glBegin(GL_LINE_STRIP);

			t.set(h,0,0);
			transform(t);				// transform to world coordinates
			m_pCamera->transform(t);	// transform to camera coordinates
			t.drawGL(null);

			t.set(h,0,(double)m_heightMap.m_width);
			transform(t);				// transform to world coordinates
			m_pCamera->transform(t);	// transform to camera coordinates
			t.drawGL(null);

		glEnd();
	}
	for(h=m_heightMap.m_width;h>0;h-=10)
	{
		glBegin(GL_LINE_STRIP);

			t.set(0,0,h);
			transform(t);
			m_pCamera->transform(t);	// transform to camera coordinates
			t.drawGL(null);

			t.set((double)m_heightMap.m_width,0,h);
			transform(t);
			m_pCamera->transform(t);	// transform to camera coordinates
			t.drawGL(null);

		glEnd();
	}
	return true;

}

/////////////////////////
// Primary Render Core //
/////////////////////////

bool jvs3d_terrain::calculateVertices(void)
{	
	int pts[7][4],							// current lod points
		opts[7][4],							// original points
		lod1, lod2,
		p[4],								// position
		ep[2][4],							// lowest & greatest along x
		i;									// temp
	int xStart, xEnd;
	int yCur, xCur;
	double	rad;							// radians of slope

	double	slopes[3][4],					// lowest to highest
			tmp;
	
	// variables pertaining to LOD
	int lastTri=4;
	int curTri, curLOD, lod[4] = {1,2,4,8};
	int xDownLODCur, yDownLODCur;
	bool boolThisSameLOD, boolThisDiffLOD;
	int thisSameLODNum=0, thisDiffLODNum=0, lastSameLODNum=0, lastDiffLODNum=0;
	int *thisSameLODBuf=null;
	int *thisDiffLODBuf=null;
	int *lastSameLODBuf=null;
	int *lastDiffLODBuf=null;
	bool upLOD=0;
	double lodEdgeSlopeAbs;
	double lodEdgeSlope;

	// debugging vars
	int borderMultiply=0;
	bool a;
	int e;

	double frustEnd;

	int hmPos[4][2];	// height map pos
	jvs3d_vertice 
			rot,
			camLookAt,	// the current position the camera is looking at
			camPos;		// the current position of the camera
			
	char // for all cai in gai incrementing gai and cai by ai
		gai, // group axis index
		cai, // cell axis index
		ai;  // increment group axis by
	bool 
		stop=false, 
		stop1=false, 
		stop2=false;	// done
	jvs3d_camera *pcam=getCamera();
	jvs3d_vertice tempPos[5];
	
	memset(p,0,sizeof(int)*3);	// zero out current point;
	memset(ep,0,sizeof(int)*2*4);	// "right" point
	memset(pts,0,sizeof(int)*7*4);	// "left" point
	memset(slopes,0,sizeof(double)*3*4);	// "left" point

	if(DBGTERR_OL)
	{
	#ifdef _WIN32
		system("cls");
	#else
		system("clear");
	#endif
	}

	if(!pcam) 
		return false;

	if(DBGTERR_OL) 
		printf("Entry test:\n");

	if( pcam->getPosition(camPos) && pcam->getLookAt(camLookAt) )
	{
		camLookAt*=50;
		camPos-=camLookAt;
		camLookAt*=1/50;
	} else return false;

	if(1)
	{
		getPosition(&opts[0][0],&opts[0][2],camPos);

		camLookAt+=camPos;
		if(DBGTERR_OL)
		{
			printf("initial conditions: \n");
			printf("camPos.m_data[0],[1],[2]=%f,%f,%f\n",
				camPos.m_data[0],camPos.m_data[1],camPos.m_data[2] );
			printf("camLookAt.m_data[0],[1],[2]=%f,%f,%f\n",
				camLookAt.m_data[0],camLookAt.m_data[1],camLookAt.m_data[2] );
			printf("pts[2][0],[2]=%u,%u\n\n",pts[2][0],pts[2][2]);
			printf("calculated presets: \n");
		}

		// DETERMINATION OF THE OTHER TWO POINTS
		{
			// line which caps the triangl
			m_pCamera->getRotation(rot);
			rad = jvs3d_math::DegreesToRadians(270-rot.m_data[1]);  // arctan slope to get radians
			rad = cos(rad - (pcam->getFOVRadians()/2))?rad:rad-(2*JVS_PI/180);
			rad = cos(rad + (pcam->getFOVRadians()/2))?rad:rad-(2*JVS_PI/180);
			rad = sin(rad - (pcam->getFOVRadians()/2))?rad:rad-(2*JVS_PI/180);
			rad = sin(rad + (pcam->getFOVRadians()/2))?rad:rad-(2*JVS_PI/180);

			// calculate points along the way;
			//  1,2 are the points making the near boundary
			//  3,4 make the middle boundary
			//  5,6 make the far boundary
			// each gai and cai inc/dec by 1,2,3 
			//  respective to containing boundary
			for(i=0;i<3;i++)
			{
				lod1=(i*2)+1;
				lod2=(i*2)+2;
				frustEnd = pcam->getFrustumEnd(i) * m_scale;
				opts[lod1][0] = cos(rad - (pcam->getFOVRadians()/2))*frustEnd+opts[0][0];
				opts[lod1][2] = sin(rad - (pcam->getFOVRadians()/2))*frustEnd+opts[0][2];
				opts[lod1][3] = (i*2)+1;
				opts[lod2][0] = cos(rad + (pcam->getFOVRadians()/2))*frustEnd+opts[0][0];
				opts[lod2][2] = sin(rad + (pcam->getFOVRadians()/2))*frustEnd+opts[0][2];
				opts[lod2][3] = (i*2)+2;

				if(1==0)
				{
					glLineWidth(2);
					glColor3f(1,1,1);
					glBegin(GL_LINES);
						tempPos[0].set(opts[lod1][0],0,opts[lod1][2]);
						transform(tempPos[0]);
						m_pCamera->transform(tempPos[0]);
						tempPos[0].m_data[1]=3;
						tempPos[0].drawGL(null);
						
						tempPos[0].set(opts[lod2][0],0,opts[lod2][2]);
						transform(tempPos[0]);
						m_pCamera->transform(tempPos[0]);
						tempPos[0].m_data[1]=3;
						tempPos[0].drawGL(null);
					glEnd();
				}
			}
			opts[0][3] = 0;

			if(DBGTERR_VIS)
			{
				getPosition(tempPos[0],opts[0][0],opts[0][2]);
				getPosition(tempPos[1],opts[1][0],opts[1][2]);

				m_pCamera->transform(tempPos[0]);
				m_pCamera->transform(tempPos[1]);

				glColor3f(1,1,0);
				glPointSize(4.0);
				glBegin(GL_POINTS);
					glVertex3dv(tempPos[0].m_data);
					glVertex3dv(tempPos[1].m_data);
				glEnd();
				glPointSize(1.0);
			}
		}
/*=====================*/
/* BEGIN TRIANGLE LOOP */
/*=====================*/
		for(curTri=lastTri;curTri>=0;curTri--)
		{
			if(DBGTERR_OL)
				printf("+===========\nTriangle %d\n+===========\n",curTri);
			///////////////////////////
			// SETUP LEVEL OF DETAIL //
			///////////////////////////

			// LOD buffer maitenance
			if(thisSameLODBuf)
			{
				if(lastSameLODBuf) 
					free(lastSameLODBuf);
				lastSameLODBuf=thisSameLODBuf;
				lastSameLODNum=thisSameLODNum;
				thisSameLODNum=0;
				thisSameLODBuf=null;
			}
			if(thisDiffLODBuf)
			{
				if(lastDiffLODBuf) 
					free(lastDiffLODBuf);
				lastDiffLODBuf=thisDiffLODBuf;
				lastDiffLODNum=thisDiffLODNum;
				thisDiffLODNum=0;
				thisDiffLODBuf=null;
			}

			////////////////////////
			//BEGIN TRIANGLE SETUP//
			////////////////////////

			// set the pts array to the current triangle we're rendering
			curLOD=0;
			curLOD=(curTri==1 || curTri==2)?1:curLOD;
			curLOD=(curTri==3 || curTri==4)?2:curLOD;
			ai=lod[curLOD];
			for(i=0;i<3;i++)
			{
				pts[i][0]=opts[i+curTri][0];
				pts[i][1]=opts[i+curTri][1];
				pts[i][2]=opts[i+curTri][2];
				pts[i][3]=opts[i+curTri][3];
			}

			// slope of the LOD edge
			{
				rad=15;
				// lowest to middle
				tmp=pts[1][0]-pts[2][0];
				lodEdgeSlope=(pts[1][2]-pts[2][2])/(tmp!=0?tmp:rad);	
				lodEdgeSlope=lodEdgeSlope;
				lodEdgeSlopeAbs=fabs(lodEdgeSlope);
				if(DBGTERR_OL && curTri%2==0)
					printf("LOD edge slope, where x=y*m+b: fabs(%f)=%f\n",lodEdgeSlope,lodEdgeSlopeAbs);
			}


			// ORDERING OF TRIANGLES BASED ON Z-AXIS
			jvs3d_math::orderArray2dx4(-1,2,3,pts);
			
			if(DBGTERR_OL)
				for(i=0;i<3;i++)
					printf("pts[%d][0],[2]=%d,%d\n",i,pts[i][0],pts[i][2]);

			// DETERMINE THE SLOPES OF THE THREE LINES INVOLVED,
			// TRACING THE EDGES OF THE TRIANGLE
			// also flag each slope for whether it is a LOD edge
			// or an internal edge to the trapezoid
			{
				rad=15;
				// lowest to middle
				tmp=pts[0][0]-pts[1][0];
				slopes[0][0]=(pts[0][2]-pts[1][2])/(tmp!=0?tmp:rad);	
				slopes[0][0]=1/slopes[0][0];
				if( ( pts[0][3]+1==pts[1][3] && pts[0][3]%2==1 ) ||
					( pts[1][3]+1==pts[0][3] && pts[1][3]%2==1 ) )
					slopes[1][0]=1;
				else
				if( ( pts[0][3] && pts[0][3]+1==pts[1][3] && pts[0][3]%2==0 ) ||
					( pts[1][3] && pts[1][3]+1==pts[0][3] && pts[1][3]%2==0 ) )
					slopes[1][0]=2;
				else slopes[1][0]=0;

				// middle to highest
				tmp=pts[1][0]-pts[2][0];
				slopes[0][1]=(pts[1][2]-pts[2][2])/(tmp!=0?tmp:rad);	
				slopes[0][1]=1/slopes[0][1];
				if( ( pts[1][3] && pts[1][3]+1==pts[2][3] && pts[1][3]%2==1 ) ||
					( pts[2][3] && pts[2][3]+1==pts[1][3] && pts[2][3]%2==1 ) )
					slopes[1][1]=1;
				else 
				if( ( pts[1][3] && pts[1][3]+1==pts[2][3] && pts[1][3]%2==0 ) ||
					( pts[2][3] && pts[2][3]+1==pts[1][3] && pts[2][3]%2==0 ) )
					slopes[1][1]=2;
				else slopes[1][1]=0;

				// lowest to highest
				tmp=pts[0][0]-pts[2][0];
				slopes[0][2]=(pts[0][2]-pts[2][2])/(tmp!=0?tmp:rad);
				slopes[0][2]=1/slopes[0][2];
				if( ( pts[0][3]+1==pts[2][3] && pts[0][3]%2==1 ) ||
					( pts[2][3]+1==pts[0][3] && pts[2][3]%2==1 ) )
					slopes[1][2]=1;
				else 
				if( ( pts[0][3]+1==pts[2][3] && pts[0][3]%2==0 ) ||
					( pts[2][3]+1==pts[0][3] && pts[2][3]%2==0 ) )
					slopes[1][2]=2;
				else slopes[1][2]=0;
				if(DBGTERR_OL)
					printf("slopes[0][0]=%f,slopes[0][1]=%f,slopes[0][2]=%f\n",slopes[0][0],slopes[0][1],slopes[0][2]);
			}

			// DETERMINE WHAT AXIS TO INCREMENT ALONG AND WHICH DIRECTION TO INCREMENT
			{
				gai=2;	// group axis index
				cai=0;	// cell axis index
				//ai=1;

				if(DBGTERR_OL) 
				{
					//printf("axis adjusted slopes : ml=%f; mr=%f; me=%f\n",ml,mr,me);
					printf("gai=%u,cai=%u,ai=%i\n\n",gai,cai,ai);
				}
			}

			//////////////////////////////////////////////////////////
			// DETERMINE LOW AND HIGH VALUES, ALIGN PER CURRENT LOD //
			//////////////////////////////////////////////////////////

			// align the top and bottom points per the LOD
			pts[0][gai]=(pts[0][gai]/lod[curLOD+upLOD])*lod[curLOD+upLOD];
			pts[0][gai]=pts[0][gai]>0?pts[0][gai]:0;

			pts[2][gai]=(pts[2][gai]/lod[curLOD+upLOD])*lod[curLOD+upLOD];
			pts[2][gai]=pts[2][gai]>0?pts[2][gai]:0;

			// if the inverse LOD edge slope is less than .5
			// then the slope is greater than 2, so create our connection 
			// traversing low x to great x rather than low z to great z
			if( lodEdgeSlopeAbs<.5 && curTri%2==0 && curTri<4)
			{
				// set the starting x to the lowest x value of the last LOD
				xStart=opts[curTri+1][0]<opts[curTri+2][0]?opts[curTri+1][0]:opts[curTri+2][0];
				xEnd=opts[curTri+1][0]>opts[curTri+2][0]?opts[curTri+1][0]:opts[curTri+2][0];
				xCur=((int)(xStart/lod[curLOD+1]))*lod[curLOD+1];

				if(DBGTERR_OL)
				{
					printf("curTri=%i, xStart=%i, xEnd=%i\n",curTri,xStart,xEnd);
					printf("%i=>%i,inc by %i;", xCur, xEnd, lod[curLOD+1]);
				}

				if(xStart<xEnd)
				for( xCur=xCur; xCur<xEnd; xCur+=lod[curLOD+1] )
				{
					xDownLODCur = xCur;

					// determine y value at this spot using slope; it is an inverse slope because here
					// f(x)=y rather than f(y)=x, like in where we are calculating the beginning and ending x
					// values for each triangle
					calcXY(lodEdgeSlope,lod[curLOD+1],opts[curTri+1][0],opts[curTri+1][2],xCur,yCur);
					calcXY(lodEdgeSlope,lod[curLOD],opts[curTri+1][0],opts[curTri+1][2],xDownLODCur,yDownLODCur);

					// find Y in lastDiffLODBuf closest to these two
					
					// get current position in this LOD
					getPosition(tempPos[0],xDownLODCur,yDownLODCur);

					if(DBGTERR_OL)
					{
						if(glIsEnabled(GL_TEXTURE_2D))
						{
							a=1;
							glDisable(GL_TEXTURE_2D);
						} else a=0;

						glColor3f(255,255,255);

						printf("%i,%i;",xCur, yCur);
						m_pCamera->transform(tempPos[0]);
						glPointSize(10);
						glBegin(GL_POINTS);
						tempPos[0].drawGL(null);
						glEnd();
					}

					//if(xCur==xDownLODCur)
					{
						// get last LOD position nearest current position
						getPosition(tempPos[1],xCur,yCur); 

						if(DBGTERR_OL)
						{
							m_pCamera->transform(tempPos[1]);
							glLineWidth(5);
							glBegin(GL_LINES);
							tempPos[0].drawGL(null);
							tempPos[1].drawGL(null);
							glEnd();
						}

						if(a)
							glEnable(GL_TEXTURE_2D);
					}
						
					// TODO: use the values above to connect the two edges

				} // for( xCur=((int)(xStart/lod[curLOD+1]))*lod[curLOD+1]; xCur<xEnd; xCur+=lod[curLOD+1] )
			} // if( lodEdgeSlopeAbs<.5 && curTri%2==0 )

			if(DBGTERR_OL) printf("\n");

/**********************/
/*START THE OUTER LOOP*/
/**********************/

			// currently each triangle renders a scanline from lowest z to highest z value
			for(p[gai]=pts[0][gai]; p[gai]<pts[2][gai]; p[gai]+=lod[curLOD])
			{
				// determine point along line from lowest to midpoint 
				// or from midpoint to highpoint
				ep[0][gai]=ep[1][gai]=p[gai];

				// use lowest to middle or middle to highest slope
				// per whether we've passed the middle point or not
				if(p[gai]>=pts[1][gai])
				{
					calcXY(slopes[0][1],lod[curLOD+upLOD],pts[2][gai],pts[2][cai],p[gai],ep[0][cai]);
					//calcXY(slopes[0][1],1,pts[2][gai],pts[2][cai],p[gai],ep[0][cai]);
					if(slopes[1][1] && pts[2][3])
						ep[0][3]=slopes[1][1];
					else ep[0][3]=0;
				} else 
				{
					calcXY(slopes[0][0],lod[curLOD+upLOD],pts[1][gai],pts[1][cai],p[gai],ep[0][cai]);
					//calcXY(slopes[0][0],1,pts[1][gai],pts[1][cai],p[gai],ep[0][cai]);
					if(slopes[1][0] && pts[1][3])
						ep[0][3]=slopes[1][0];
					else ep[0][3]=0;
				}

				// determine point along line from lowest to highest
				calcXY(slopes[0][2],lod[curLOD+upLOD],pts[0][gai],pts[0][cai],p[gai],ep[1][cai]);
				//calcXY(slopes[0][2],1,pts[0][gai],pts[0][cai],p[gai],ep[1][cai]);
				
				if(slopes[1][2] && pts[0][3])
					ep[1][3]=slopes[1][2];
				else ep[1][3]=0;

				// order from least to greatest : l to r
				jvs3d_math::orderArray2dx4(-1,cai,2,ep);
				
				// if it's a triangle of a different LOD edge
				// then the last recorded was the same LOD edge
				if(curTri%2==1 && curTri<lastTri && curTri)
					// scan through recorded for same y values
					for(int i=0; i<lastSameLODNum; i++)
					{
						if( lastSameLODBuf[i*2+1]==p[2])
						{
							// determine whether the recorded x for this y
							// is closer to the leading or ending square to be
							// rendered in the scanline
							// set the start or ending point based on that.
							if( abs(abs(ep[0][cai])-abs(lastSameLODBuf[i*2])) < 
								abs(abs(ep[1][cai])-abs(lastSameLODBuf[i*2])) )
								// in theory the starting point should be the
								// ending point of the last, so we should subtract
								// in order to accomodate that...doesn't work however.
								ep[0][cai]=lastSameLODBuf[i*2];
							else ep[1][cai]=lastSameLODBuf[i*2];//-lod[curLOD];
						}
					}

				//ep[0][cai]=(ep[0][cai]/lod[curLOD+upLOD])*lod[curLOD+upLOD];
				ep[0][cai]=ep[0][cai]>0?ep[0][cai]:0;

				//ep[1][cai]=(ep[1][cai]/lod[curLOD+upLOD])*lod[curLOD+upLOD];
				ep[1][cai]=ep[1][cai]>0?ep[1][cai]:0;

//////////////////////////
// START THE INNER LOOP //
//////////////////////////

				for( p[cai] = ep[0][cai]; p[cai] < ep[1][cai]; p[cai]+=lod[curLOD] )
				{

					if( p[cai]-lod[curLOD]>0 && 
						p[cai]<m_heightMap.m_width-lod[curLOD] && 
						p[gai]-lod[curLOD]>0 && 
						p[gai]<m_heightMap.m_height-lod[curLOD])
					{
						hmPos[0][0]=p[0];
						hmPos[0][1]=p[2];
						hmPos[1][0]=p[0]-lod[curLOD];
						hmPos[1][1]=p[2];
						hmPos[2][0]=p[0];
						hmPos[2][1]=p[2]-lod[curLOD];
						hmPos[3][0]=p[0]-lod[curLOD];
						hmPos[3][1]=p[2]-lod[curLOD];

						// draw the square associated with the position
						if(m_boolDrawGL)
						{

							glColor3f(.5,.5,.5);
							glLineWidth(2);

							/////////////////////////////////////////////////////////
							// RECORD AND FLAGSET IF WE ARE ON AN INTERESTING EDGE //
							/////////////////////////////////////////////////////////							
							if( (p[cai]==ep[0][cai] && ep[0][3]==1) ||
								((p[cai]==ep[1][cai] || p[cai]==ep[1][cai]-lod[curLOD]) && ep[1][3]==1) )
							{
								// we're on an edge shared between two different level of detail triangles
								if(curTri%2==0 && curTri<lastTri)
								{
									boolThisDiffLOD=true;
									boolThisSameLOD=false;
								} else { boolThisSameLOD=boolThisDiffLOD=false; }

								// it's an edge of the tri on a different LOD of a tri 
								thisDiffLODNum++;
								thisDiffLODBuf=(int*)realloc(thisDiffLODBuf,thisDiffLODNum*2*sizeof(int));
								thisDiffLODBuf[(thisDiffLODNum-1)*2]=p[0];
								thisDiffLODBuf[(thisDiffLODNum-1)*2+1]=p[2];
							} else
							if( (p[cai]==ep[0][cai] && ep[0][3]==2) ||
								((p[cai]==ep[1][cai] || p[cai]==ep[1][cai]-lod[curLOD]) && ep[1][3]==2) )
							{
								// we're on an edge shared between two same level of detail triangles

								if(curTri%2 && curTri<lastTri)
								{
									boolThisDiffLOD=false;
									boolThisSameLOD=true;
								} else { boolThisSameLOD=boolThisDiffLOD=false; }

								// it's on a "LOD is the same" edge
								thisSameLODNum++;
								thisSameLODBuf=(int*)realloc(thisSameLODBuf,thisSameLODNum*2*sizeof(int));
								thisSameLODBuf[(thisSameLODNum-1)*2]=p[0];
								thisSameLODBuf[(thisSameLODNum-1)*2+1]=p[2];
							} else { boolThisSameLOD=boolThisDiffLOD=false; }

							// actually render the two triangles
							if(1==0)
							{
								getPosition(tempPos[0],hmPos[0][0],hmPos[0][1]);
								m_pCamera->transform(tempPos[0]);
								glDisable(GL_TEXTURE_2D);
								glPointSize(4.0);
								if(curLOD==0)
									glColor3f(1,0,0);
								else if(curLOD==1)
									glColor3f(0,1,0);
								else glColor3f(0,0,1);
								glBegin(GL_POINTS);
								tempPos[0].drawGL(null);
								glEnd();
							} else drawPatch(hmPos);							

							if( boolThisDiffLOD && lastDiffLODBuf && lodEdgeSlopeAbs>.5)
							{
								a=0;
								// cycle through the lastDiffLODBuf and find the one which matches
								// the current heightMap Y value
								for(i=0; i<lastDiffLODNum; i++)
								{
									if( lastDiffLODBuf[i*2+1]==hmPos[0][1] )
									{
										tempPos[0].m_data[0]=lastDiffLODBuf[i*2];
										tempPos[0].m_data[1]=lastDiffLODBuf[i*2+1];
										a=1;
									}
								}
								if(a==1)
								{
									// get the stored position from the last tri's edge
									getPosition(tempPos[0],
										tempPos[0].m_data[0],
										tempPos[0].m_data[1]);
									m_pCamera->transform(tempPos[0]);

									// get the world coords for this position
									getPosition(tempPos[2],
										hmPos[0][0],
										hmPos[0][1]);
									m_pCamera->transform(tempPos[2]);

									if(glIsEnabled(GL_TEXTURE_2D))
									{
										glDisable(GL_TEXTURE_2D);
										a=1;
									} else a=0;
									glDepthMask(GL_FALSE);
									glDepthFunc(GL_ALWAYS);

									glLineWidth(2.0);
									glBegin(GL_LINES);

									glColor3f(0,1,1);
								
									tempPos[2].drawGL(null);
									tempPos[0].drawGL(null);

									glEnd();

									glPointSize(8);
									glColor3f(1,1,0);
									glBegin(GL_POINTS);
									tempPos[0].drawGL(null);
									glEnd();

									glDepthMask(GL_TRUE);
									glDepthFunc(GL_LESS);
						
									if(a) glEnable(GL_TEXTURE_2D);
								}
							}
						}
					} // p[0]>0 && p[0]<m_heightMap.m_width && p[2]>0 && p[2]<m_heightMap.m_height
				} // for( p[cai] = s ; p[cai] < r[cai] ; p[cai]++ )

////////////////////////
// END THE INNER LOOP //
////////////////////////

			} // gai loop; for(p[gai]=pts[0][gai]; p[gai]<pts[2][gai]; p[gai]+=lod[curLOD])

/********************/
/*END THE OUTER LOOP*/
/********************/
			
		} // for(curTri=0;curTri<3;curTri++)
		if(thisDiffLODBuf) free(thisDiffLODBuf);
		if(lastDiffLODBuf) free(lastDiffLODBuf);
		if(thisSameLODBuf) free(thisSameLODBuf);
		if(lastSameLODBuf) free(lastSameLODBuf);
	} else return false;
	return true;
}

void jvs3d_terrain::drawPatch(int hmPos[4][2])
{
	// variables for texturing
	int t2bn[4];					// number fo textures to blend
	unsigned char **t2bl[4];	// list of texture_id's and alpha values
	GLuint listIdForMultiTex=0;
	static jvs3d_material *pThisMat=null, *pLastMat=null;
	jvs3d_vertice norms[4];
	int i,d,a,b,s;
	jvs3d_vertice tempPos[5];

	// get the pos and txtr coord
	for(i=0;i<4;i++)
	{
		getPosition(tempPos[i],hmPos[i][0],hmPos[i][1]);
		//tempPos[i].m_data[1]=0;
		tempPos[i].m_temp[3]=
			getTextureCoord(
				tempPos[i].m_temp[0],tempPos[i].m_temp[1],
				hmPos[i][0],hmPos[i][1] );
		tempPos[i].m_ptr=&hmPos[i];
	}

	for(i=0;i<4;i++)
	{
		m_pCamera->transform(tempPos[i]);
	}

	// get texturing information if it exists for this block
	for(i=0;i<4;i++)
	{
		t2bl[i]=(unsigned char **)
			getTextureIds(
				hmPos[i][0], hmPos[i][1], 
				t2bn[i]
			);
	}
	if( !t2bn[0] )// || (p[cai]==l[cai] && p[cai]==r[cai]-1 ) )
	{
		glDisable(GL_TEXTURE_2D); 
		glColor3f(.5f,.5f,.5f); 
		pLastMat=0;
		pThisMat=0;
		t2bn[0]=0;
		
		if(t2bl[0])
		{
			for(i=0;i<4;i++)
			{
				delete t2bl[i][0];
				delete t2bl[i][1];
				delete t2bl[i];
				t2bl[i]=null;
			}
		}
	}

	b=t2bn[0]?t2bn[0]:1;
	a=0;

	norms[0].calculateNormal(tempPos[0],tempPos[1],tempPos[2]);
	norms[1].calculateNormal(tempPos[3],tempPos[2],tempPos[1]);
	norms[2].calculateNormal(tempPos[0],tempPos[1],tempPos[3]);
	norms[3].calculateNormal(tempPos[3],tempPos[2],tempPos[0]);

	for( s=a; s<b; s++ )
	{
		if(t2bn[0])
		{
			if( s==1 )
			{
				glEnable(GL_BLEND);
				glDepthMask(GL_FALSE);
				glDepthFunc(GL_EQUAL);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			}

			pThisMat=
				(jvs3d_material*)
				m_pMaterials->getByPKId( ((unsigned long*)t2bl[0][0])[s] );

			if( pThisMat && !glIsEnabled(GL_TEXTURE_2D))
				glEnable(GL_TEXTURE_2D);
			if( pThisMat && pThisMat!=pLastMat )
			{
				pThisMat->bindGL();
			} 
			pLastMat=pThisMat;									
		}

		{
			glBegin(GL_TRIANGLES);
			glNormal3dv(norms[0].m_data);
			for(i=0;i<3;i++)
			{
				if(t2bn[i]==t2bn[0])
				{
					for(d=0;d<t2bn[i];d++)
					{
						if( ((unsigned long*)t2bl[i][0])[d]==
							((unsigned long*)t2bl[0][0])[s] ) 
							 break;
					} if(d==t2bn[i]) d=0;

					glColor4f( 0, 0, 0, 
						(float)t2bl[i][1][d]*(1.0f/255.0f) );
				}

				if(pThisMat && tempPos[i].m_temp[3])
					glTexCoord2d(tempPos[i].m_temp[0],tempPos[i].m_temp[1]);

				tempPos[i].drawGL(null);
			}
			glEnd();

			glBegin(GL_TRIANGLES);
			glNormal3dv(norms[1].m_data);
			for(i=3;i>0;i--)
			{
				if(t2bn[i]==t2bn[0])
				{
					for(d=0;d<t2bn[i];d++)
					{
						if( ((unsigned long*)t2bl[i][0])[d]==
							((unsigned long*)t2bl[0][0])[s]) 
							 break;
					} if(d==t2bn[i]) d=0;

					glColor4f( 0, 0, 0, 
						(float)t2bl[i][1][d]*(1.0f/255.0f) );
				}

				if(pThisMat && tempPos[i].m_temp[3])
					glTexCoord2d(tempPos[i].m_temp[0],tempPos[i].m_temp[1]);

				tempPos[i].drawGL(null);
			}
			glEnd();
		}
	}
	// clean up after multi texturing
	if( t2bn[0]>1 )// && listIdForMultiTex )
	{
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);
	}
	if(t2bl[0])
	{
		for(i=0;i<4;i++)
		{
			delete t2bl[i][0];
			delete t2bl[i][1];
			delete t2bl[i];
			t2bl[i]=null;
		}
	}
}

// really so that i may be certain that i am using the same formula for every calculated point
void jvs3d_terrain::calcXY(double slope, int alignOn, int ptX, int ptY, int x, int &y)
// slope:   slope of the line
// alignOn: align by modulus equals 0
// ptX: passes thru x
// ptY: passes thru y
// x: f(x)=y
{
	y=(int)( ((double)(x-ptX)) * slope + ((double)ptY) );
	y=(y/alignOn)*alignOn;
}

/***********************************
 *TERRAIN TEXTURE MAPPING FUNCTIONS*
 ***********************************/

jvs3d_terrain_texture_mapping::jvs3d_terrain_texture_mapping()
{
	memset(&m_textureIds,0,sizeof(unsigned long)*4);
}

jvs3d_terrain_texture_mapping::~jvs3d_terrain_texture_mapping() {}

void jvs3d_terrain_texture_mapping::getTextureIds(
	unsigned long &red, 
	unsigned long &green, 
	unsigned long &blue, 
	unsigned long &alpha)
{
	red=m_textureIds[0]; 
	green=m_textureIds[1];
	blue=m_textureIds[2];
	alpha=m_textureIds[3];
}
void jvs3d_terrain_texture_mapping::setTextureIds(
	unsigned long red, 
	unsigned long green, 
	unsigned long blue, 
	unsigned long alpha)
{
	m_textureIds[0]=red; 
	m_textureIds[1]=green; 
	m_textureIds[2]=blue;
	m_textureIds[3]=alpha;
}
unsigned long jvs3d_terrain_texture_mapping::getRed(void)	{ return m_textureIds[0]; }
unsigned long jvs3d_terrain_texture_mapping::getGreen(void) { return m_textureIds[1]; }
unsigned long jvs3d_terrain_texture_mapping::getBlue(void)	{ return m_textureIds[2]; }
unsigned long jvs3d_terrain_texture_mapping::getAlpha(void)	{ return m_textureIds[3]; }
unsigned long jvs3d_terrain_texture_mapping::getTextureId(int idx) { if(idx<4) return m_textureIds[idx]; else return false; }
