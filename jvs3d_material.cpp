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
 * class jvs3d_material - start
 ***************************************************************/

jvs3d_material::jvs3d_material()
{
	m_data=null;
	m_filepath=null;
	m_imageType=0;
	m_materialType=0;
	m_glTextureObject=0;
}

jvs3d_material::~jvs3d_material()
{
	if(m_data) delete m_data;
	if(m_filepath) delete m_filepath;
#ifdef _MSC_VER
	if(m_glTextureObject) glDeleteTextures(1,&m_glTextureObject);
#endif
}

bool jvs3d_material::getPixel(char *pVals, int x, int y)
{
	unsigned long offset=((y * m_width)+x)*3;
	if(m_width)
	{
		*(pVals)=*(char*)(m_data+offset);
		*(pVals+1)=*(char*)(m_data+offset+1);
		*(pVals+2)=*(char*)(m_data+offset+2);
		if(DBGMAT)
		{
			printf("RGBA@%x=>%i,%i,%i;",
				m_data+offset,
				*(char*)(m_data+offset),
				*(char*)(m_data+offset+1),
				*(char*)(m_data+offset+2) );
		}
		return true;
	} else return false;
}

bool jvs3d_material::bindGL(void)
{
	if(m_data || m_glTextureObject)
	{
		if(!glIsEnabled(GL_TEXTURE_2D))
			glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
		glBindTexture(GL_TEXTURE_2D, m_glTextureObject);
		return true;
	}
	return false;
}

bool jvs3d_material::initializeGL(const char type[3], const char *filename)
{
	jvs3d_image_bmp_infoheader bmp;
	const unsigned char * ret=gluErrorString(glGetError());
	if( _stricmp(type,"bmp")==0 )
	{
		m_data=loadBitmapFile(filename,&bmp);
		m_width=bmp.biWidth;
		m_height=bmp.biHeight;
	}
	else if( _stricmp(type,"tga")==0 )
		m_data=loadTargaFile(filename);
	else return false;
	
	if( m_data )
	{
#ifdef _MSC_VER
		
		// set the way opengl interprets our m_data array
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// get an id to use
		glGenTextures(1,&m_glTextureObject);

		//bind texture
		glBindTexture(GL_TEXTURE_2D, m_glTextureObject);

		// repeat/norepeat
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);

		//filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_LINEAR);

		//load texture
		// THIS FAILS IF IMG DIM ARE NOT 2^k where k is the length
		gluBuild2DMipmaps(
			GL_TEXTURE_2D,
			GL_RGB,
			m_width,
			m_height,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			(unsigned char *)m_data);
		//glTexImage2D(
		//	GL_TEXTURE_2D,0,GL_RGB,
		//	m_width, m_height,
		//	0, GL_RGB, GL_UNSIGNED_BYTE, (unsigned char *)m_data);
	//	delete m_data;
	//	m_data=null;
#endif
		return true;
	} else return false;
}

////////////////////////////////////////////
// bitmap loading function				  //
//		-- ripped of from opengl game pro //
////////////////////////////////////////////

unsigned char *jvs3d_material::loadBitmapFile(const char *filename, jvs3d_image_bmp_infoheader *bitmapInfoHeader)
{
	FILE *filePtr;
	jvs3d_image_bmp_fileheader bitmapFileHeader;
	unsigned char *bitmapImage;
	unsigned int imageIdx=0;
	unsigned char tempRGB;
	
	// open filename in "read binary" mode
	filePtr=fopen(filename,"rb");
	if(!filePtr)
		return NULL;
		
	// read the bitmap file header
	fread(&bitmapFileHeader, sizeof(jvs3d_image_bmp_fileheader), 1, filePtr);
	
	// verify that this is a bitmap by checking for the universal bitmap id
	if(!(bitmapFileHeader.bfType & 0x00004d42))
	{
		fclose(filePtr);
		return NULL;
	}
	
	// read the bitmap information header
	fread(bitmapInfoHeader, sizeof(jvs3d_image_bmp_infoheader), 1, filePtr);
	
	if(bitmapInfoHeader->biCompression!=BI_RGB)
	{
		fclose(filePtr);
		return NULL;
	}

	// move file pointer to beginning of bitmap data
	fseek(filePtr,bitmapFileHeader.bOffBits, SEEK_SET);
	
	// allocate enough memory for the bitmap image data
	bitmapImage = (unsigned char *)malloc(bitmapInfoHeader->biSizeImage);
	
	// verify memory allocation
	if(!bitmapImage)
	{
		free(bitmapImage);
		fclose(filePtr);
		return NULL;
	}
	
	// read in the bitmap image data
	fread(bitmapImage, 1, bitmapInfoHeader->biSizeImage, filePtr);
	
	// make sure bitmap image data was read
	if(bitmapImage==NULL)
	{
		fclose(filePtr);
		return NULL;
	}
	
	// swap the R and B values to gett RGB since the bitmap color format is in BGR
	for(imageIdx=0;imageIdx<bitmapInfoHeader->biSizeImage;imageIdx+=3)
	{
		tempRGB = bitmapImage[imageIdx];
		bitmapImage[imageIdx]=bitmapImage[imageIdx+2];
		bitmapImage[imageIdx+2]=tempRGB;
	}
	
	// fclose the file and return tthe bitmtap image data
	fclose(filePtr);
	m_data=bitmapImage;

	printf("BMP image loaded\n");
	printf("File name    - \"%s\"\n",filename);
	printf("image size   - %u\n", bitmapInfoHeader->biSizeImage);
	printf("bitcount     - %u\n", bitmapInfoHeader->biBitCount);
	printf("color planes - %u\n", bitmapInfoHeader->biPlanes);
	printf("image width  - %u\n", bitmapInfoHeader->biWidth);
	printf("image height - %u\n", bitmapInfoHeader->biHeight);
	printf("\n");

	return m_data;
}

unsigned char * jvs3d_material::loadTargaFile(const char *filename)
{
	jvs3d_image_targa_file tgaFile;
	FILE * filePtr;
	unsigned char ucharBad;
	short int sintBad;
	long imageSize;
	int colorMode;
	long imageIdx;
	unsigned char colorSwap;

	filePtr=fopen(filename,"rb");
	if(!filePtr)
		return 0;

	fread(&ucharBad, sizeof(unsigned char), 1, filePtr);
	fread(&ucharBad, sizeof(unsigned char), 1, filePtr);

	fread(&tgaFile.imageTypeCode, sizeof(unsigned char),1,filePtr);

	if( (tgaFile.imageTypeCode!=2) && (tgaFile.imageTypeCode!=3) )
	{
		fclose(filePtr);
		return NULL;
	}

	fread(&sintBad,sizeof(short int),1,filePtr);
	fread(&sintBad,sizeof(short int),1,filePtr);

	fread(&ucharBad, sizeof(unsigned char), 1, filePtr);
	fread(&sintBad, sizeof(short int),1,filePtr);
	fread(&sintBad, sizeof(short int),1,filePtr);

	fread(&tgaFile.imageWidth, sizeof(short int), 1, filePtr);
	fread(&tgaFile.imageHeight, sizeof(short int), 1, filePtr);

	fread(&tgaFile.bitCount, sizeof(unsigned char),1,filePtr);

	fread(&ucharBad,sizeof(unsigned char),1,filePtr);

	colorMode=tgaFile.bitCount/8;
	imageSize=tgaFile.imageHeight * tgaFile.imageWidth * colorMode;

	tgaFile.imageData = (unsigned char *)malloc( sizeof(unsigned char) * imageSize );

	fread(tgaFile.imageData, sizeof(unsigned char), imageSize, filePtr);

	for(imageIdx=0;imageIdx<imageSize;imageIdx+=colorMode)
	{
		colorSwap = tgaFile.imageData[imageIdx];
		tgaFile.imageData[imageIdx]=tgaFile.imageData[imageIdx+2];
		tgaFile.imageData[imageIdx+2]=colorSwap;
	}

	m_data=tgaFile.imageData;
	m_width=tgaFile.imageWidth;
	m_height=tgaFile.imageHeight;

	printf("TGA image loaded\n");
	printf("File name    - \"%s\"\n",filename);
	printf("image size   - %u\n",imageSize);
	printf("bitcount     - %u\n",tgaFile.bitCount);
	printf("color mode   - %u\n", colorMode);
	printf("image width  - %u\n", tgaFile.imageWidth);
	printf("image height - %u\n", tgaFile.imageHeight);
	printf("\n");

	return m_data;
}