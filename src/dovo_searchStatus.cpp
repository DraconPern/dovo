#include "dovo_searchStatus.h"

dovo_searchStatus::dovo_searchStatus( wxWindow* parent )
	:
	searchStatus( parent )
{	
	timer.Connect(wxEVT_TIMER, wxTimerEventHandler( dovo_searchStatus::OnTimer ), NULL, this );
	timer.Start(500);
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
		m_gauge1->Pulse();
	}
}

void dovo_searchStatus::OnStop( wxCommandEvent& event )
{
	m_scanner->Cancel();
}
