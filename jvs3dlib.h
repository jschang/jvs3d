#ifndef __JVS3DLIB__
#define __JVS3DLIB__

#ifndef _MSC_VER
	#include <lcms.h>
	typedef long LONG;
	#define null 0
	#define NULL null
	#define BI_RGB 0
#else
//	#include <afxwin.h>
	#include <windows.h>
	#include <GL/gl.h> 			//OPENGL
	#include <GL/glu.h>			//OPENGL
	#include <GL/glaux.h>		//OPENGL
//	#include <GL/glext.h>			//OPENGL
#endif

// turn various debug outputs on and off
const bool DBGOBJLOAD	=false;
const bool DBGOBJDISP	=false;
const bool DBGVERT		=false;
const bool DBGVERT_MATH =false;
const bool DBGMATH		=false;
const bool DBGMAT		=false;
const bool DBGPOLY		=false;
const bool DBGFAC3D 	=false;
const bool DBGDEL		=false;
const bool DBGTERR_OL	=true;
const bool DBGTERR_ML	=false;
const bool DBGTERR_IL	=false;
const bool DBGTERR_VIS	=false;
const bool DBGTERR_TXTR	=false;

#define JVS_PI 3.1415926535897/*932384626433832795028841971f*/

typedef unsigned int uint;

class jvs3d_material;
class jvs3d_vertice;
class jvs3d_matrix;
class jvs3d_armature;
class jvs3d_object;
class jvs3d_polygon;
class jvs3d_item;

////////////////////////////////////
// JVS3D Image formats structures //
////////////////////////////////////

typedef struct jvs3d_image_targa_file_type
{
	unsigned char imageTypeCode;
	short int	imageWidth;
	short int	imageHeight;
	unsigned char bitCount;
	unsigned char *imageData;
} jvs3d_image_targa_file;

typedef struct jvs3d_image_bmp_fileheader_type
{
	WORD 	bfType;
	DWORD	bfSize;
	WORD	bfReserved1;
	WORD	bfReserved2;
	DWORD	bOffBits;
} jvs3d_image_bmp_fileheader;

typedef struct jvs3d_image_bmp_infoheader_type
{
	DWORD	biSize;
	LONG	biWidth;
	LONG	biHeight;
	WORD	biPlanes;
	WORD	biBitCount;
	DWORD	biCompression;
	DWORD	biSizeImage;
	LONG	biXPelsPerMeter;
	LONG	biYPelsPerMeter;
	DWORD	biClrUsed;
	DWORD	biClrImportant;
} jvs3d_image_bmp_infoheader;

///////////////////////////
// Encapsulate all below //
///////////////////////////

class jvs3d_item : public jvs_baseclass
{
public: // public properties
	jvs3d_item *m_parentItem;
	class jvs3d_object		*m_objects;		// pointer to constituent objects
	class jvs3d_material	*m_materials;	// pointer to list of materials
											//  ...may be the global list
	class jvs3d_cycle		*m_cycles;		// a list of cycles
	class jvs3d_armature	*m_armatures;	// a list of armatures
public: // public methods
	jvs3d_item();
	~jvs3d_item();
	char loadXMLFile(FILE *fp);
};
//typedef class jvs3d_item jvs3d_item_static;
//typedef class jvs3d_item jvs3d_item_animated;

//////////////////////////
// MATHEMATICAL CLASSES //
//////////////////////////

// just a collection of math functions...
// called using jvs3d_math::function() rather than ever
// actually constructing a math object
class jvs3d_math : public jvs_baseclass
{
public:
	static double RadiansToDegrees(double rad);
	static double DegreesToRadians(double deg);
	static double getDistance(double x1, double y1, double x2, double y2);
	static bool orderArray2dx4(int upORdown, int idx, int d1, int arr[][4]);
	// d1: size of the first dimension
	// d2: size of the seconf dimension
};

class jvs3d_vertice : public jvs_baseclass 
{
	public:
		double m_data[4];
		double m_temp[4];
		jvs3d_object *m_object;
		jvs3d_armature *m_armature;

		bool set(float x, float y, float z);
		bool setTemp(float x, float y, float z);
		bool set(jvs3d_vertice *v);
		bool setTemp(jvs3d_vertice *v);

		jvs3d_vertice();
		~jvs3d_vertice();

		void drawGL(jvs3d_object *pobj);

	// eh, some mathematical type functions
		double dotcos(jvs3d_vertice a);
		double length(void);
		void subtract(jvs3d_vertice a);
		void add(jvs3d_vertice a);
		void scale(double s);
		static void swap(jvs3d_vertice &one, jvs3d_vertice &two);
		void operator*=(jvs3d_matrix m);
		void operator*=(double s){scale(s);};
		void operator+=(jvs3d_vertice a){add(a);};
		void operator-=(jvs3d_vertice a){subtract(a);};
		void backup(void) { for(int i=0;i<3;i++){m_temp[i]=m_data[i];}};
		void restore(void) { for(int i=0;i<3;i++){m_data[i]=m_temp[i];}};
		void crossProduct(jvs3d_vertice one, jvs3d_vertice two, jvs3d_vertice three);
		void calculateNormal(jvs3d_vertice one, jvs3d_vertice two, jvs3d_vertice three);
		//void normalize(void);
		void normalize(void);
		static int greatestPoint(jvs3d_vertice verts[],int numVertices,int axis);
		static int leastPoint(jvs3d_vertice verts[],int numVertices,int axis);
	public: // file io
		char loadXMLFile(FILE *fp);
		bool storeXMLFileList(FILE *fp);
		bool storeXMLFile(FILE *fp);

//void operator=(jvs3d_vertice *a);
};

#define JVS3D_MATRIX_IDENT	43981
#define JVS3D_MATRIX_ROT	43982
#define JVS3D_MATRIX_TRANS	43983
#define JVS3D_MATRIX_SCALE	43984
class jvs3d_matrix : public jvs_baseclass
{
public:
	double m_data[4][4];	// m_data[r][c]public:
public:
	jvs3d_matrix();
	~jvs3d_matrix();
	void operator*=(jvs3d_matrix a);
	void operator*=(double s);
	void multiply(jvs3d_matrix a);

	bool initialize(unsigned int type, void *inp);
};

///////////////////////////////
// MOST ESSENTIAL OF CLASSES //
///////////////////////////////

#define JVS3D_MAT_COLOR		34508
#define JVS3D_MAT_TEXTURE	34509
class jvs3d_material : public jvs_baseclass
{
public: // attributes
	unsigned int m_materialType;	// material type; texture, flat color, 
									//  phong, goraud, etc...per jvs3dlib.h
	unsigned int m_glTextureObject;	// gl texture object

	float m_color[3];			// color intensity 0-1, r,g,b

	char *m_filepath;			// absolute path to image
	unsigned int m_imageType;	// image type per jvs3dlib.h

	unsigned char *m_data;		// loaded image bits
	unsigned int m_width;
	unsigned int m_height;
public: // operations
	jvs3d_material();
	~jvs3d_material();
	bool getPixel(char *vals, int x, int y);
	bool initializeGL(const char type[3], const char *filename);
	bool bindGL(void);
	unsigned char * loadBitmapFile(const char *filename, 
		jvs3d_image_bmp_infoheader *bitmapInfoHeader);
	unsigned char * loadTargaFile(const char *filename);
public: // file io
	char loadXMLFile(FILE *fp);
	char loadXMLFileList(FILE *fp);
	bool storeXMLFile(FILE *fp);
	bool storeXMLFileList(FILE *fp);
};

class jvs3d_polygon	: public jvs_baseclass 
{
public:
	uint *m_index;	// list of indexes associated with polygon; init to NULL
	double *m_txtrX;
	double *m_txtrY;
	uint m_num;		// number of indexes
	
	jvs3d_material *m_materialPtr;

	unsigned long m_polyFace;
	unsigned long m_polyMode;
	unsigned long m_drawMode;

	float m_colorR;
	float m_colorG;
	float m_colorB;

	unsigned long m_drawmode;
public:
	jvs3d_polygon();
	~jvs3d_polygon();

	bool drawGL(jvs3d_object *pobj, jvs3d_vertice *vlist);
	void polygonMode(unsigned long face, unsigned long mode);
	void drawMode(unsigned long mode);
	double * getLenAng(jvs3d_vertice &verts);
	bool prepTextureCoords(jvs3d_vertice &verts);
	void textureTranslate(double x, double y);
	void textureScale(double x, double y);
	void textureRotate(double theta);
	void textureCenter(double *x, double *y);

public: // file io
	char loadXMLFileList(FILE *fp);
	char loadXMLFile(FILE *fp);
	bool storeXMLFile(FILE *fp);
	bool storeXMLFileList(FILE *fp);

};
class jvs3d_object : public jvs_baseclass
{

public:
	jvs3d_vertice m_translate;	// translate by this vertice
	jvs3d_vertice m_rotate;		// rotate by this vertice
	jvs3d_vertice *mp_vertices;	// the linked list of vertices
	jvs3d_polygon *mp_polygons;	// the linked list of polygons

	unsigned long m_polyFace;
	unsigned long m_polyMode;
	unsigned long m_drawMode;

public:
	jvs3d_object();
	~jvs3d_object();	

public: // display
	bool drawGL(void);
	bool drawGL(unsigned long mode, unsigned long face);
	void polygonMode(unsigned long face, unsigned long mode);
	void drawMode(unsigned long mode);
	bool applyFrame(class jvs3d_frame *frame, class jvs3d_armature *exclude);

public: // internal adjustment
	bool newCenter(void);
	
public:	// creation
	bool addVertice(float x, float y, float z);
	bool addPolygon(float txo, float tyo, int length, uint *idx_list);
	
public: // status
	unsigned int verticeCount(void);
	
public: // odbc and serialization
	bool loadXFile(const char *pathfile);
	bool storeXFile(const char *pathfile);
	bool loadAC3DFile(const char *pathfile);
	bool storeAC3DFile(const char *pathfile);
	char loadXMLFileList(FILE *fp);
	char loadXMLFile(FILE *fp);
	bool storeXMLFileList(FILE *fp);
	bool storeXMLFile(FILE *fp);
};

///////////////////////////////////////////////
// SOME OPENGL FUNCTIONALLITY TO DERIVE FROM //
///////////////////////////////////////////////

class jvs3d_opengl : public jvs_baseclass
{
protected: // properties
//	PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2fARB;
//	PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB;
//	PFNGLCLIENTACTIVETEXTUREARBPROC	glClientActiveTextureARB;

private:
	// OPENGL multitexturing stuff
	bool m_bMultiTexture;
	
protected: // methods
	void initializeMultiTextureSupport(void);
	bool multiTextureSupport(void) { return m_bMultiTexture; };
};

///////////////////////////////
// ANIMATION SUPPORT CLASSES //
///////////////////////////////

class jvs3d_armature : public jvs_baseclass
{
public:
	class jvs3d_vertice	m_pivot;		// point about which armature pivots
	class jvs3d_vertice	m_pivotXForm;	// pivot point after transformations
	class jvs3d_matrix	m_matrix;		// matrix to accumulate calculations 
	//jvs_baseclass *	
	//	m_vertices;			// currently unused, which armature a vertice 
							//  belongs to is stored in the vertice 
							//  class
	class jvs_baseclass * 
		m_armatures;		// child armatures, so that matrix's can be 
							//  pulled from where we are
	class jvs3d_armature *
		m_parentArmature;	// parent to this armature
	char * m_parentName;

	// when generating a frame,
	// first transform the top level armature matrix,
	// and cascade that to each successive child
	// then cycle through vertices, performing the multiplication
	// then only once per vertice using the matrix in it's armature

public:
	jvs3d_armature();
	~jvs3d_armature();

	/*bool addVertice(jvs3d_vertice *v);
	bool remVertice(jvs3d_vertice *v);
	jvs_baseclass * hasVertice(jvs3d_vertice *v);*/

	bool addArmature(jvs3d_armature *a);
	bool remArmature(jvs3d_armature *a);
	jvs_baseclass * hasArmature(jvs3d_armature *a);
	bool setupMatrices(class jvs3d_armupdate *udates,
		class jvs3d_armature *parent);
	bool clearMatrices(void);

public: // file io
	char loadXMLFile(FILE *fp);
	char loadXMLFileList(FILE *fp);
	bool storeXMLFile(FILE *fp);
	bool storeXMLFileList(FILE *fp);

};

class jvs3d_armupdate : public jvs_baseclass
{
public: // armature data members
	jvs3d_armature *m_armature;
	double m_rotation[3];

public: // armature methods
	jvs3d_armupdate();
	jvs3d_armupdate* findArmature(jvs3d_armature *arm);

public: // file io
	char loadXMLFileList(FILE *fp);
	char loadXMLFile(FILE *fp);
	bool storeXMLFile(FILE *fp);
	bool storeXMLFileList(FILE *fp);
};

class jvs3d_frame : public jvs_baseclass
{
public: // frame public data members
	class jvs3d_armupdate *m_armUpdates;
	class jvs3d_cycle *m_parentCycle;
	double m_translation[3];
	double m_rotation[3];

public: // frame public methods
	jvs3d_frame();
	~jvs3d_frame();

public: // file io
	char loadXMLFileList(FILE *fp);
	char loadXMLFile(FILE *fp);
	bool storeXMLFile(FILE *fp);
	bool storeXMLFileList(FILE *fp);

};

class jvs3d_cycle : public jvs_baseclass
{
public: // data members
	jvs3d_frame *m_frames;
	jvs3d_frame *m_curFrame;

public:
	jvs3d_cycle();
	~jvs3d_cycle();

public: // file io
	char loadXMLFile(FILE *fp);
	char loadXMLFileList(FILE *fp);
	bool storeXMLFile(FILE *fp);
	bool storeXMLFileList(FILE *fp);
};

////////////////////////////////
// JVS3D File Parsing classes //
////////////////////////////////

#define JVS3D_FILE_AC3D_MATERIAL 123
#define JVS3D_FILE_AC3D_OBJECT 124
class jvs3d_file_ac3d : public jvs_textparser
{
public:
	jvs3d_object *m_obj;
public:
	jvs3d_file_ac3d(const char *filename, jvs3d_object *obj)
		{if(openFile(filename)) m_obj=obj;};
	jvs3d_file_ac3d(jvs3d_object *obj){m_obj=obj;};
	jvs3d_file_ac3d(){m_obj=null;};
	bool loadFile(const char *pathfile, class jvs3d_object *obj);
	int returnType(const char *cmd);
	int loadMaterial(void);
	int loadObject(void);
};

class jvs3d_file_x : public jvs_textparser
{
public:
	char file[256];
public:
	bool readList(int elem_per);
	bool setFile(const char *fullpath);
	bool verifyHeader();
	int getNumberOfMeshes();
	bool getMeshName(int index, char *receptor);
	int	getMeshNumberOfVertices(int index);
	int getMeshNumberOfFaces(int index);
};

///////////////////
// TERRAIN CLASS //
///////////////////
class jvs3d_terrain_texture_mapping : public jvs3d_material
{
public:
	jvs3d_terrain_texture_mapping();
	~jvs3d_terrain_texture_mapping();
	void getTextureIds(
		unsigned long &red, 
		unsigned long &green, 
		unsigned long &blue, 
		unsigned long &alpha);
	void setTextureIds(
		unsigned long red, 
		unsigned long green, 
		unsigned long blue, 
		unsigned long alpha);
	unsigned long getRed(void);
	unsigned long getGreen(void);
	unsigned long getBlue(void);
	unsigned long getAlpha(void);
	unsigned long getTextureId(int idx);
private:
	unsigned long m_textureIds[4];
};

class jvs3d_camera;
class jvs3d_terrain : public jvs3d_opengl
{
public:
	jvs3d_terrain();
	jvs3d_terrain(const char *fileName);
	~jvs3d_terrain();

	// initialization
	bool	loadHeightMap(const char *fileName);
	bool	loadTextureMapping(const char *fileName, unsigned long *pks);

	// rendering functions
	bool	calculateVertices(void);

	// bool
	bool textureMappingLoaded(void) { return m_pTextureMappings?1:0; }

	// accessors
	void *  getTextureIds(int x, int z, int &num);
	bool	mapLoaded(void){return m_boolMapLoaded;}
	double	getHeightAt(double x, double z);
	int		getWidth() { return m_boolMapLoaded?m_heightMap.m_width:(-1); }
	int		getHeight() { return m_boolMapLoaded?m_heightMap.m_height:(-1); }
	bool	setCamera(jvs3d_camera *pcam);
	bool	setTranslate(jvs3d_vertice &pos);
	bool	setTranslate(double x, double y, double z);
	bool	setMaterials(jvs3d_material *pMaterials);
	jvs3d_camera * getCamera(void);
	
	// pixel space to world coordinate wrappers
	bool getPosition(jvs3d_vertice &pos, int x, int y);		
		// convert known map x,y to world coordinates
	bool getPosition(int *x, int *y, jvs3d_vertice m_pos);	
		// convert known world coordinates to map x, y
	bool getPosition(jvs3d_vertice &pos);
	unsigned long getTextureId(int x, int z);

	// coordinate transformations; PC->WC
	bool	transform(jvs3d_vertice &t);
	bool	untransform(jvs3d_vertice &t);

	// debug functions
	bool	drawGrid(void);
	bool	drawAllTerrain(void);

private: // data
	jvs3d_camera	*m_pCamera;			// camera object associated with terrain
	jvs3d_object	m_object;			// currently to be rendered section
	double			*m_pTextureCoords;	// an ptr array of x scanlines of texture coords
	jvs3d_material	*m_pMaterials;		// a linked list of textures

	// properties effected coordinate transformation
	jvs3d_vertice	m_translate;	// OC: translation from pixel space to world space
	jvs3d_vertice	m_position;		// WC: position
	double			m_scale;		// scale up/down from pixel space to world space
	double			m_scaleHeightCorrect;
									// scaler to apply to each height(y) element of the map

	jvs3d_material	m_heightMap;	// material holding the texture of the 
									//   height map

	// need to move m_textureMap to use this instead,
	// each coord would cycle through the members of this
	// getting the texture_ids and their associated levels
	// then determining a percentage blend for each texture_id
	// appearing at the coordinate
	jvs3d_terrain_texture_mapping *m_pTextureMappings;	

	bool m_boolMapLoaded;			// a map is loaded true | false
	bool m_boolTextureMapLoaded;	// a texture mapping is loaded for the terrain
	bool m_boolDrawGL;				// draw as we calculate or store 
	double			m_blurRadius;	// radius (in world coordinates) at 
									//   which to begin render reduction
									//   in the temp object

private: // methods
	void setInitialValues(void);
	void determineAndSetupMultiTextureSupport(void);
	bool calculateTextureCoords(void);	// to be run in the tail end of loadHeightMap
	bool getTextureCoord(double &tX, double &tY, int x, int y);
	bool setTextureCoord(double tX, double tY, int x, int y);
	void drawPatch(int hmPos[4][2]);
	void calcXY(double slope, int alignOn, int ptX, int ptY, int x, int &y);
};

//////////////////
// CAMERA CLASS //
//////////////////

class jvs3d_camera : public jvs_baseclass
{
public:
	jvs3d_camera();
	~jvs3d_camera();

	// accessors
	bool getLookAt(jvs3d_vertice &lookAt);
	bool updateLookAt(double pitch, double yaw, double roll);
	bool updateLookAt(jvs3d_vertice &pos);
	double getFrustumEnd(int);
	bool setFrustumEnd(int, double);
	double getFOVDegrees(void);
	double getFOVRadians(void);
	bool setFOVDegrees(double);
	bool setFOVRadians(double);
	double getFollowDistance(void);
	void setFollowDistance(double);
	bool getPosition(jvs3d_vertice &pos);
	bool setPosition(jvs3d_vertice pos);
	bool getRotation(jvs3d_vertice &pos);
	void setVertical(double y) {m_position.m_data[1]=y;};

	// fundamental operations
	bool rotate(char dir, jvs3d_vertice &pos);		// coord conv component; dir -1 for rev, 1 to pos
	bool translate(char dir, jvs3d_vertice &pos);	// coord conv component; dir -1 for rev, 1 to pos

	bool transform(jvs3d_vertice &pos);		// WC->CC
	bool untransform(jvs3d_vertice &pos);	// CC->WC

	// debug operations
	bool drawAxis(void);

	// world position modifiers
	bool	forward(double units);
	bool	backward(double units);

private:
	double m_FOVRadians;
	double m_FOVDegrees;
	double m_followDistance;		// distance to follow target pos from
	double m_frustumEnd[3];			// as far as the eye can see =) , high, medium, low

	// defines transform
	jvs3d_vertice m_position;		// WC: the position of the camera
	jvs3d_vertice m_lastPosition;	// WC: the last position of the camera
	jvs3d_vertice m_rotation;		// WC: rotation along each axis in degrees

	// current direction we're looking at from position
	jvs3d_vertice m_lookAt;			// CC: the position of the object to track
	jvs3d_vertice m_lastLookAt;		// CC: the last point the camera looked at
};

#endif // __JVS3DLIB__