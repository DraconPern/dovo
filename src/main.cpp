// wxWidgets "Hello world" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "dovo_mainFrame.h"
#include "update.h"

// Visual Leak Detector
#if defined(_WIN32) && defined(_DEBUG) && _MSC_VER < 1920
#include <vld.h>
#endif

#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include "dovo_about.h"
#include "dcm2img.h"
#include <boost/asio/ssl.hpp>

class MyApp: public wxApp
{
public:
	virtual bool OnInit();
	virtual int OnExit();
	virtual int OnRun();

public:
    DECLARE_EVENT_TABLE()
    void OnAbout(wxCommandEvent& evt);

	boost::thread updater;
	bool m_shouldExit = false;
};

wxIMPLEMENT_APP(MyApp);

BEGIN_EVENT_TABLE(MyApp, wxApp)
EVT_MENU(wxID_ABOUT, MyApp::OnAbout)
END_EVENT_TABLE()

bool MyApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	wxTheApp->SetAppName("dovo");
	wxTheApp->SetVendorName("FrontMotion");

	wxConfig::Get()->SetAppName("dovo");
	wxConfig::Get()->SetVendorName("FrontMotion");

	boost::filesystem::path::codecvt();  // ensure VC++ does not race during initialization.
	boost::asio::ssl::detail::openssl_init<> _openssl_init;

	// check for update in background and save result for next run
	updater = boost::thread(&updateChecker);

	// see if there's a new version. Note that we just look at what we downloaded on a previous run
	wxString json = wxConfig::Get()->Read("/Settings/UpdateInfo");

	m_shouldExit = false;
	if(informUserOfUpdate(json.ToUTF8().data()))
	{
		// we need to exit...
		m_shouldExit = true;
		return true;
	}

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

#ifdef _WIN32
	frame->SetIcon(wxICON(aaaaaaaa));
#endif

	frame->Show( true );
	SetTopWindow(frame);

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

int MyApp::OnRun()
{
	if(!m_shouldExit)	// run normal code if we aren't exiting immediatly
        wxApp::OnRun();

    return m_shouldExit;
}
