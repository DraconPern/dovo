#include "dovo_about.h"

dovo_about::dovo_about( wxWindow* parent )
:
about( parent )
{	
	m_version->SetLabel(wxString("Version ") + DOVO_VERSION);
	m_buildinfo->SetLabel(wxString("Build date: ") + __DATE__);
}
