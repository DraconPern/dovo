#include "dovo_destination.h"

dovo_destination::dovo_destination( wxWindow* parent )
	:
	destination( parent )
{
	invalidInput = ",";
	wxTextValidator textval(wxFILTER_EMPTY | wxFILTER_EXCLUDE_CHAR_LIST);
	// seems like putting it in the ctor doesn't work
	textval.SetCharExcludes(invalidInput);

	m_name->SetValidator(textval);
	m_destinationHost->SetValidator(textval);
	m_destinationPort->SetValidator(wxIntegerValidator<int>());
	m_destinationAETitle->SetValidator(textval);
	m_destinationAETitle->SetMaxLength(16);
	m_ourAETitle->SetValidator(textval);
	m_ourAETitle->SetMaxLength(16);
}

void dovo_destination::OnInitDialog( wxInitDialogEvent& event )
{
	m_destinationList->InsertColumn(0, _("Name"), wxLIST_FORMAT_LEFT);
	for(unsigned int i = 0; i < m_destinations.size(); i++)
		m_destinationList->InsertItem(i, wxString::FromUTF8(m_destinations[i].name.c_str()));

	m_destinationList->SetColumnWidth(0, m_destinationList->GetSize().GetWidth());

	if(m_destinations.size() > 0)
		m_destinationList->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	SetCtrlState();
}

void dovo_destination::OnDeselected( wxListEvent& event )
{
	// UpdateItem(event.m_itemIndex);
}

void dovo_destination::OnSelect( wxListEvent& event )
{
	SetCtrlState();
}

void dovo_destination::OnAdd( wxCommandEvent& event )
{
	// save the old one
	UpdateItem();

	m_destinations.push_back(DestinationEntry("[New Name 1]", "localhost", 104, "SCP", "FRONTMOTION"));

	unsigned int i = m_destinations.size() - 1;
	m_destinationList->InsertItem(i, wxString::FromUTF8(m_destinations[i].name.c_str()));
	m_destinationList->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

	SetCtrlState();

	m_name->SetFocus();
	m_name->SetSelection(-1, -1);
}

void dovo_destination::OnDelete( wxCommandEvent& event )
{
	int sel = GetSelectedDestinationItem();
	if ( sel == -1 )
		return;

	// delete first, because OnDeselected is triggered
	m_destinationList->DeleteItem(sel);

	m_destinations.erase(m_destinations.begin() + sel);

	SetCtrlState();
}

void dovo_destination::OnEcho( wxCommandEvent& event )
{	
	int sel = GetSelectedDestinationItem();
	if ( sel == -1 )
		return;
	
	wxBusyCursor wait;
	if(DICOMSender::Echo(m_destinations[sel]))
	{
		wxMessageBox(_("Success"), _("Echo"), wxOK, this);		
	}
	else
	{
		wxMessageBox(_("Failed"), _("Echo"), wxOK, this);
	}

}

void dovo_destination::OnNameText( wxCommandEvent& event )
{
	UpdateItem();

	// update the list
	int sel = GetSelectedDestinationItem();
	if(sel != -1)
	{
		m_destinationList->SetItemText(sel, m_destinations[sel].name);
	}
}

void dovo_destination::OnDestinationHostText( wxCommandEvent& event )
{
	UpdateItem();
}

void dovo_destination::OnDestinationPortText( wxCommandEvent& event )
{
	UpdateItem();
}

void dovo_destination::OnDestinationAETitleText( wxCommandEvent& event )
{
	UpdateItem();
}

void dovo_destination::OnOurAETitleText( wxCommandEvent& event )
{
	UpdateItem();
}

void dovo_destination::OnOK( wxCommandEvent& event )
{

	event.Skip();
}


void dovo_destination::UpdateItem(int sel)
{
	TransferDataFromWindow();	

	if(sel == -1)
	{		
		sel = GetSelectedDestinationItem(); 
		if ( sel == -1 )
			return;
	}

	m_destinations[sel].name = m_name->GetValue().ToUTF8();
	m_destinations[sel].destinationHost = m_destinationHost->GetValue().ToUTF8();
	try { m_destinations[sel].destinationPort = boost::lexical_cast<int>(m_destinationPort->GetValue()); }
	catch(...) { m_destinations[sel].destinationPort = 104; }
	m_destinations[sel].destinationAETitle = m_destinationAETitle->GetValue().ToUTF8();
	m_destinations[sel].ourAETitle = m_ourAETitle->GetValue().ToUTF8();
}

int dovo_destination::GetSelectedDestinationItem()
{
	long sel = -1;
	return m_destinationList->GetNextItem(sel, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED); 	
}

void  dovo_destination::SetCtrlState()
{
	long sel = GetSelectedDestinationItem();
	if ( sel == -1 )
	{
		m_name->Enable(false);
		m_name->ChangeValue("");
		m_destinationHost->Enable(false);
		m_destinationHost->ChangeValue("");
		m_destinationPort->Enable(false);
		m_destinationPort->ChangeValue("");
		m_destinationAETitle->Enable(false);
		m_destinationAETitle->ChangeValue("");
		m_ourAETitle->Enable(false);
		m_ourAETitle->ChangeValue("");
	}
	else
	{		
		m_name->Enable(true);
		m_name->ChangeValue(wxString::FromUTF8(m_destinations[sel].name.c_str()));
		m_destinationHost->Enable(true);
		m_destinationHost->ChangeValue(wxString::FromUTF8(m_destinations[sel].destinationHost.c_str()));
		m_destinationPort->Enable(true);
		m_destinationPort->ChangeValue(boost::lexical_cast<std::string>(m_destinations[sel].destinationPort));
		m_destinationAETitle->Enable(true);
		m_destinationAETitle->ChangeValue(wxString::FromUTF8(m_destinations[sel].destinationAETitle.c_str()));
		m_ourAETitle->Enable(true);
		m_ourAETitle->ChangeValue(wxString::FromUTF8(m_destinations[sel].ourAETitle.c_str()));
	}
}