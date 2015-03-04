#include "dovo_mainFrame.h"

dovo_mainFrame::dovo_mainFrame( wxWindow* parent )
	:
	mainFrame( parent )
{	
	// Create the UI
	m_patients->InsertColumn(0, "Name");
	m_patients->InsertColumn(1, "ID");
	m_patients->InsertColumn(2, "Birthday");

	m_studies->InsertColumn(0, "Date");
	m_studies->InsertColumn(1, "Study");
	m_studies->InsertColumn(2, "StudyID");

	m_series->InsertColumn(0, "Series");
	m_series->InsertColumn(1, "SeriesID");

	m_instances->InsertColumn(0, L"SOPUid");
	m_instances->InsertColumn(1, L"path");

	// Fill in settings
	wxConfig::Get()->SetPath("/Settings");
	m_directory->SetValue(wxConfig::Get()->Read("LastDir"));

	m_engine.LoadDestinationList();
	m_engine.LoadGlobalDestinationList();

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
	dlg.m_destinations = m_engine.destinations;

	if(dlg.ShowModal() == wxID_OK)
	{
		std::string oldsel = m_destination->GetStringSelection();
		m_engine.destinations = dlg.m_destinations;	
		m_engine.SaveDestinationList();
		
		FillDestinationList();
		m_destination->SetStringSelection(oldsel);
	}
}


int fillname(void *param,int columns,char** values, char**names)
{
	dovo_mainFrame *win = (dovo_mainFrame *) param;
	/*
	win->m_patients->InsertItem(0, CA2W(values[0], CP_UTF8));
	dlg->patientsCtrl.SetItemText(0, 1, CA2W(values[1], CP_UTF8));
	dlg->patientsCtrl.SetItemText(0, 2, CA2W(values[2], CP_UTF8));
	*/
	return 0; 
}

void dovo_mainFrame::OnUpdate( wxCommandEvent& event )
{		
	m_patients->DeleteAllItems();

	std::string a =  m_directory->GetValue();
	m_engine.StartScan(a);


	/*
	);
	*/
	dovo_searchStatus dlg(this);
	//dlg.scanner = &scanner;
	
	dlg.ShowModal();	
	

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
	for(itr = m_engine.globalDestinations.begin(); itr != m_engine.globalDestinations.end(); itr++)
		m_destination->Append((*itr).name + L" (*)");

	for(itr = m_engine.destinations.begin(); itr != m_engine.destinations.end(); itr++)
		m_destination->Append((*itr).name);
}
