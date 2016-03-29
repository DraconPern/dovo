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

m_patientNameCtrl->Disable();
m_patientIDCtrl->Disable();
m_birthdayCtrl->Disable();
m_enableEdit->SetValue(false);
}

void dovo_changePatientInfo::m_enableEditOnCheckBox( wxCommandEvent& event )
{
	if(m_enableEdit->GetValue())
	{
		m_patientNameCtrl->Enable();
		m_patientIDCtrl->Enable();
		m_birthdayCtrl->Enable();

		m_enableEdit->Disable();
	}
}

void dovo_changePatientInfo::OnOK( wxCommandEvent& event )
{
	m_patientName = m_patientNameCtrl->GetValue();
	m_patientID = m_patientIDCtrl->GetValue();
	m_birthday = m_birthdayCtrl->GetValue();
	m_changeInfo = m_enableEdit->GetValue();
	event.Skip();
}



