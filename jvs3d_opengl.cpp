#ifndef _MSC_VER
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

void jvs3d_opengl::initializeMultiTextureSupport(void)
{
	char *ext = (char*)glGetString( GL_EXTENSIONS );

	if( ext && strstr( ext, "GL_ARB_multitexture" ) == NULL )
	{
		m_bMultiTexture=false;
	}
	else
	{
//		glActiveTextureARB       = (PFNGLCLIENTACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
//		glMultiTexCoord2fARB     = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");
//		glClientActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glClientActiveTextureARB");
		m_bMultiTexture=true;
//		if( !glActiveTextureARB || !glMultiTexCoord2fARB || !glClientActiveTextureARB )
//		{
			m_bMultiTexture=false;
//		}
	}
	printf("MultiTexturing Support: %s\n",m_bMultiTexture?"true":"false");
}