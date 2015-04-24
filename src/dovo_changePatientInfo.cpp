#include "dovo_changePatientInfo.h"

dovo_changePatientInfo::dovo_changePatientInfo( wxWindow* parent )
	:
	changePatientInfo( parent )
{

}

void dovo_changePatientInfo::OnInitDialog( wxInitDialogEvent& event )
{
	m_patientNameCtrl->SetValue(m_patientName);
	m_patientIDCtrl->SetValue(m_patientID);
	m_birthdayCtrl->SetValue(m_birthday);
}

void dovo_changePatientInfo::OnOK( wxCommandEvent& event )
{
	m_patientName = m_patientNameCtrl->GetValue();
	m_patientID = m_patientIDCtrl->GetValue();
	m_birthday = m_birthdayCtrl->GetValue();

	event.Skip();
}


