
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/config.h>
#include <wx/log.h>

#include "engine.h"
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_escape_char.hpp>
#include "sqlite3_exec_stmt.h"
#include <boost/thread.hpp>

engine::engine()	
{
	db = NULL;

	if(sqlite3_open_v2(":memory:", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL))
	{
		std::ostringstream msg;
		msg << "Can't create database: " << sqlite3_errmsg(db);
		throw std::runtime_error(msg.str().c_str());				
	}	

	sqlite3_exec(db, "CREATE TABLE images (name TEXT, patid TEXT, birthday TEXT, \
					 studyuid TEXT, modality TEXT, studydesc TEXT, studydate TEXT, \
					 seriesuid TEXT, seriesdesc TEXT, \
					 sopuid TEXT UNIQUE, filename TEXT, sent INTEGER)", NULL, NULL, NULL);

}

engine::~engine()
{
	if(db)
		sqlite3_close(db);
}

void engine::SaveDestinationList()
{
	wxConfig::Get()->SetPath("/");
	wxConfig::Get()->DeleteGroup("Destinations");
	for(unsigned int i = 0; i < destinations.size(); i++)
	{

		std::stringstream stream;
		stream << destinations[i].name << "," 
			<< destinations[i].destinationHost << "," 
			<< destinations[i].destinationPort << "," 
			<< destinations[i].destinationAETitle << "," 
			<< destinations[i].ourAETitle;

		wxConfig::Get()->SetPath("/Destinations");
		wxConfig::Get()->Write(boost::lexical_cast<std::string>(i + 1), wxString::FromUTF8(stream.str().c_str()));
	}

	wxConfig::Get()->Flush();
}

void engine::LoadDestinationList()
{		
	wxConfig::Get()->SetPath("/Destinations");
	wxString str;
	long dummy;
	// first enum all entries
	bool bCont = wxConfig::Get()->GetFirstEntry(str, dummy);
	while ( bCont ) 
	{
		using namespace boost::spirit::classic;

		wxString data;
		data = wxConfig::Get()->Read(str);

		std::vector<std::string> items;		
		//parse(data.ToStdWstring().c_str(),
		parse(data.ToUTF8().data(),
			((*(anychar_p - L','))[append(items)]) >>
			(L',') >>
			((*(anychar_p - L','))[append(items)]) >>
			(L',') >>
			((*(anychar_p - L','))[append(items)]) >>
			(L',') >>
			((*(anychar_p - L','))[append(items)]) >>
			(L',') >>
			(*anychar_p)[append(items)]
		, space_p);

		if(items.size() == 5)
		{
			// try, if error, just don't add it.
			try { destinations.push_back(DestinationEntry(items[0], items[1], boost::lexical_cast<int>(items[2]), items[3], items[4])); }
			catch(...) {}
		}
		
		bCont = wxConfig::Get()->GetNextEntry(str, dummy);
	}
}


void engine::LoadGlobalDestinationList()
{	
	// turn off error message
	wxLogNull nolog;

	// only valid for windows
#if defined(__WINDOWS__) && wxUSE_CONFIG_NATIVE
	wxRegKey registry;
	registry.SetName(wxRegKey::HKLM, "Software\\Policies\\FrontMotion\\fmdeye\\Destinations");
	registry.Open(wxRegKey::Read);

	wxString str;
	long dummy;
	// first enum all entries
	bool bCont = registry.GetFirstValue(str, dummy);
	while ( bCont ) 
	{
		using namespace boost::spirit::classic;

		wxString data;		
		registry.QueryValue(str, data);

		std::vector<std::string> items;
		parse(data.ToUTF8().data(),
			((*(anychar_p - L','))[append(items)]) >>
			(L',') >>
			((*(anychar_p - L','))[append(items)]) >>
			(L',') >>
			((*(anychar_p - L','))[append(items)]) >>
			(L',') >>
			((*(anychar_p - L','))[append(items)]) >>
			(L',') >>
			(*anychar_p)[append(items)]
		, space_p);

		if(items.size() == 5)
		{
			try { globalDestinations.push_back(DestinationEntry(items[0].c_str(), items[1].c_str(), boost::lexical_cast<int>(items[2]), items[3].c_str(), items[4].c_str())); }
			catch(...) {}
		}

		bCont = registry.GetNextValue(str, dummy);
	}
#endif
}

void engine::StartScan(boost::filesystem::path path)
{
	sqlite3_exec(db, "DELETE FROM images", NULL, NULL, NULL);
	scanner.Initialize(db, path);
	boost::thread t(DICOMFileScanner::DoScanThread, &scanner);
	t.detach();
}

void engine::StopScan()
{
	scanner.Cancel();
}


void engine::StartSend(std::string PatientName, std::string NewPatientName, std::string NewPatientID, std::string NewBirthDay, int destination)
{
	// find the destination
	std::string destinationHost;
	int destinationPort;
	std::string destinationAETitle, ourAETitle;

	if(destination < globalDestinations.size())
	{
		destinationHost = globalDestinations[destination].destinationHost;
		destinationPort = globalDestinations[destination].destinationPort;
		destinationAETitle = globalDestinations[destination].destinationAETitle;
		ourAETitle = globalDestinations[destination].ourAETitle;
	}
	else
	{
		destination -= globalDestinations.size();
		destinationHost = destinations[destination].destinationHost;
		destinationPort = destinations[destination].destinationPort;
		destinationAETitle = destinations[destination].destinationAETitle;
		ourAETitle = destinations[destination].ourAETitle;
	}

	sender.Initialize(db, PatientName, NewPatientName, NewPatientID, NewBirthDay,
		destinationHost, destinationPort, destinationAETitle, ourAETitle);

	boost::thread t(DICOMSender::DoSendThread, &sender);
	t.detach(); 
}

void engine::StopSend()
{
	sender.Cancel();
}

void engine::GetPatients(sqlite3_callback fillname, void *obj)
{	
	std::string selectsql = "SELECT name, patid, birthday FROM images GROUP BY name, patid ORDER BY name";
	sqlite3_stmt *select;
	sqlite3_prepare_v2(db, selectsql.c_str(), selectsql.length(), &select, NULL);
	sqlite3_exec_stmt(select, fillname, obj, NULL);
	sqlite3_finalize(select);
}

void engine::GetStudies(std::string patientname, sqlite3_callback fillstudy, void *obj)
{
	std::string selectsql = "SELECT DISTINCT studyuid, studydesc, studydate FROM images WHERE name = ? ORDER BY studyuid ASC";
	sqlite3_stmt *select;
	sqlite3_prepare_v2(db, selectsql.c_str(), selectsql.length(), &select, NULL);

	sqlite3_bind_text(select, 1, patientname.c_str(), patientname.length(), SQLITE_STATIC);

	sqlite3_exec_stmt(select, fillstudy, obj, NULL);		
	sqlite3_finalize(select);
}

void engine::GetSeries(std::string studyuid, sqlite3_callback fillseries, void *obj)
{
	std::string selectsql = "SELECT DISTINCT seriesuid, seriesdesc FROM images WHERE (studyuid = ?) ORDER BY seriesuid ASC";
	sqlite3_stmt *select;
	sqlite3_prepare_v2(db, selectsql.c_str(), selectsql.length(), &select, NULL);

	sqlite3_bind_text(select, 1, studyuid.c_str(), studyuid.length(), SQLITE_STATIC);

	sqlite3_exec_stmt(select, fillseries, obj, NULL);		
	sqlite3_finalize(select);
}


void engine::GetInstances(std::string seriesuid, sqlite3_callback fillinstances, void *obj)
{
	std::string selectsql = "SELECT sopuid, filename FROM images WHERE (seriesuid = ?) ORDER BY sopuid ASC";
	sqlite3_stmt *select;
	sqlite3_prepare_v2(db, selectsql.c_str(), selectsql.length(), &select, NULL);

	sqlite3_bind_text(select, 1, seriesuid.c_str(), seriesuid.length(), SQLITE_STATIC);

	sqlite3_exec_stmt(select, fillinstances, obj, NULL);		
	sqlite3_finalize(select);
}

void engine::GetImage(std::string sopid, sqlite3_callback fillinstances, void *obj)
{
	/*
	std::string selectsql = "SELECT filename FROM images WHERE (sopuid = ?)";
	sqlite3_stmt *select;
	sqlite3_prepare_v2(db, selectsql.c_str(), selectsql.length(), &select, NULL);

	sqlite3_bind_text(select, 1, sopid.c_str(), sopid.length(), SQLITE_STATIC);

	sqlite3_exec_stmt(select, fillinstances, obj, NULL);		
	sqlite3_finalize(select);*/
	boost::filesystem::path filename;
	

}