#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <memory.h>

#include "jvslib.h"

//////////////////////////////
// Construction/Destruction //
//////////////////////////////

jvs_baseclass::jvs_baseclass()
{
	ms_name=null;
	mul_class_id=0;
	mul_type_id=0;
	mul_db_pk_id=0;
	mp_jvs_baseclass_top=null;
	mp_jvs_baseclass_next=null;
	mp_jvs_baseclass_prev=null;
	ul_jvs_type_id=JVS_CT_BASECLASS;
	ul_jvs_errnum=null;
	s_jvs_error=null;
	m_ptr=null;
	m_loaded=false;
}

jvs_baseclass::~jvs_baseclass()
{
	// remove member from current list so as to preserve it's integrity
	removeMember(this);
}

//////////////////////////////////////
// Functions to handle linked lists //
//////////////////////////////////////

void * jvs_baseclass::getAt(int index)
{
	jvs_baseclass *cur=null;;
	if(index>=0)
	{
		cur=(jvs_baseclass*)getTop();
		if(!cur) return null;
		while(index && cur)
		{
			cur=(jvs_baseclass*)cur->getNext();
			index--;
		}
	}
	else
	{
		cur=(jvs_baseclass*)getEnd();
		if(!cur) return null;
		while(index<0 && cur)
		{
			cur=(jvs_baseclass*)cur->getPrev();
			index++;
		}
	}
	return cur;
}

void * jvs_baseclass::getByPKId(unsigned long id)
{
	jvs_baseclass *cur=(jvs_baseclass*)getTop();
	while(cur)
	{
		if(cur->mul_db_pk_id==id)
			return cur;	
		cur=(jvs_baseclass*)cur->getNext();
	}
	return false;
}

void * jvs_baseclass::getByName(const char *name)
{
	jvs_baseclass *cur=(jvs_baseclass*)getTop();
	if(!name) return false;
	while(cur)
	{
		if(cur->ms_name)
		{
			if( strlen(name) && strcmp(name,cur->ms_name)==0 )
				return cur;	
		}
		cur=(jvs_baseclass*)cur->getNext();
	}
	return false;
}

void * jvs_baseclass::getTop(void)
// top will either be this or the beginning of the list
{
	jvs_baseclass *cur=null;
	cur=this;
	while(cur->mp_jvs_baseclass_prev)
		cur=(jvs_baseclass*)cur->getPrev();
	if(DBGBASE) printf("jvs_baseclass::getTop() returning %u\n");
	return cur;
}

void * jvs_baseclass::getEnd(void)
// end will either be this or the end of the list
{
	jvs_baseclass *cur=this;
	while(cur->mp_jvs_baseclass_next)
	{
		if(DBGBASE) printf("%u ",cur);
		cur=(jvs_baseclass*)cur->getNext();
	} 
	if(DBGBASE) printf("\njvs_baseclass::getEnd() returning %u\n",cur);
	return cur;
}

void * jvs_baseclass::getNext(void)
{return mp_jvs_baseclass_next;}

void * jvs_baseclass::getPrev(void)
{return mp_jvs_baseclass_prev;}

bool jvs_baseclass::setNext(jvs_baseclass *next)
{mp_jvs_baseclass_next=next;return true;}

bool jvs_baseclass::setPrev(jvs_baseclass *prev)
{mp_jvs_baseclass_prev=prev;return true;}

bool jvs_baseclass::replaceTop(jvs_baseclass *totop)
{
	if(!totop) return false;

	jvs_baseclass *top=(jvs_baseclass*)getTop();
	jvs_baseclass *temp1=null;
	jvs_baseclass *temp2=null;

	// if it's something in the list, adjust it's neighbors
	if((jvs_baseclass*)totop->getTop() == top)
	{
		temp1=(jvs_baseclass*)totop->getNext();
		temp2=(jvs_baseclass*)totop->getPrev();

		if(temp1 && temp2)
		{
			temp1->setPrev(temp2);
			temp2->setNext(temp1);
		}
	}

	top->setPrev(totop);
	totop->setNext(top);
	totop->setPrev(null);

	return true;
}

bool jvs_baseclass::appendEnd(jvs_baseclass *toend)
{
	if(!toend) return false;
	if(!(*this)[toend])
	{
		jvs_baseclass * end=(jvs_baseclass*)getEnd();
		end->setNext(toend);
		toend->setPrev(end);
		toend->setNext(null);
	}
	return true;
}

jvs_baseclass * jvs_baseclass::removeMember(jvs_baseclass *takeout)
{
	if(!takeout) return false;
	jvs_baseclass *next=(jvs_baseclass*)takeout->getNext();
	jvs_baseclass *prev=(jvs_baseclass*)takeout->getPrev();
	if(next) next->setPrev(prev);
	if(prev) prev->setNext(next);
	takeout->setNext(null);
	takeout->setPrev(null);
	if(next) return (jvs_baseclass*)next->getTop();
	else if(prev) return (jvs_baseclass*)prev->getTop();
	else return null;
}

bool jvs_baseclass::killMember(jvs_baseclass *killthis)
{
	if(!killthis) return false;
	removeMember(killthis);
	delete killthis;
	return true;
}

unsigned int jvs_baseclass::countList(void)
{
	jvs_baseclass *ptr=(jvs_baseclass*)this->getTop();
	int i=0;

	if(ptr) do 
	{
		ptr=(jvs_baseclass*)ptr->getNext();
		i++;
	} while(ptr);

	return i;		
}

//////////////////////////////
// Identification functions //
//////////////////////////////

void jvs_baseclass::setClassId(unsigned int class_id)	{mul_class_id=class_id;}
void jvs_baseclass::setTypeId(unsigned int type_id)		{mul_type_id=type_id;}
void jvs_baseclass::setPKId(unsigned long pk_id)		{mul_db_pk_id=pk_id;}
void jvs_baseclass::setName(const char *name)
{
	int len=strlen(name);

	if(ms_name) delete ms_name;
	ms_name = new char[len+1];

	memcpy(ms_name,name,len+1);
}
unsigned long jvs_baseclass::getClassId(void)	{return mul_class_id;}
unsigned long jvs_baseclass::getTypeId(void)	{return mul_type_id;}
unsigned long jvs_baseclass::getPKId(void)		{return mul_db_pk_id;}
const char *jvs_baseclass::getName(void)		{return ms_name;}

//////////////////////////////
// Error handling functions //
//////////////////////////////

char * jvs_baseclass::Error()			{return s_jvs_error;} 
unsigned long jvs_baseclass::errNum()	{return ul_jvs_errnum;}

bool jvs_baseclass::errorReset()
{
	if(s_jvs_error)
		free(s_jvs_error);
	ul_jvs_errnum=0;
	return true;
}

bool jvs_baseclass::errorSet(unsigned long errnum, const char *str)
{
	if(!errorReset()) return false;
	ul_jvs_errnum=errnum;
	s_jvs_error=(char *)malloc(sizeof(str));
	if(!strcpy(s_jvs_error,str)) return false;
	return true;
}


