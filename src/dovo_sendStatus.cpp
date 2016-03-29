#include "dovo_sendStatus.h"

dovo_sendStatus::dovo_sendStatus( wxWindow* parent )
	:
	sendStatus( parent )
{
	timer.Connect(wxEVT_TIMER, wxTimerEventHandler( dovo_sendStatus::OnTimer ), NULL, this );
	timer.Start(200);	
}

void dovo_sendStatus::OnStop( wxCommandEvent& event )
{	
	m_sender->Cancel();

	if(m_sender->IsDone())
		EndModal(0);
}


dovo_sendStatus::~dovo_sendStatus()
{
	timer.Disconnect(wxEVT_TIMER, wxTimerEventHandler( dovo_sendStatus::OnTimer ), NULL, this );
}

void dovo_sendStatus::OnTimer( wxTimerEvent& event )
{
	// update cout => log window		
	std::string str = m_sender->ReadLog();

	if(str.length() != 0)
	{
		m_log->AppendText(wxString::FromUTF8(str.c_str()));
	}

	if(m_sender->IsDone())
	{				
		timer.Disconnect(wxEVT_TIMER, wxTimerEventHandler( dovo_sendStatus::OnTimer ), NULL, this );
		m_stop->SetLabel(_("Close"));
		m_log->AppendText("\nDone\n");

		m_progress->SetValue(100);
		/*
		if(m_sender->IsCanceled())
			EndDialog(0);*/
	}
	else
	{
		m_progress->Pulse();
	}
}