// wxWidgets "Hello world" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "dovo_mainFrame.h"

class MyApp: public wxApp
{
public:
	virtual bool OnInit();	

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
	wxTheApp->SetAppName("fmdeye");
	wxTheApp->SetVendorName("FrontMotion");

	wxConfig::Get()->SetAppName("fmdeye");
	wxConfig::Get()->SetVendorName("FrontMotion");

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

void MyApp::OnAbout(wxCommandEvent& evt)
{
    dovo_about dlg(NULL);
	dlg.ShowModal();
}