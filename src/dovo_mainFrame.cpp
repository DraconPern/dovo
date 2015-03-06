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

void dovo_mainFrame::OnPatientsSelected( wxListEvent& event )
{
	// TODO: Implement OnPatientsSelected
}

void dovo_mainFrame::OnStudiesSelected( wxListEvent& event )
{
	// TODO: Implement OnStudiesSelected
}

void dovo_mainFrame::OnSeriesSelected( wxListEvent& event )
{
	// TODO: Implement OnSeriesSelected
}

void dovo_mainFrame::OnInstancesSelected( wxListEvent& event )
{
	// TODO: Implement OnInstancesSelected
}

void dovo_mainFrame::OnUpdate( wxCommandEvent& event )
{
	m_patients->DeleteAllItems();

	m_engine.StartScan(m_directory->GetValue());

	dovo_searchStatus dlg(this);
	dlg.m_scanner = &m_engine.scanner;

	// show and wait for thread to end.
	dlg.ShowModal();

	m_engine.GetPatients(fillname, this);
}

void dovo_mainFrame::OnSend( wxCommandEvent& event )
{	
	long item = m_patients->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
	{
		wxMessageBox("Please select a Patient.", "Error", wxOK, this);
		return;
	}
	
	if(m_destination->GetSelection() == wxNOT_FOUND)
	{
		wxMessageBox("Please select a destination.", "Error", wxOK, this);
		return;
	}

	wxString patientname = m_patients->GetItemText(item);
	m_engine.StartSend(patientname, "", "", "", m_destination->GetSelection());

	dovo_sendStatus dlg(this);
	dlg.m_sender = &m_engine.sender;

	// show and wait for thread to end.
	dlg.ShowModal();
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


int dovo_mainFrame::fillname(void *param,int columns,char** values, char**names)
{
	dovo_mainFrame *win = (dovo_mainFrame *) param;	
	win->m_patients->InsertItem(0, values[0]);
	win->m_patients->SetItem(0, 1, values[1]);
	win->m_patients->SetItem(0, 2, values[2]);
	return 0; 
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