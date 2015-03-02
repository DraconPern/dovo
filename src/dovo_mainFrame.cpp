#include "dovo_mainFrame.h"
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_escape_char.hpp>
using namespace boost::spirit::classic;

dovo_mainFrame::dovo_mainFrame( wxWindow* parent )
	:
	mainFrame( parent )
{
	wxConfig config("fmdeye", "FrontMotion");

	config.SetPath("/Settings");	
	m_directory->SetValue(config.Read("LastDir"));

	config.SetPath("/Destinations");
	wxString str;
	long dummy;
	// first enum all entries
	bool bCont = config.GetFirstEntry(str, dummy);
	while ( bCont ) {

		std::string data;
		std::vector<std::wstring> items;
		data = config.Read("str");
		/*parse(data,
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
			PolicyDestinations.push_back(DestinationEntry(items[0].c_str(), items[1].c_str(), _wtoi(items[2].c_str()), items[3].c_str(), items[4].c_str()));			
		}*/
		bCont = config.GetNextEntry(str, dummy);
	}

	SetDestinations();
}

dovo_mainFrame::~dovo_mainFrame()
{
	wxConfig config("fmdeye", "FrontMotion");

	config.SetPath("/Settings");	
	config.Write("LastDir", m_directory->GetValue());
}
/*

void dovo_mainFrame::LoadPolicyDestinations()
{
CLoadDestinations reg(HKEY_LOCAL_MACHINE);
reg.RegistryTreeWalk(L"Software\\Policies\\FrontMotion\\fmdeye\\Destinations", CRegistry::RTW_NotifyValue);

std::map<CString, CString>::iterator itr;
for(itr = reg.entries.begin(); itr != reg.entries.end(); itr++)
{
std::vector<std::wstring> items;

parse((*itr).second.GetBuffer(),
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
PolicyDestinations.push_back(DestinationEntry(items[0].c_str(), items[1].c_str(), _wtoi(items[2].c_str()), items[3].c_str(), items[4].c_str()));			
}
}
}

void dovo_mainFrame::LoadDestinations()
{
CLoadDestinations reg;
reg.RegistryTreeWalk(L"Software\\FrontMotion\\fmdeye\\Destinations", CRegistry::RTW_NotifyValue);

std::map<CString, CString>::iterator itr;
for(itr = reg.entries.begin(); itr != reg.entries.end(); itr++)
{
std::vector<std::wstring> items;

parse((*itr).second.GetBuffer(),
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
Destinations.push_back(DestinationEntry(items[0].c_str(), items[1].c_str(), _wtoi(items[2].c_str()), items[3].c_str(), items[4].c_str()));			
}
}
}
*/
void dovo_mainFrame::SetDestinations()
{
	// add to combo box
	m_destination->Clear();
	std::vector<DestinationEntry>::iterator itr;	
	for(itr = policyDestinations.begin(); itr != policyDestinations.end(); itr++)
		m_destination->Append((*itr).Name + L" (*)");

	for(itr = destinations.begin(); itr != destinations.end(); itr++)
		m_destination->Append((*itr).Name);
}
void dovo_mainFrame::OnBrowse( wxCommandEvent& event )
{	
	wxDirDialog dlg(this, "", m_directory->GetValue(), wxRESIZE_BORDER | wxDD_DIR_MUST_EXIST);
	if(dlg.ShowModal() == wxID_OK)
	{
		m_directory->SetValue(dlg.GetPath());		
	}
}

void dovo_mainFrame::OnDestinationEdit( wxCommandEvent& event )
{
	dovo_destination dlg(this);
	dlg.destinations = destinations;

	if(dlg.ShowModal() == wxID_OK)
	{
		destinations = dlg.destinations;
		SetDestinations();
	}
}

