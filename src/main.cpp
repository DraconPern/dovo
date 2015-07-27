// wxWidgets "Hello world" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "dovo_mainFrame.h"
#include "update.h"

// Visual Leak Detector
#ifdef _WIN32
#include <vld.h>
#endif 

#include <boost/thread.hpp>


class MyApp: public wxApp
{
public:
	virtual bool OnInit();	
	virtual int OnExit();

public:
    DECLARE_EVENT_TABLE() 
    void OnAbout(wxCommandEvent& evt);
	boost::thread updater;
};

wxIMPLEMENT_APP(MyApp);

BEGIN_EVENT_TABLE(MyApp, wxApp)
	EVT_MENU(wxID_ABOUT, MyApp::OnAbout)
	END_EVENT_TABLE()

	bool MyApp::OnInit()
{	
	wxTheApp->SetAppName("dovo");
	wxTheApp->SetVendorName("FrontMotion");

	wxConfig::Get()->SetAppName("dovo");
	wxConfig::Get()->SetVendorName("FrontMotion");

	// see if there's a new version. Note that we just look at what we downloaded on a previous run
	wxString json = wxConfig::Get()->Read("/Settings/UpdateInfo");

	if(informUserOfUpdate(json.ToUTF8().data()))
	{
		// we need to exit...
		// get update w/o threading
		updateChecker();
		return true;
	}

	// check for update	for next run
	updater = boost::thread(&updateChecker);


	/* 
	Update notice, message
	Exit notice, message

	*/

	RegisterCodecs();

	dovo_mainFrame *frame = new dovo_mainFrame(NULL);

#ifdef __WXMAC__	
	wxMenuBar* menubar = new wxMenuBar();
	wxMenu* menu = new  wxMenu();

	menu->Append(wxID_ABOUT, _("About")); 
	menu->Append(wxID_EXIT, _("Exit"));

	menubar->Append(menu, _("File"));

	frame->SetMenuBar(menubar);
#endif

	frame->Show( true );	

	return true;
}

int MyApp::OnExit()
{
	DeregisterCodecs();

	updater.join();
	return wxApp::OnExit();
}

void MyApp::OnAbout(wxCommandEvent& evt)
{
	dovo_about dlg(NULL);
	dlg.ShowModal();
}
