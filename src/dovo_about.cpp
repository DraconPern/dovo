#include "dovo_about.h"

dovo_about::dovo_about( wxWindow* parent )
:
about( parent )
{
	wxString date = __DATE__;
	wxString time = __TIME__;
	m_version->SetLabel(wxString("Version ") + __DATE__);
}
