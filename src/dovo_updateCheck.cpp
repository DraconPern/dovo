#include "dovo_updateCheck.h"

dovo_updateCheck::dovo_updateCheck( wxWindow* parent )
:
updateCheck( parent )
{

}

void dovo_updateCheck::updateCheckOnInitDialog( wxInitDialogEvent& event )
{
	if(m_updatetext.length() > 0)
		m_needtoupgrade->SetLabel(m_updatetext);

	m_remoteversion->SetLabel(m_version);
	m_remotemessage->SetLabel(m_message);
}
