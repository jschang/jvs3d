#ifndef __JVSLIB_BASECLASS__
#define __JVSLIB_BASECLASS__

#include "jvslibtypes.h"

#define null NULL
const bool DEBUG	=false;
const bool DBGBASE	=false;

class jvs_baseclass
{
public: // properties
	void *m_ptr;
	bool m_loaded;
public: // methods

	// construction/destruction
	jvs_baseclass();	
	~jvs_baseclass();

	// linked list support
	void * getTop(void);
	void * getNext(void);
	void * getPrev(void);
	void * getEnd(void);

	void * getAt(int index);
	void * getByName(const char *name);
	void * getByPKId(unsigned long id);

	void * operator[](int index){return getAt(index);};
	void * operator[](const char *name){return getByName(name);};
	void * operator[](jvs_baseclass *ptr)
	{
		jvs_baseclass *thisPtr=(jvs_baseclass*)getTop();
		while(thisPtr)
		{
			if(thisPtr==ptr)
				return ptr;
			thisPtr=(jvs_baseclass*)thisPtr->getNext();
		}
		return null;
	}

	bool replaceTop(jvs_baseclass *totop);
	bool appendEnd(jvs_baseclass *toend);
	jvs_baseclass * removeMember(jvs_baseclass *takeout);
	bool killMember(jvs_baseclass *killthis);
	unsigned int countList(void);

	// identification support
	void setClassId(unsigned int class_id);
	void setTypeId(unsigned int type_id);
	void setPKId(unsigned long pk_id);
	void setName(const char *name);
	unsigned long getClassId(void);
	unsigned long getTypeId(void);
	unsigned long getPKId(void);
	const char *getName(void);

	// error handling
	char * Error();			// return a pointer to the current error
	unsigned long errNum();	// return the type of the last recorded error
	bool errorReset();
	bool errorSet(unsigned long errnum, const char *str);


protected:	// data members; so we can set this easily in derived classes

	unsigned long ul_jvs_type_id;	// may be a jvslib class, 
	unsigned long ul_jvs_errnum;	// a ulong to contain error number
	char *s_jvs_error;				// a string to contain error messages

private:	// data members
	jvs_baseclass *mp_jvs_baseclass_top;
	jvs_baseclass *mp_jvs_baseclass_next;
	jvs_baseclass *mp_jvs_baseclass_prev;	// use this?  or not?
	char ms_jvs_baseclass_key[80];

	unsigned long mul_class_id;		// class type id
	unsigned long mul_type_id;		// the type id of the object/vector/etc...so 
									// for some loops, you know
	unsigned long mul_db_pk_id;		// for members that have a primary key in the db
	char *ms_name;					// give it a name

private:
	// most basic functions
	bool setNext(jvs_baseclass *next);
	bool setPrev(jvs_baseclass *prev);
};

#endif // __JVSLIB_BASECLASS__

#ifndef __JVSLIB_TEXTPARSER__
#define __JVSLIB_TEXTPARSER__

// a text parsing class intended to be inherited from
// really just handles ... uhm ... stuff

#define JVSTP_NUM	23455101
#define JVSTP_STR	23455102
#define JVSTP_OPR	23455103

class jvs_textparser : public jvs_baseclass
{
public: // data members
	char s_jvs_cur[256];				// current segment we're working on
	double d_jvs_cur;					// the number returned if it were a number
	unsigned long ul_jvs_line_number;	// current line number we're working on
	FILE *file_jvs_tp_fd;				// text file descriptor; deriving classes shouldn't 
										// need this if i program this correctly.

public:
	jvs_textparser();
	~jvs_textparser();
	static char * getNextTag(FILE *fp);
	static char * getCDATATag(FILE *fp);
	static char * getTagContent(FILE *fp);
	bool openFile(const char *file);
	bool closeFile();
	bool isEOF();
	unsigned long readNext();
	static bool inStr(const char *needle, const char *haystack);

	double convertToNumber(const char *str);
	bool isNumeric(const char *str);
	bool isAlphaChar(char c);
	unsigned long isOperator(char c);
	int trimStrLen(const char *str);
};

#endif // __JVSLIB_TEXTPARSER__

#ifndef __JVSLIB_QUERY__	// only define once
#ifdef MYSQL_VERSION_ID		// only if the mysql header has been included
#define __JVSLIB_QUERY__

class jvs_query : public jvs_baseclass
{
	public:
		MYSQL 			*m_mysql;
		MYSQL_RES		*m_result;
		MYSQL_FIELD		*m_fields;
		MYSQL_ROW		m_cur_row;
		unsigned long 	*m_lengths;
		unsigned int	m_num_rows;
		unsigned int	m_num_fields;
		unsigned int	m_error;
		char 			*m_errorMessage;
	public:
		jvs_query(MYSQL* mysql, const char *sql);
		~jvs_query();
		
		unsigned int runQuery(const char *sql);
		
		int getColumnIndex(const char *colname);
		int getColumnLength(const char *colname);
		void *getColumnData(const char *colname);
				
		MYSQL_ROW getRow(unsigned int row);
		MYSQL_ROW getRow(void);
};

#endif
#endif // __JVSLIB_QUERY__