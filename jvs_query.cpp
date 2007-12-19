#ifdef __JVSLIB_QUERY__

#include <iostream>

#ifndef _MSC_VER
	#include <lcms.h>
	#include <mysql/mysql.h>
#endif

#include "jvslib.h"

jvs_query::jvs_query(MYSQL* mysql, const char *sql)
{
	m_errorMessage=null;
	m_error=0;
	m_num_fields=0;
	m_num_rows=0;
	m_fields=null;
	m_result=null;
	m_mysql=null;
	m_cur_row=null;

	if(mysql)
	{
		m_mysql=(MYSQL*)mysql;
		if(!runQuery(sql))
		{
			m_result = mysql_store_result(m_mysql);
			if(m_result)
			{
				m_num_fields	= mysql_num_fields(m_result);
				m_num_rows 		= mysql_num_rows(m_result);
				m_fields 		= mysql_fetch_fields(m_result);
				m_lengths 		= mysql_fetch_lengths(m_result);
			} else
			{
				if(mysql_field_count(mysql)==0)
					m_num_rows=mysql_affected_rows(mysql);
				else
				{
					m_error=mysql_errno(mysql);
					m_errorMessage=(char*)mysql_error(mysql);
				}
			}
		} else
		{
			m_error=mysql_errno(mysql);
			m_errorMessage=(char*)mysql_error(mysql);
		}
	} else m_mysql=null;
}

jvs_query::~jvs_query()
{
	if(m_result)
		mysql_free_result(m_result);
}

unsigned int jvs_query::runQuery(const char *sql)
{
	if(m_mysql)
		return mysql_real_query(m_mysql,sql,strlen(sql));
	else return false;
}

MYSQL_ROW jvs_query::getRow(unsigned int row)
{
	mysql_data_seek(m_result,row);
	return m_cur_row = mysql_fetch_row(m_result);
}

MYSQL_ROW jvs_query::getRow(void)
{
	return m_cur_row=mysql_fetch_row(m_result);
}

void *jvs_query::getColumnData(const char *colname)
{
	int idx=-1;
	if(m_cur_row)
	{
		if(idx=getColumnIndex(colname) > -1)
			return m_cur_row[idx];
		else return null;
	} else return null;
}

int jvs_query::getColumnLength(const char *colname)
{
	int idx=-1;
	if(idx=getColumnIndex(colname) > -1)
		return m_lengths[idx];
	else return -1;
}

int jvs_query::getColumnIndex(const char *colname)
{
	if(!m_num_fields) return (-1);
	for(int i=0; i<m_num_fields; i++)
		if(stricmp(colname,m_fields[i].name)==0)
			return i;
	return (-1);
}

#endif