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
};

wxIMPLEMENT_APP(MyApp);
bool MyApp::OnInit()
{
	dovo_mainFrame *frame = new dovo_mainFrame(NULL);
	frame->Show( true );
	return true;
}