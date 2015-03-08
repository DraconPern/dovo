#include "dovo_destination.h"

dovo_destination::dovo_destination( wxWindow* parent )
	:
	destination( parent )
{

}

void dovo_destination::OnInitDialog( wxInitDialogEvent& event )
{
	m_destinationList->InsertColumn(0, "Name", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	for(unsigned int i = 0; i < m_destinations.size(); i++)
		m_destinationList->InsertItem(i, wxString::FromUTF8(m_destinations[i].name.c_str()));

	m_destinationList->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	SetCtrlState();
}

void dovo_destination::OnDeselected( wxListEvent& event )
{
	UpdateItem(event.m_itemIndex);
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

void dovo_destination::OnNameUpdate( wxKeyEvent& event )
{
	int sel = GetSelectedDestinationItem();
	if(sel != -1)
	{
		wxString name = m_name->GetValue();
		if(name.length() > 0) 
		{
			m_destinations[sel].name = name.ToUTF8();

			m_destinationList->DeleteItem(sel);
			m_destinationList->InsertItem(sel, name);
			m_destinationList->SetItemState(sel, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
			event.Skip();
		}
		else
		{
			wxMessageBox( wxT("Name can't be blank!"), "Error", wxOK | wxICON_WARNING);
			m_name->SetValue(wxString::FromUTF8(m_destinations[sel].name.c_str()));
		}

	}
	else
	{
		event.Skip();
	}
}

void dovo_destination::OnOK( wxCommandEvent& event )
{
	UpdateItem();
	event.Skip();
}


void  dovo_destination::SetCtrlState()
{
	long sel = GetSelectedDestinationItem();
	if ( sel == -1 )
	{
		m_name->Enable(false);
		m_name->SetValue("");
		m_destinationHost->Enable(false);
		m_destinationHost->SetValue("");
		m_destinationPort->Enable(false);
		m_destinationPort->SetValue("");
		m_destinationAETitle->Enable(false);
		m_destinationAETitle->SetValue("");
		m_ourAETitle->Enable(false);
		m_ourAETitle->SetValue("");
	}
	else
	{
		TransferDataToWindow();
		m_name->Enable(true);
		m_name->SetValue(wxString::FromUTF8(m_destinations[sel].name.c_str()));
		m_destinationHost->Enable(true);
		m_destinationHost->SetValue(wxString::FromUTF8(m_destinations[sel].destinationHost.c_str()));
		m_destinationPort->Enable(true);
		m_destinationPort->SetValue(boost::lexical_cast<std::string>(m_destinations[sel].destinationPort));
		m_destinationAETitle->Enable(true);
		m_destinationAETitle->SetValue(wxString::FromUTF8(m_destinations[sel].destinationAETitle.c_str()));
		m_ourAETitle->Enable(true);
		m_ourAETitle->SetValue(wxString::FromUTF8(m_destinations[sel].ourAETitle.c_str()));
	}
}

int dovo_destination::GetSelectedDestinationItem()
{
	long sel = -1;
	return m_destinationList->GetNextItem(sel, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED); 	
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
	m_destinations[sel].destinationPort = boost::lexical_cast<int>(m_port);
	m_destinations[sel].destinationAETitle = m_destinationAETitle->GetValue().ToUTF8();
	m_destinations[sel].ourAETitle = m_ourAETitle->GetValue().ToUTF8();
}