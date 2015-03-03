#include "dovo_mainFrame.h"

dovo_mainFrame::dovo_mainFrame( wxWindow* parent )
	:
	mainFrame( parent )
{	
	wxConfig::Get()->SetPath("/Settings");
	m_directory->SetValue(wxConfig::Get()->Read("LastDir"));

	LoadDestinationList();

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
	wxConfig::Get()->SetPath("/Settings");	
	wxConfig::Get()->Write("LastDir", m_directory->GetValue());

	SaveDestinationList();
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
			destinations.push_back(DestinationEntry(items[0].c_str(), items[1].c_str(), atoi(items[2].c_str()), items[3].c_str(), items[4].c_str()));

		bCont = wxConfig::Get()->GetNextEntry(str, dummy);
	}

}


void dovo_mainFrame::LoadGlobalDestinationList()
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
			destinations.push_back(DestinationEntry(items[0].c_str(), items[1].c_str(), atoi(items[2].c_str()), items[3].c_str(), items[4].c_str()));

		bCont = wxConfig::Get()->GetNextEntry(str, dummy);
	}

}