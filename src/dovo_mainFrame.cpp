#include "dovo_mainFrame.h"

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
	while ( bCont ) 
	{
		using namespace boost::spirit::classic;

		wxString data;
		std::vector<std::string> items;
		data = config.Read(str);
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
			destinations.push_back(DestinationEntry(items[0].c_str(), items[1].c_str(), atoi(items[2].c_str()), items[3].c_str(), items[4].c_str()));

		bCont = config.GetNextEntry(str, dummy);
	}

	SetDestinations();
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
		SetDestinations();
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
	// TODO: Implement OnAbout
}

void dovo_mainFrame::OnExit( wxCommandEvent& event )
{
	Close();
}


dovo_mainFrame::~dovo_mainFrame()
{
	wxConfig config("fmdeye", "FrontMotion");

	config.SetPath("/Settings");	
	config.Write("LastDir", m_directory->GetValue());
}

void dovo_mainFrame::SetDestinations()
{
	// add to combo box
	m_destination->Clear();
	std::vector<DestinationEntry>::iterator itr;	
	for(itr = policyDestinations.begin(); itr != policyDestinations.end(); itr++)
		m_destination->Append((*itr).name + L" (*)");

	for(itr = destinations.begin(); itr != destinations.end(); itr++)
		m_destination->Append((*itr).name);
}
