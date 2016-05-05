#ifndef _PATIENTDATA_
#define _PATIENTDATA_

#include "sqlite3.h"
#include <vector>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>

class Instance
{
public:
	Instance(std::string sopuid, std::string seriesuid, boost::filesystem::path filename, std::string sopclassuid, std::string transfersyntax)
		: sopuid(sopuid), seriesuid(seriesuid), filename(filename), sopclassuid(sopclassuid), transfersyntax(transfersyntax) {}
	std::string sopuid;
	std::string seriesuid;
	boost::filesystem::path filename;
	std::string sopclassuid;
	std::string transfersyntax;
};

class Series
{
public:
	Series(std::string seriesuid, std::string studyuid, std::string seriesdesc)
		: seriesuid(seriesuid), studyuid(studyuid), seriesdesc(seriesdesc) {}
	std::string seriesuid;
	std::string studyuid;
	std::string seriesdesc;
};

class Study
{
public:
	Study(std::string studyuid, std::string patid, std::string patname, std::string studydesc, std::string studydate)
		: studyuid(studyuid), patid(patid), patname(patname), studydesc(studydesc), studydate(studydate) {}
	std::string studyuid;
	std::string patid;
	std::string patname;
	std::string studydesc;
	std::string studydate;
};

class Patient
{
public:
	Patient(std::string patid, std::string name, std::string birthday)
		: patid(patid), name(name), birthday(birthday) {};
	std::string patid;
	std::string name;
	std::string birthday;
};

class PatientData
{
public:

	PatientData();
	~PatientData();
	int AddPatient(std::string patid, std::string patname, std::string birthday);
	void GetPatients(boost::function< int(Patient &) > action);
	// void GetPatients(std::vector<Patient> &patients);
	int AddStudy(std::string studyuid, std::string patid, std::string patname, std::string studydesc, std::string studydate);
	void GetStudies(boost::function< int(Study &) > action);
	void GetStudies(std::string patientid, std::string patientname, boost::function< int(Study &) > action);
	// void GetStudies(std::vector<Study> &studies);
	int AddSeries(std::string seriesuid, std::string studyuid, std::string seriesdesc);	
	void GetSeries(std::string studyuid, boost::function< int(Series &) > action);
	// void GetSeries(std::vector<Series> &series);
	int AddInstance(std::string sopuid, std::string seriesuid, boost::filesystem::path filename, std::string sopclassuid, std::string transfersyntax);
	void GetInstances(std::string seriesuid, boost::function< int(Instance &) > action);
	// void GetInstances(std::vector<Instance> &instances);
	void Clear();

protected:

	void createdb();

	sqlite3 *db;
};


#endif