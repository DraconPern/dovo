#include "dovo_sendStatus.h"

dovo_sendStatus::dovo_sendStatus( wxWindow* parent )
:
sendStatus( parent )
{
	timer.Connect(wxEVT_TIMER, wxTimerEventHandler( dovo_sendStatus::OnTimer ), NULL, this );
	timer.Start(500);
}

void dovo_sendStatus::OnStop( wxCommandEvent& event )
{
	m_sender->Cancel();
}


dovo_sendStatus::~dovo_sendStatus()
{
	timer.Disconnect(wxEVT_TIMER, wxTimerEventHandler( dovo_sendStatus::OnTimer ), NULL, this );
}

void dovo_sendStatus::OnTimer( wxTimerEvent& event )
{
	if(m_sender->IsDone())
	{						
		EndModal(0);
	}
	else
	{
		m_progress->Pulse();
	}
}