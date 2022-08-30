#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <iterator>
#include <boost/thread.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include "fetch.h"

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/platinfo.h>

#include "dovo_updateCheck.h"

// returns true if the application needs to exit
bool informUserOfUpdate(std::string json)
{
	std::string thisversion = DOVO_VERSION;
	std::string version, message;
	bool mustupdate = false;
	dovo_updateCheck updatecheckdlg(NULL);

	try
	{
		boost::property_tree::ptree pt;
		std::istringstream is(json);
		boost::property_tree::read_json(is, pt);

		version = pt.get_child("dovo.version").data();
		mustupdate = pt.get_child("dovo.mustupdate").get_value<bool>();
		message = pt.get_child("dovo.message").data();
		updatecheckdlg.m_message = message;
		updatecheckdlg.m_version = version;
	}
	catch (std::exception& e)
	{
		return false;
	}

	boost::gregorian::date compiledate = boost::date_time::parse_date<boost::gregorian::date>(__DATE__, boost::date_time::ymd_order_us);
	boost::gregorian::date timelimit = compiledate + boost::gregorian::years(1);
	boost::gregorian::date today = boost::gregorian::day_clock::local_day();

	/*
	if(today > timelimit)
	{				

		updatecheckdlg.m_updatetext = wxT("This software is outdated, please upgrade!");		
		updatecheckdlg.ShowModal();
		return true;
	}*/

	if (version != thisversion)
	{
		if (mustupdate)
		{
			updatecheckdlg.m_updatetext = wxT("You must upgrade to a new version:");
			updatecheckdlg.ShowModal();
			return true;
		}

		updatecheckdlg.m_updatetext = wxT("A new version of dovo is available:");
		updatecheckdlg.ShowModal();
	}

	return false;
}

int getUpdateJSON(std::string &updateinfo);

void updateChecker()
{
	std::string updateinfo;
	getUpdateJSON(updateinfo);
	wxConfig::Get()->Write("/Settings/UpdateInfo", updateinfo.c_str());	
}

int getUpdateJSON(std::string &updateinfo)
{
	FetchOptions options;

	const wxPlatformInfo& pinfo = wxPlatformInfo::Get();
	std::stringstream agent;
	agent << "dovo/" DOVO_VERSION;
	agent << " (";
	agent << pinfo.GetOperatingSystemIdName() << " " << pinfo.GetOSMajorVersion() << "." << pinfo.GetOSMinorVersion();
	agent << ")";

	options.useragent = agent.str();
	std::string potentialupdateinfo = fetch_string_withretry("https://www.draconpern.com/software/dovo/", options);

	// process the json info that's embeded in the page
	if(potentialupdateinfo.length() == 0)
		return 1;

	std::string marker = "<!-- updatecheck";
	size_t startmarker = potentialupdateinfo.find(marker);
	if(startmarker == std::string::npos)
		return 1;
	startmarker += marker.length();

	size_t stopmarker = potentialupdateinfo.find("-->", startmarker);
	if(stopmarker == std::string::npos)
		return 1;

	updateinfo = potentialupdateinfo.substr(startmarker, stopmarker - startmarker);

	return 0;
}
