#include "dovo_searchStatus.h"

dovo_searchStatus::dovo_searchStatus( wxWindow* parent )
:
searchStatus( parent )
{

#if defined(_WIN32)
	SetMinSize(wxSize(600, 120));
	SetSize(wxSize(600, 120));
#elif defined(__WXMAC__)
	SetMinSize(wxSize(400, 100));
	SetSize(wxSize(400, 100));
#endif
	Center();
	timer.Connect(wxEVT_TIMER, wxTimerEventHandler( dovo_searchStatus::OnTimer ), NULL, this );
	timer.Start(500);
}

void dovo_searchStatus::OnStop( wxCommandEvent& event )
{
	m_scanner->Cancel();
}


dovo_searchStatus::~dovo_searchStatus()
{
	timer.Disconnect(wxEVT_TIMER, wxTimerEventHandler( dovo_searchStatus::OnTimer ), NULL, this );
}

void dovo_searchStatus::OnTimer( wxTimerEvent& event )
{
	if(m_scanner->IsDone())
	{
		timer.Disconnect(wxEVT_TIMER, wxTimerEventHandler( dovo_searchStatus::OnTimer ), NULL, this );
		if(m_scanner->IsCanceled())
			EndModal(IDCANCEL);
		else
			EndModal(IDOK);
	}
	else
	{
		m_progress->Pulse();
	}
}
