#include "dovo_searchStatus.h"

dovo_searchStatus::dovo_searchStatus( wxWindow* parent )
	:
	searchStatus( parent )
{	
#if defined(_WIN32)
	SetMinSize(wxSize(600, 150));
	SetSize(wxSize(600, 150));
#elif defined(__WXMAC__)
	SetMinSize(wxSize(600, 150));
	SetSize(wxSize(600, 150));	
#endif 

	timer.Connect(wxEVT_TIMER, wxTimerEventHandler( dovo_searchStatus::OnTimer ), NULL, this );
	timer.Start(500);
	m_progress->SetRange(200);

}

dovo_searchStatus::~dovo_searchStatus()
{
	timer.Disconnect(wxEVT_TIMER, wxTimerEventHandler( dovo_searchStatus::OnTimer ), NULL, this );
}

void dovo_searchStatus::OnTimer( wxTimerEvent& event )
{
	if(m_scanner->IsDone())
	{						
		EndModal(0);
	}
	else
	{
		m_progress->Pulse();
	}
}

void dovo_searchStatus::OnStop( wxCommandEvent& event )
{
	m_scanner->Cancel();
}
