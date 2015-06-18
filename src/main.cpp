// wxWidgets "Hello world" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "dovo_mainFrame.h"

// Visual Leak Detector
#ifdef _WIN32
#include <vld.h>
#endif 

time_t cvt_TIME(char const *time);

class MyApp: public wxApp
{
public:
	virtual bool OnInit();	
	virtual int OnExit();

public:
    DECLARE_EVENT_TABLE() 
    void OnAbout(wxCommandEvent& evt);    
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

	time_t t = cvt_TIME(__DATE__);

	// check for update
	wxConfig::Get()->SetPath("/Settings");
	wxConfig::Get()->Read("LastUpdateCheck");
	
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
	return wxApp::OnExit();
}

void MyApp::OnAbout(wxCommandEvent& evt)
{
    dovo_about dlg(NULL);
	dlg.ShowModal();
}

time_t cvt_TIME(char const *time) { 
    char s_month[5];
    int month, day, year;
    struct tm t = {0};
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

    sscanf(time, "%s %d %d", s_month, &day, &year);

    month = (strstr(month_names, s_month)-month_names)/3;

    t.tm_mon = month;
    t.tm_mday = day;
    t.tm_year = year - 1900;
    t.tm_isdst = -1;

    return mktime(&t);
}