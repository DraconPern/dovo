#include "dovo_mainFrame.h"

dovo_mainFrame::dovo_mainFrame( wxWindow* parent )
	:
	mainFrame( parent )
{	
	wxConfig::Get()->SetPath("/Settings");
	m_directory->SetValue(wxConfig::Get()->Read("LastDir"));

	LoadDestinationList();
	LoadGlobalDestinationList();

	FillDestinationList();
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
	dlg.m_destinations = destinations;

	if(dlg.ShowModal() == wxID_OK)
	{
		destinations = dlg.m_destinations;
		FillDestinationList();
		SaveDestinationList();
		wxConfig::Get()->Flush();
	}
}

void dovo_mainFrame::OnUpdate( wxCommandEvent& event )
{
	// TODO: Implement OnUpdate
}

void dovo_mainFrame::OnSend( wxCommandEvent& event )
{
	// TODO: Implement OnSend
}

void dovo_mainFrame::OnAbout( wxCommandEvent& event )
{
	dovo_about dlg(this);	
	dlg.ShowModal();
}

void dovo_mainFrame::OnExit( wxCommandEvent& event )
{
	Close();
}


dovo_mainFrame::~dovo_mainFrame()
{	
	wxConfig::Get()->SetPath("/Settings");	
	wxConfig::Get()->Write("LastDir", m_directory->GetValue());
	wxConfig::Get()->Flush();
}

void dovo_mainFrame::FillDestinationList()
{
	// add to combo box
	m_destination->Clear();
	std::vector<DestinationEntry>::iterator itr;	
	for(itr = globalDestinations.begin(); itr != globalDestinations.end(); itr++)
		m_destination->Append((*itr).name + L" (*)");

	for(itr = destinations.begin(); itr != destinations.end(); itr++)
		m_destination->Append((*itr).name);
}

void dovo_mainFrame::SaveDestinationList()
{
	wxConfig::Get()->SetPath("/");
	wxConfig::Get()->DeleteGroup("Destinations");
	for(int i = 0; i < destinations.size(); i++)
	{
		
		std::ostringstream stream;
		stream << destinations[i].name << "," 
			<< destinations[i].destinationHost << "," 
			<< destinations[i].destinationPort << "," 
			<< destinations[i].destinationAETitle << "," 
			<< destinations[i].ourAETitle;
		
		wxConfig::Get()->SetPath("/Destinations");
		wxConfig::Get()->Write(boost::lexical_cast<std::string>(i + 1), stream.str().c_str());
	}

	wxConfig::Get()->Flush();
}

void dovo_mainFrame::LoadDestinationList()
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
		std::vector<std::string> items;
		data = wxConfig::Get()->Read(str);
		parse((const char*)data.mb_str(wxConvUTF8),
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
			destinations.push_back(DestinationEntry(items[0], items[1], boost::lexical_cast<int>(items[2]), items[3], items[4]));

		bCont = wxConfig::Get()->GetNextEntry(str, dummy);
	}
}


void dovo_mainFrame::LoadGlobalDestinationList()
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
		std::vector<std::string> items;
		registry.QueryValue(str, data);
		parse((const char*)data.mb_str(wxConvUTF8),
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
			globalDestinations.push_back(DestinationEntry(items[0].c_str(), items[1].c_str(), atoi(items[2].c_str()), items[3].c_str(), items[4].c_str()));

		bCont = registry.GetNextValue(str, dummy);
	}
#endif
}