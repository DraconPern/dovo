#include "dovo_mainFrame.h"

dovo_mainFrame::dovo_mainFrame( wxWindow* parent )
	:
	mainFrame( parent )
{
#if defined(_WIN32)
	SetMinSize(wxSize(800, 700));
	SetSize(wxSize(800, 700));
#elif defined(__WXMAC__)
	SetMinSize(wxSize(800, 700));
	SetSize(wxSize(800, 700));
	m_about->Hide();
	m_exit->Hide();
#endif 
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
		wxString oldsel = m_destination->GetStringSelection();
		m_engine.destinations = dlg.m_destinations;
		m_engine.SaveDestinationList();

		FillDestinationList();
		m_destination->SetStringSelection(oldsel);
	}
}

void dovo_mainFrame::OnPatientsSelected( wxListEvent& event )
{
	long item = m_patients->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	m_studies->DeleteAllItems();
	m_engine.GetStudies(m_patients->GetItemText(item).ToUTF8().data(), fillstudies, this);
	m_studies->SetColumnWidth(0, wxLIST_AUTOSIZE);
	m_studies->SetColumnWidth(1, wxLIST_AUTOSIZE);
	m_studies->SetColumnWidth(2, wxLIST_AUTOSIZE);

	if(m_studies->GetItemCount() > 0)
		m_studies->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void dovo_mainFrame::OnStudiesSelected( wxListEvent& event )
{
	long item = m_studies->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	m_series->DeleteAllItems();
	m_engine.GetSeries(m_studies->GetItemText(item, 2).ToUTF8().data(), fillseries, this);
	m_series->SetColumnWidth(0, wxLIST_AUTOSIZE);
	m_series->SetColumnWidth(1, wxLIST_AUTOSIZE);	

	if(m_series->GetItemCount() > 0)
		m_series->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void dovo_mainFrame::OnSeriesSelected( wxListEvent& event )
{
	long item = m_series->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	m_instances->DeleteAllItems();
	m_engine.GetInstances(m_series->GetItemText(item, 1).ToUTF8().data(), fillinstances, this);
	m_instances->SetColumnWidth(0, wxLIST_AUTOSIZE);
	m_instances->SetColumnWidth(1, wxLIST_AUTOSIZE);

	if(m_instances->GetItemCount() > 0)
		m_instances->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void dovo_mainFrame::OnInstancesSelected( wxListEvent& event )
{
	// draw it
}

void dovo_mainFrame::OnUpdate( wxCommandEvent& event )
{
	m_patients->DeleteAllItems();
	m_studies->DeleteAllItems();
	m_series->DeleteAllItems();
	m_instances->DeleteAllItems();
#ifdef _WIN32
    boost::filesystem::path p = m_directory->GetValue();
#else
	boost::filesystem::path p = m_directory->GetValue().ToUTF8().data();
#endif
    m_engine.StartScan(p);

	dovo_searchStatus dlg(this);
	dlg.m_scanner = &m_engine.scanner;

	// show and wait for thread to end.
	dlg.ShowModal();

	m_engine.GetPatients(fillpatients, this);
	m_patients->SetColumnWidth(0, wxLIST_AUTOSIZE);
	m_patients->SetColumnWidth(1, wxLIST_AUTOSIZE);
	m_patients->SetColumnWidth(2, wxLIST_AUTOSIZE);

	if(m_patients->GetItemCount() > 0)
		m_patients->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
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
	m_engine.StartSend(patientname.ToUTF8().data(), "", "", "", m_destination->GetSelection());

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


void dovo_mainFrame::FillDestinationList()
{
	// add to combo box
	m_destination->Clear();
	std::vector<DestinationEntry>::iterator itr;	
	for(itr = m_engine.globalDestinations.begin(); itr != m_engine.globalDestinations.end(); itr++)	
		m_destination->Append(wxString::FromUTF8((*itr).name.c_str()) + L" (*)");

	for(itr = m_engine.destinations.begin(); itr != m_engine.destinations.end(); itr++)
		m_destination->Append(wxString::FromUTF8((*itr).name.c_str()));	
}


dovo_mainFrame::~dovo_mainFrame()
{
	wxConfig::Get()->SetPath("/Settings");	
	wxConfig::Get()->Write("LastDir", m_directory->GetValue());
	wxConfig::Get()->Flush();
}

int dovo_mainFrame::fillstudies(void *param,int columns,char** values, char**names)
{
	dovo_mainFrame *win = (dovo_mainFrame *) param;	

	std::string date;
	date = values[2];

	struct tm thedate;
	memset(&thedate, 0, sizeof(struct tm));
	thedate.tm_year = boost::lexical_cast<int>(date.substr(0, 4)) - 1900;
	thedate.tm_mon = boost::lexical_cast<int>(date.substr(4, 2)) - 1;
	thedate.tm_mday = boost::lexical_cast<int>(date.substr(6, 2));

	char buf[1024];
	memset(buf, 0, 1024);

	strftime(buf, 1024, "%x", &thedate);

	win->m_studies->InsertItem(0, buf);
	win->m_studies->SetItem(0, 1, wxString::FromUTF8(values[1]));
	win->m_studies->SetItem(0, 2, wxString::FromUTF8(values[0]));
	return 0; 
}

int dovo_mainFrame::fillpatients(void *param,int columns,char** values, char**names)
{
	dovo_mainFrame *win = (dovo_mainFrame *) param;	
	win->m_patients->InsertItem(0, values[0]);
	win->m_patients->SetItem(0, 1, values[1]);
	win->m_patients->SetItem(0, 2, values[2]);
	return 0; 
}

int dovo_mainFrame::fillseries(void *param,int columns,char** values, char**names)
{
	dovo_mainFrame *win = (dovo_mainFrame *) param;	
	win->m_series->InsertItem(0, wxString::FromUTF8(values[1]));
	win->m_series->SetItem(0, 1, wxString::FromUTF8(values[0]));	
	return 0; 
}

int dovo_mainFrame::fillinstances(void *param,int columns,char** values, char**names)
{

	dovo_mainFrame *win = (dovo_mainFrame *) param;	
	win->m_instances->InsertItem(0, wxString::FromUTF8(values[0]));
	win->m_instances->SetItem(0, 1, wxString::FromUTF8(values[1]));	
	return 0; 
}