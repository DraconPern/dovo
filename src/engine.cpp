
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


engine::engine()	
	: scanner(patientdata), sender(patientdata)
{
	
}

engine::~engine()
{
	
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
	registry.SetName(wxRegKey::HKLM, "Software\\Policies\\FrontMotion\\dovo\\Destinations");
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
	patientdata.Clear();
	scanner.DoScanAsync(path);
}

void engine::StopScan()
{
	scanner.Cancel();
}


void engine::StartSend(std::string PatientID, std::string PatientName, bool changePatientInfo, std::string NewPatientID, std::string NewPatientName, std::string NewBirthDay, int destination)
{
	// find the destination
	DestinationEntry dest;

	if(destination < globalDestinations.size())
	{
		dest = globalDestinations[destination];		
	}
	else
	{
		destination -= globalDestinations.size();
		dest = destinations[destination];
	}
	
	sender.DoSendAsync(PatientID, PatientName, changePatientInfo, NewPatientID, NewPatientName, NewBirthDay, dest);
	
}

void engine::StopSend()
{
	sender.Cancel();
}
