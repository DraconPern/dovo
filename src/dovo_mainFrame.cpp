#include "dovo_mainFrame.h"
#include <boost/lexical_cast.hpp>
#include <map>
#include "alphanum.hpp"

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
	m_patients->InsertColumn(0, "ID");
	m_patients->InsertColumn(1, _("Name"));
	m_patients->InsertColumn(2, _("Birthday"));

	m_studies->InsertColumn(0, _("Date"));
	m_studies->InsertColumn(1, _("Study"));
	m_studies->InsertColumn(2, "StudyUID");

	m_series->InsertColumn(0, _("Series"));
	m_series->InsertColumn(1, "SeriesUID");

	m_instances->InsertColumn(0, "sop UID");
	m_instances->InsertColumn(1, _("path"));

	// Fill in settings
	m_directory->SetValue(wxConfig::Get()->Read("/Settings/LastDir"));

	m_engine.LoadDestinationList();
	m_engine.LoadGlobalDestinationList();

	FillDestinationList();
	m_destination->SetStringSelection(wxConfig::Get()->Read("/Settings/LastDestination"));

	image.Create(1, 1);
}

dovo_mainFrame::~dovo_mainFrame()
{
	wxConfig::Get()->SetPath("/Settings");
	wxConfig::Get()->Write("LastDir", m_directory->GetValue());
	wxConfig::Get()->Write("LastDestination", m_destination->GetStringSelection());
	wxConfig::Get()->Flush();
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

void dovo_mainFrame::OnUpdate( wxCommandEvent& event )
{
	m_patients->DeleteAllItems();
	m_studies->DeleteAllItems();
	m_series->DeleteAllItems();
	m_instances->DeleteAllItems();

	boost::filesystem::path p(m_directory->GetValue().fn_str());
	m_engine.StartScan(p);

	dovo_searchStatus dlg(this);
	dlg.m_scanner = &m_engine.scanner;

	// show and wait for thread to end.
	dlg.ShowModal();
	
	m_engine.patientdata.GetPatients(boost::bind(&dovo_mainFrame::fillpatientscallback, this, boost::placeholders::_1));
	m_patients->SetColumnWidth(0, wxLIST_AUTOSIZE);
	m_patients->SetColumnWidth(1, wxLIST_AUTOSIZE);
	m_patients->SetColumnWidth(2, wxLIST_AUTOSIZE);

	//if(m_patients->GetItemCount() > 0)
	//	m_patients->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

int dovo_mainFrame::fillpatientscallback(Patient &patient)
{	
	m_patients->InsertItem(0, patient.patid);
	m_patients->SetItem(0, 1, wxString::FromUTF8(patient.name.c_str()));
	m_patients->SetItem(0, 2, patient.birthday);
	return 0;
}

void dovo_mainFrame::OnPatientsSelected( wxListEvent& event )
{
	long item = m_patients->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	m_studies->DeleteAllItems();
	m_series->DeleteAllItems();
	m_instances->DeleteAllItems();

	m_engine.patientdata.GetStudies(m_patients->GetItemText(item, 0).ToUTF8().data(), m_patients->GetItemText(item, 1).ToUTF8().data(), boost::bind(&dovo_mainFrame::fillstudiescallback, this, boost::placeholders::_1));

	if(m_studies->GetItemCount() > 0)
	{
		m_studies->SetColumnWidth(0, wxLIST_AUTOSIZE);
		m_studies->SetColumnWidth(1, wxLIST_AUTOSIZE);
		m_studies->SetColumnWidth(2, wxLIST_AUTOSIZE);
		m_studies->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}
}

int dovo_mainFrame::fillstudiescallback(Study &study)
{		
	struct tm thedate;
	memset(&thedate, 0, sizeof(struct tm));
	try
	{
		thedate.tm_year = boost::lexical_cast<int>(study.studydate.substr(0, 4)) - 1900;
		thedate.tm_mon = boost::lexical_cast<int>(study.studydate.substr(4, 2)) - 1;
		thedate.tm_mday = boost::lexical_cast<int>(study.studydate.substr(6, 2));
	}
	catch(...)
	{
		thedate.tm_year = 0;
		thedate.tm_mon = 0;
		thedate.tm_mday = 1;
	}

	char buf[1024];
	memset(buf, 0, 1024);

	strftime(buf, 1024, "%x", &thedate);

	m_studies->InsertItem(0, buf);
	m_studies->SetItem(0, 1, wxString::FromUTF8(study.studydesc.c_str()));
	m_studies->SetItem(0, 2, wxString::FromUTF8(study.studyuid.c_str()));
	return 0;
}

void dovo_mainFrame::OnStudiesSelected( wxListEvent& event )
{
	long item = m_studies->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	naturalstringmap entries;

	m_series->DeleteAllItems();
	m_instances->DeleteAllItems();

	m_engine.patientdata.GetSeries(m_studies->GetItemText(item, 2).ToUTF8().data(), boost::bind(&dovo_mainFrame::fillseriescallback, this, boost::placeholders::_1, &entries));

	int j = 0;
	for(naturalstringmap::iterator ii = entries.begin(); ii != entries.end(); ++ii)
	{
		m_series->InsertItem(j, wxString::FromUTF8((*ii).second.c_str()));
		m_series->SetItem(j, 1, wxString::FromUTF8((*ii).first.c_str()));
		j++;
	}

	if(m_series->GetItemCount() > 0)
	{
		m_series->SetColumnWidth(0, wxLIST_AUTOSIZE);
		m_series->SetColumnWidth(1, wxLIST_AUTOSIZE);
		m_series->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}
}

int dovo_mainFrame::fillseriescallback(Series &series, naturalstringmap *entries)
{	
	entries->insert(std::pair<std::string, std::string>(series.seriesuid, series.seriesdesc));	
	return 0;
}

void dovo_mainFrame::OnSeriesSelected( wxListEvent& event )
{
	long item = m_series->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	naturalpathmap entries;

	m_instances->DeleteAllItems();
	m_engine.patientdata.GetInstances(m_series->GetItemText(item, 1).ToUTF8().data(), boost::bind(&dovo_mainFrame::fillinstancescallback, this, boost::placeholders::_1, &entries));

	int j = 0;
	for(naturalpathmap::iterator ii = entries.begin(); ii != entries.end(); ++ii)
	{
		m_instances->InsertItem(j, wxString::FromUTF8((*ii).first.c_str()));
		m_instances->SetItem(j, 1, (*ii).second.c_str());
		j++;
	}

	if(m_instances->GetItemCount() > 0)
	{
		m_instances->SetColumnWidth(0, wxLIST_AUTOSIZE);
		m_instances->SetColumnWidth(1, wxLIST_AUTOSIZE);
		m_instances->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}
}

int dovo_mainFrame::fillinstancescallback(Instance &instance, naturalpathmap *entries)
{	
	entries->insert(std::pair<std::string, boost::filesystem::path>(instance.sopuid, instance.filename));
	return 0;
}

void dovo_mainFrame::OnInstancesSelected( wxListEvent& event )
{
	long item = m_instances->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	wxSize s = m_preview->GetClientSize();

	wxBusyCursor wait;
	boost::filesystem::path filename(m_instances->GetItemText(item, 1).fn_str());
	dcm2img(filename, s.GetWidth(), s.GetHeight(), image);

	wxClientDC dc(m_preview);
	renderPreview(dc);
}

void dovo_mainFrame::OnPaintPreview( wxPaintEvent& event )
{
	wxPaintDC dc(m_preview);
	renderPreview(dc);
}

void dovo_mainFrame::renderPreview(wxDC& dc)
{
	dc.DrawBitmap(image, 0, 0, false );
}

void dovo_mainFrame::OnSend( wxCommandEvent& event )
{
	long item = m_patients->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
	{
		wxMessageBox(_("Please select a Patient."), _("Error"), wxOK, this);
		return;
	}

	if(m_destination->GetSelection() == wxNOT_FOUND)
	{
		wxMessageBox(_("Please select a destination."), _("Error"), wxOK, this);
		return;
	}
	
	wxString patientid = m_patients->GetItemText(item);
	wxString patientname = m_patients->GetItemText(item, 1);

	dovo_changePatientInfo changepatinfo(this);
	changepatinfo.m_patientID = patientid;
	changepatinfo.m_patientName = patientname;
	changepatinfo.m_birthday = m_patients->GetItemText(item, 2);
	
	if(changepatinfo.ShowModal() == wxID_OK)
	{				
		m_engine.StartSend(patientid.ToUTF8().data(), 
			patientname.ToUTF8().data(),
			changepatinfo.m_changeInfo, 
			changepatinfo.m_patientID.ToUTF8().data(), 
			changepatinfo.m_patientName.ToUTF8().data(), 
			changepatinfo.m_birthday.ToUTF8().data(), 
			m_destination->GetSelection());

		dovo_sendStatus dlg(this);
		dlg.m_sender = &m_engine.sender;

		// show and wait for thread to end.
		dlg.ShowModal();
	}
}

void dovo_mainFrame::OnQuickSend(wxCommandEvent& event)
{
	if (m_destination->GetSelection() == wxNOT_FOUND)
	{
		wxMessageBox(_("Please select a destination."), _("Error"), wxOK, this);
		return;
	}

	m_patients->DeleteAllItems();
	m_studies->DeleteAllItems();
	m_series->DeleteAllItems();
	m_instances->DeleteAllItems();

	boost::filesystem::path p(m_directory->GetValue().fn_str());
	m_engine.StartScan(p);

	dovo_searchStatus search_dlg(this);
	search_dlg.m_scanner = &m_engine.scanner;

	// show and wait for thread to end.
	int search_status = search_dlg.ShowModal();

	m_engine.patientdata.GetPatients(boost::bind(&dovo_mainFrame::fillpatientscallback, this, boost::placeholders::_1));
	m_patients->SetColumnWidth(0, wxLIST_AUTOSIZE);
	m_patients->SetColumnWidth(1, wxLIST_AUTOSIZE);
	m_patients->SetColumnWidth(2, wxLIST_AUTOSIZE);

	if (search_status != wxID_OK)
		return;

	// start send
	m_engine.StartQuickSend(m_destination->GetSelection());

	dovo_sendStatus dlg(this);
	dlg.m_sender = &m_engine.sender;

	// show and wait for thread to end.
	dlg.ShowModal();
}

void dovo_mainFrame::OnEcho(wxCommandEvent& event)
{
	if (m_destination->GetSelection() == wxNOT_FOUND)
	{
		wxMessageBox(_("Please select a destination."), _("Error"), wxOK, this);
		return;
	}

	int destination = m_destination->GetSelection();
	// find the destination
	DestinationEntry dest;

	if (destination < m_engine.globalDestinations.size())
	{
		dest = m_engine.globalDestinations[destination];
	}
	else
	{
		destination -= m_engine.globalDestinations.size();
		dest = m_engine.destinations[destination];
	}

	wxBusyCursor wait;
	if (DICOMSender::Echo(dest))
	{
		wxMessageBox(_("Success"), _("Echo"), wxOK, this);
	}
	else
	{
		wxMessageBox(_("Failed"), _("Echo"), wxOK, this);
	}

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

void dovo_mainFrame::OnStudiesRightClick( wxListEvent& event)
{
	//m_studies.men
	//void *data = reinterpret_cast<void *>(event.().GetData());
	wxMenu mnu;
	//mnu.SetClientData(data);
	mnu.Append(1, "Send Study");
	mnu.Append(2, "Copy StudyUID");
	mnu.Connect(1, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(dovo_mainFrame::OnSendStudy), NULL, this);
	mnu.Connect(2, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(dovo_mainFrame::OnCopyStudyUID), NULL, this);
	PopupMenu(&mnu);	
}

void dovo_mainFrame::OnSeriesRightClick( wxListEvent& event)
{
	//m_studies.men
	//void *data = reinterpret_cast<void *>(event.().GetData());
	wxMenu mnu;
	//mnu.SetClientData(data);
	mnu.Append(1, "Send Series");
	mnu.Append(2, "Copy SeriesUID");
	mnu.Connect(1, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(dovo_mainFrame::OnSendSeries), NULL, this);
	mnu.Connect(2, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(dovo_mainFrame::OnCopySeriesUID), NULL, this);
	PopupMenu(&mnu);
}

void dovo_mainFrame::OnInstancesRightClick(wxListEvent& event)
{
	//m_studies.men
	//void *data = reinterpret_cast<void *>(event.().GetData());
	wxMenu mnu;
	//mnu.SetClientData(data);	
	mnu.Append(2, "Copy SOPUID");	
	mnu.Connect(2, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(dovo_mainFrame::OnCopySOPUID), NULL, this);
	PopupMenu(&mnu);
}

void dovo_mainFrame::OnSendStudy(wxCommandEvent& event)
{
	long item = m_patients->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
	{
		wxMessageBox(_("Please select a Patient."), _("Error"), wxOK, this);
		return;
	}

	long item2 = m_studies->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item2 == -1)
	{
		wxMessageBox(_("Please select a Study."), _("Error"), wxOK, this);
		return;
	}

	if (m_destination->GetSelection() == wxNOT_FOUND)
	{
		wxMessageBox(_("Please select a destination."), _("Error"), wxOK, this);
		return;
	}

	wxString patientid = m_patients->GetItemText(item);
	wxString patientname = m_patients->GetItemText(item, 1);

	dovo_changePatientInfo changepatinfo(this);
	changepatinfo.m_patientID = patientid;
	changepatinfo.m_patientName = patientname;
	changepatinfo.m_birthday = m_patients->GetItemText(item, 2);

	wxString studyuid = m_studies->GetItemText(item2, 2);

	if (changepatinfo.ShowModal() == wxID_OK)
	{
		m_engine.StartSendStudy(studyuid.ToUTF8().data(),			
			changepatinfo.m_changeInfo,
			changepatinfo.m_patientID.ToUTF8().data(),
			changepatinfo.m_patientName.ToUTF8().data(),
			changepatinfo.m_birthday.ToUTF8().data(),
			m_destination->GetSelection());

		dovo_sendStatus dlg(this);
		dlg.m_sender = &m_engine.sender;

		// show and wait for thread to end.
		dlg.ShowModal();
	}
}

void dovo_mainFrame::OnSendSeries(wxCommandEvent& event)
{
	long item = m_patients->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
	{
		wxMessageBox(_("Please select a Patient."), _("Error"), wxOK, this);
		return;
	}

	long item2 = m_studies->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item2 == -1)
	{
		wxMessageBox(_("Please select a Study."), _("Error"), wxOK, this);
		return;
	}

	long item3 = m_series->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item3 == -1)
	{
		wxMessageBox(_("Please select a Series."), _("Error"), wxOK, this);
		return;
	}

	if (m_destination->GetSelection() == wxNOT_FOUND)
	{
		wxMessageBox(_("Please select a destination."), _("Error"), wxOK, this);
		return;
	}

	wxString patientid = m_patients->GetItemText(item);
	wxString patientname = m_patients->GetItemText(item, 1);

	dovo_changePatientInfo changepatinfo(this);
	changepatinfo.m_patientID = patientid;
	changepatinfo.m_patientName = patientname;
	changepatinfo.m_birthday = m_patients->GetItemText(item, 2);

	wxString studyuid = m_studies->GetItemText(item2, 2);
	wxString seriesuid = m_series->GetItemText(item3, 1);

	if (changepatinfo.ShowModal() == wxID_OK)
	{
		m_engine.StartSendSeries(studyuid.ToUTF8().data(),
			seriesuid.ToUTF8().data(),
			changepatinfo.m_changeInfo,
			changepatinfo.m_patientID.ToUTF8().data(),
			changepatinfo.m_patientName.ToUTF8().data(),
			changepatinfo.m_birthday.ToUTF8().data(),
			m_destination->GetSelection());

		dovo_sendStatus dlg(this);
		dlg.m_sender = &m_engine.sender;

		// show and wait for thread to end.
		dlg.ShowModal();
	}
}

void dovo_mainFrame::OnCopyStudyUID(wxCommandEvent& event)
{
	long item2 = m_studies->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item2 == -1)
	{
		wxMessageBox(_("Please select a Study."), _("Error"), wxOK, this);
		return;
	}

	wxString studyuid = m_studies->GetItemText(item2, 2);

	if (wxTheClipboard->Open())
	{
		wxTheClipboard->Clear();
		wxTheClipboard->SetData(new wxTextDataObject(studyuid));
		wxTheClipboard->Flush();
		wxTheClipboard->Close();
	}
}

void dovo_mainFrame::OnCopySeriesUID(wxCommandEvent& event)
{
	long item3 = m_series->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item3 == -1)
	{
		wxMessageBox(_("Please select a Series."), _("Error"), wxOK, this);
		return;
	}

	wxString seriesuid = m_series->GetItemText(item3, 1);

	if (wxTheClipboard->Open())
	{
		wxTheClipboard->Clear();
		wxTheClipboard->SetData(new wxTextDataObject(seriesuid));
		wxTheClipboard->Flush();
		wxTheClipboard->Close();
	}
}

void dovo_mainFrame::OnCopySOPUID(wxCommandEvent& event)
{
	long item4 = m_instances->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item4 == -1)
	{
		wxMessageBox(_("Please select an instance."), _("Error"), wxOK, this);
		return;
	}

	wxString sopuid = m_instances->GetItemText(item4, 0);

	if (wxTheClipboard->Open())
	{
		wxTheClipboard->Clear();
		wxTheClipboard->SetData(new wxTextDataObject(sopuid));
		wxTheClipboard->Flush();
		wxTheClipboard->Close();
	}
}