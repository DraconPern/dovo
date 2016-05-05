
#include "patientdata.h"
#include "sqlite3_exec_stmt.h"
#include <boost/locale.hpp>
#include <sstream>

PatientData::PatientData()
{
	db = NULL;
	createdb();
}

void PatientData::createdb()
{
	if(sqlite3_open_v2(":memory:", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL))
	{
		std::ostringstream msg;
		msg << "Can't create database: " << sqlite3_errmsg(db);
		throw std::runtime_error(msg.str().c_str());				
	}	
	
	sqlite3_exec(db, "CREATE TABLE patients (patid TEXT, patname TEXT, birthday TEXT, PRIMARY KEY (patid, patname))", NULL, NULL, NULL);
	sqlite3_exec(db, "CREATE TABLE studies (studyuid TEXT UNIQUE, patid TEXT, patname TEXT, studydesc TEXT, studydate TEXT)", NULL, NULL, NULL);
	sqlite3_exec(db, "CREATE TABLE series (seriesuid TEXT UNIQUE, studyuid TEXT, seriesdesc TEXT)", NULL, NULL, NULL);
	sqlite3_exec(db, "CREATE TABLE instances (sopuid TEXT UNIQUE, seriesuid TEXT, filename TEXT, sopclassuid TEXT, transfersyntax TEXT)", NULL, NULL, NULL);
}

PatientData::~PatientData()
{
	if(db)
		sqlite3_close(db);
}
void PatientData::Clear()
{
	if(db)
		sqlite3_close(db);

	createdb();
}

int PatientData::AddPatient(std::string patid, std::string patname, std::string birthday)
{
	sqlite3_stmt *insert;
	std::string sql = "INSERT INTO patients VALUES(?, ?, ?)";
	sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &insert, NULL);
	sqlite3_bind_text(insert, 1, patid.c_str(), patid.length(), SQLITE_STATIC);
	sqlite3_bind_text(insert, 2, patname.c_str(), patname.length(), SQLITE_STATIC);
	sqlite3_bind_text(insert, 3, birthday.c_str(), birthday.length(), SQLITE_STATIC);
	int res = sqlite3_exec_stmt(insert, NULL, NULL, NULL);
	sqlite3_finalize(insert);
	if(res == SQLITE_DONE || res == SQLITE_ABORT)
		return 1;
	else
		return 0;
}

int getpatientscallback(void *param,int columns,char** values, char**names)
{
	boost::function<int(Patient &)> pfn = * static_cast<boost::function<int(Patient &)> *>(param);	
	Patient result(values[0], values[1], values[2]);
	return pfn(result);	
}

void PatientData::GetPatients(boost::function<int(Patient &) > action)
{	
	std::string selectsql = "SELECT patid, patname, birthday FROM patients ORDER BY patid";
	sqlite3_stmt *select;
	sqlite3_prepare_v2(db, selectsql.c_str(), selectsql.length(), &select, NULL);
	sqlite3_exec_stmt(select, getpatientscallback, &action, NULL);
	sqlite3_finalize(select);
}

int PatientData::AddStudy(std::string studyuid, std::string patid, std::string patname, std::string studydesc, std::string studydate)
{
	sqlite3_stmt *insert;
	std::string sql = "INSERT INTO studies VALUES(?, ?, ?, ?, ?)";
	sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &insert, NULL);
	sqlite3_bind_text(insert, 1, studyuid.c_str(), studyuid.length(), SQLITE_STATIC);
	sqlite3_bind_text(insert, 2, patid.c_str(), patid.length(), SQLITE_STATIC);
	sqlite3_bind_text(insert, 3, patname.c_str(), patname.length(), SQLITE_STATIC);
	sqlite3_bind_text(insert, 4, studydesc.c_str(), studydesc.length(), SQLITE_STATIC);
	sqlite3_bind_text(insert, 5, studydate.c_str(), studydate.length(), SQLITE_STATIC);
	int res = sqlite3_exec_stmt(insert, NULL, NULL, NULL);
	sqlite3_finalize(insert);
	if(res == SQLITE_DONE || res == SQLITE_ABORT)
		return 1;
	else
		return 0;
}

int getstudiescallback(void *param,int columns,char** values, char**names)
{
	boost::function<int(Study &)> pfn = * static_cast<boost::function<int(Study &)> *>(param);
	Study result(values[0], values[1], values[2], values[3], values[4]);
	return pfn(result);
}

// void PatientData::GetStudies(std::vector<Study> &studies)
void PatientData::GetStudies(boost::function< int(Study &) > action)
{
	std::string selectsql = "SELECT studyuid, patid, patname, studydesc, studydate FROM studies ORDER BY studyuid ASC";
	sqlite3_stmt *select;
	sqlite3_prepare_v2(db, selectsql.c_str(), selectsql.length(), &select, NULL);	
	sqlite3_exec_stmt(select, getstudiescallback, &action, NULL);		
	sqlite3_finalize(select);
}

void PatientData::GetStudies(std::string patientid, std::string patientname, boost::function< int(Study &) > action)
{
	std::string selectsql = "SELECT studyuid, patid, patname, studydesc, studydate FROM studies WHERE (patid = ? AND patname = ?) ORDER BY studyuid ASC";
	sqlite3_stmt *select;
	sqlite3_prepare_v2(db, selectsql.c_str(), selectsql.length(), &select, NULL);
	sqlite3_bind_text(select, 1, patientid.c_str(), patientid.length(), SQLITE_STATIC);
	sqlite3_bind_text(select, 2, patientname.c_str(), patientname.length(), SQLITE_STATIC);
	sqlite3_exec_stmt(select, getstudiescallback, &action, NULL);		
	sqlite3_finalize(select);
}

int PatientData::AddSeries(std::string seriesuid, std::string studyuid, std::string seriesdesc)
{
	sqlite3_stmt *insert;
	std::string sql = "INSERT INTO series VALUES(?, ?, ?)";
	sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &insert, NULL);
	sqlite3_bind_text(insert, 1, seriesuid.c_str(), seriesuid.length(), SQLITE_STATIC);
	sqlite3_bind_text(insert, 2, studyuid.c_str(), studyuid.length(), SQLITE_STATIC);
	sqlite3_bind_text(insert, 3, seriesdesc.c_str(), seriesdesc.length(), SQLITE_STATIC);	
	int res = sqlite3_exec_stmt(insert, NULL, NULL, NULL);
	sqlite3_finalize(insert);
	if(res == SQLITE_DONE || res == SQLITE_ABORT)
		return 1;
	else
		return 0;
}

int getseriescallback(void *param,int columns,char** values, char**names)
{
	boost::function<int(Series &)> pfn = * static_cast<boost::function<int(Series &)> *>(param);	
	Series result(values[0], values[1], values[2]);
	return pfn(result);
}

void PatientData::GetSeries(std::string studyuid, boost::function< int(Series &) > action)
{
	std::string selectsql = "SELECT seriesuid, studyuid, seriesdesc FROM series WHERE (studyuid = ?) ORDER BY seriesuid ASC";
	sqlite3_stmt *select;
	sqlite3_prepare_v2(db, selectsql.c_str(), selectsql.length(), &select, NULL);
	sqlite3_bind_text(select, 1, studyuid.c_str(), studyuid.length(), SQLITE_STATIC);
	sqlite3_exec_stmt(select, getseriescallback, &action, NULL);		
	sqlite3_finalize(select);
}

int PatientData::AddInstance(std::string sopuid, std::string seriesuid, boost::filesystem::path filename, std::string sopclassuid, std::string transfersyntax)
{
	sqlite3_stmt *insert;
	std::string sql = "INSERT INTO instances VALUES(?, ?, ?, ?, ?)";
	sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &insert, NULL);
	sqlite3_bind_text(insert, 1, sopuid.c_str(), sopuid.length(), SQLITE_STATIC);
	sqlite3_bind_text(insert, 2, seriesuid.c_str(), seriesuid.length(), SQLITE_STATIC);
#ifdef _WIN32
	std::string p = boost::locale::conv::utf_to_utf<char>(filename.c_str());
#else
	std::string p = filename.string();
#endif
	sqlite3_bind_text(insert, 3, p.c_str(), p.length(), SQLITE_STATIC);	
	sqlite3_bind_text(insert, 4, sopclassuid.c_str(), sopclassuid.length(), SQLITE_STATIC);	
	sqlite3_bind_text(insert, 5, transfersyntax.c_str(), transfersyntax.length(), SQLITE_STATIC);	
	int res = sqlite3_exec_stmt(insert, NULL, NULL, NULL);
	sqlite3_finalize(insert);
	if(res == SQLITE_DONE || res == SQLITE_ABORT)
		return 1;
	else
		return 0;
}

int getinstancescallback(void *param,int columns,char** values, char**names)
{
	boost::function<int(Instance &)> pfn = * static_cast<boost::function<int(Instance &)> *>(param);
#ifdef _WIN32	
	Instance result(values[0], values[1], boost::locale::conv::utf_to_utf<boost::filesystem::path::value_type>(values[2]), values[3], values[4]);
#else
	Instance result(values[0], values[1], values[2], values[3], values[4]);
#endif
	return pfn(result);
}

//void PatientData::GetInstances(std::vector<Instance> &instances)
void PatientData::GetInstances(std::string seriesuid, boost::function< int(Instance &) > action)
{
	std::string selectsql = "SELECT sopuid, seriesuid, filename, sopclassuid, transfersyntax FROM instances WHERE (seriesuid = ?) ORDER BY sopuid ASC";
	sqlite3_stmt *select;
	sqlite3_prepare_v2(db, selectsql.c_str(), selectsql.length(), &select, NULL);
	sqlite3_bind_text(select, 1, seriesuid.c_str(), seriesuid.length(), SQLITE_STATIC);
	sqlite3_exec_stmt(select, getinstancescallback, &action, NULL);		
	sqlite3_finalize(select);
}


