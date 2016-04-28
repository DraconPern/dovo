///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun  5 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "gui.h"

///////////////////////////////////////////////////////////////////////////

destination::destination( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer23;
	bSizer23 = new wxBoxSizer( wxVERTICAL );
	
	bSizer23->SetMinSize( wxSize( 469,555 ) ); 
	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxHORIZONTAL );
	
	m_destinationList = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL );
	m_destinationList->SetMinSize( wxSize( 5,5 ) );
	
	bSizer24->Add( m_destinationList, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer30;
	bSizer30 = new wxBoxSizer( wxVERTICAL );
	
	m_add = new wxButton( this, wxID_ANY, _("Add"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer30->Add( m_add, 0, wxALL, 5 );
	
	m_delete = new wxButton( this, wxID_ANY, _("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer30->Add( m_delete, 0, wxALL, 5 );
	
	m_echo = new wxButton( this, wxID_ANY, _("Echo"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer30->Add( m_echo, 0, wxALL, 5 );
	
	
	bSizer24->Add( bSizer30, 0, wxEXPAND, 5 );
	
	
	bSizer23->Add( bSizer24, 1, wxEXPAND, 5 );
	
	
	bSizer23->Add( 0, 5, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer2->AddGrowableCol( 1 );
	fgSizer2->SetFlexibleDirection( wxVERTICAL );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText3 = new wxStaticText( this, wxID_ANY, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	fgSizer2->Add( m_staticText3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer31;
	bSizer31 = new wxBoxSizer( wxHORIZONTAL );
	
	m_name = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer31->Add( m_name, 1, wxALL, 5 );
	
	
	fgSizer2->Add( bSizer31, 1, wxEXPAND, 5 );
	
	m_staticText4 = new wxStaticText( this, wxID_ANY, _("Destination Host:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	fgSizer2->Add( m_staticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer32;
	bSizer32 = new wxBoxSizer( wxHORIZONTAL );
	
	m_destinationHost = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer32->Add( m_destinationHost, 1, wxALL, 5 );
	
	
	fgSizer2->Add( bSizer32, 1, wxEXPAND, 5 );
	
	m_staticText5 = new wxStaticText( this, wxID_ANY, _("Destination Port:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText5->Wrap( -1 );
	fgSizer2->Add( m_staticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer29;
	bSizer29 = new wxBoxSizer( wxHORIZONTAL );
	
	m_destinationPort = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer29->Add( m_destinationPort, 1, wxALL, 5 );
	
	
	fgSizer2->Add( bSizer29, 1, wxEXPAND, 5 );
	
	m_staticText6 = new wxStaticText( this, wxID_ANY, _("Destination AE Title:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->Wrap( -1 );
	fgSizer2->Add( m_staticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer33;
	bSizer33 = new wxBoxSizer( wxHORIZONTAL );
	
	m_destinationAETitle = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer33->Add( m_destinationAETitle, 1, wxALL, 5 );
	
	
	fgSizer2->Add( bSizer33, 1, wxEXPAND, 5 );
	
	m_staticText7 = new wxStaticText( this, wxID_ANY, _("Our AE Title:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText7->Wrap( -1 );
	fgSizer2->Add( m_staticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer25;
	bSizer25 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ourAETitle = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer25->Add( m_ourAETitle, 1, wxALL, 5 );
	
	
	fgSizer2->Add( bSizer25, 1, wxEXPAND, 5 );
	
	
	bSizer23->Add( fgSizer2, 0, wxEXPAND, 5 );
	
	
	bSizer23->Add( 0, 5, 0, wxEXPAND, 5 );
	
	m_sdbSizer2 = new wxStdDialogButtonSizer();
	m_sdbSizer2OK = new wxButton( this, wxID_OK );
	m_sdbSizer2->AddButton( m_sdbSizer2OK );
	m_sdbSizer2Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer2->AddButton( m_sdbSizer2Cancel );
	m_sdbSizer2->Realize();
	
	bSizer23->Add( m_sdbSizer2, 0, wxALIGN_LEFT|wxALIGN_TOP|wxBOTTOM|wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer23 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( destination::OnInitDialog ) );
	m_destinationList->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler( destination::OnDeselected ), NULL, this );
	m_destinationList->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( destination::OnSelect ), NULL, this );
	m_add->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( destination::OnAdd ), NULL, this );
	m_delete->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( destination::OnDelete ), NULL, this );
	m_echo->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( destination::OnEcho ), NULL, this );
	m_name->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( destination::OnNameText ), NULL, this );
	m_destinationHost->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( destination::OnDestinationHostText ), NULL, this );
	m_destinationPort->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( destination::OnDestinationPortText ), NULL, this );
	m_destinationAETitle->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( destination::OnDestinationAETitleText ), NULL, this );
	m_ourAETitle->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( destination::OnOurAETitleText ), NULL, this );
	m_sdbSizer2OK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( destination::OnOK ), NULL, this );
}

destination::~destination()
{
	// Disconnect Events
	this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( destination::OnInitDialog ) );
	m_destinationList->Disconnect( wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler( destination::OnDeselected ), NULL, this );
	m_destinationList->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( destination::OnSelect ), NULL, this );
	m_add->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( destination::OnAdd ), NULL, this );
	m_delete->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( destination::OnDelete ), NULL, this );
	m_echo->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( destination::OnEcho ), NULL, this );
	m_name->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( destination::OnNameText ), NULL, this );
	m_destinationHost->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( destination::OnDestinationHostText ), NULL, this );
	m_destinationPort->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( destination::OnDestinationPortText ), NULL, this );
	m_destinationAETitle->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( destination::OnDestinationAETitleText ), NULL, this );
	m_ourAETitle->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( destination::OnOurAETitleText ), NULL, this );
	m_sdbSizer2OK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( destination::OnOK ), NULL, this );
	
}

mainFrame::mainFrame( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	
	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer16;
	bSizer16 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 2, 3, 0, 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_NONE );
	
	m_staticText1 = new wxStaticText( this, wxID_ANY, _("Directory:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	fgSizer1->Add( m_staticText1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_directory = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_directory, 1, wxALL|wxEXPAND, 5 );
	
	m_button5 = new wxButton( this, wxID_ANY, _("Browse..."), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_button5, 0, wxALL, 5 );
	
	m_staticText2 = new wxStaticText( this, wxID_ANY, _("Destination:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	fgSizer1->Add( m_staticText2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_destination = new wxComboBox( this, wxID_ANY, _("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN|wxCB_READONLY ); 
	fgSizer1->Add( m_destination, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
	
	m_button6 = new wxButton( this, wxID_ANY, _("Edit..."), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_button6, 0, wxALL, 5 );
	
	
	bSizer16->Add( fgSizer1, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxHORIZONTAL );
	
	bSizer13->SetMinSize( wxSize( 100,100 ) ); 
	m_patients = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL );
	m_patients->SetMinSize( wxSize( 200,150 ) );
	
	bSizer13->Add( m_patients, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer( wxVERTICAL );
	
	m_studies = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL );
	m_studies->SetMinSize( wxSize( -1,150 ) );
	
	bSizer15->Add( m_studies, 1, wxALL|wxEXPAND, 5 );
	
	m_series = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL );
	m_series->SetMinSize( wxSize( -1,150 ) );
	
	bSizer15->Add( m_series, 1, wxALL|wxEXPAND, 5 );
	
	
	bSizer13->Add( bSizer15, 2, wxEXPAND, 5 );
	
	
	bSizer16->Add( bSizer13, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer141;
	bSizer141 = new wxBoxSizer( wxHORIZONTAL );
	
	bSizer141->SetMinSize( wxSize( 100,100 ) ); 
	wxBoxSizer* bSizer30;
	bSizer30 = new wxBoxSizer( wxVERTICAL );
	
	m_preview = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	m_preview->SetMinSize( wxSize( 200,-1 ) );
	
	bSizer30->Add( m_preview, 1, wxALL|wxEXPAND, 5 );
	
	
	bSizer141->Add( bSizer30, 1, wxEXPAND, 5 );
	
	m_instances = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL );
	m_instances->SetMinSize( wxSize( -1,200 ) );
	
	bSizer141->Add( m_instances, 2, wxALL|wxEXPAND, 5 );
	
	
	bSizer16->Add( bSizer141, 2, wxEXPAND, 5 );
	
	
	bSizer14->Add( bSizer16, 1, wxBOTTOM|wxEXPAND|wxLEFT|wxTOP, 5 );
	
	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer161;
	bSizer161 = new wxBoxSizer( wxVERTICAL );
	
	m_update = new wxButton( this, wxID_ANY, _("Update"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer161->Add( m_update, 0, wxALL, 5 );
	
	m_send = new wxButton( this, wxID_ANY, _("Send"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer161->Add( m_send, 0, wxALL, 5 );
	
	
	bSizer17->Add( bSizer161, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer191;
	bSizer191 = new wxBoxSizer( wxVERTICAL );
	
	m_panel3 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	bSizer191->Add( m_panel3, 1, wxEXPAND | wxALL, 5 );
	
	
	bSizer17->Add( bSizer191, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer181;
	bSizer181 = new wxBoxSizer( wxVERTICAL );
	
	m_about = new wxButton( this, wxID_ANY, _("About"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer181->Add( m_about, 0, wxALL, 5 );
	
	m_exit = new wxButton( this, wxID_ANY, _("Exit"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer181->Add( m_exit, 0, wxALL|wxBOTTOM, 5 );
	
	
	bSizer17->Add( bSizer181, 0, wxEXPAND, 5 );
	
	
	bSizer14->Add( bSizer17, 0, wxBOTTOM|wxEXPAND|wxRIGHT|wxTOP, 5 );
	
	
	this->SetSizer( bSizer14 );
	this->Layout();
	bSizer14->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_button5->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnBrowse ), NULL, this );
	m_button6->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnDestinationEdit ), NULL, this );
	m_patients->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( mainFrame::OnPatientsSelected ), NULL, this );
	m_studies->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( mainFrame::OnStudiesSelected ), NULL, this );
	m_series->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( mainFrame::OnSeriesSelected ), NULL, this );
	m_preview->Connect( wxEVT_PAINT, wxPaintEventHandler( mainFrame::OnPaintPreview ), NULL, this );
	m_instances->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( mainFrame::OnInstancesSelected ), NULL, this );
	m_update->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnUpdate ), NULL, this );
	m_send->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnSend ), NULL, this );
	m_about->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnAbout ), NULL, this );
	m_exit->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnExit ), NULL, this );
}

mainFrame::~mainFrame()
{
	// Disconnect Events
	m_button5->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnBrowse ), NULL, this );
	m_button6->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnDestinationEdit ), NULL, this );
	m_patients->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( mainFrame::OnPatientsSelected ), NULL, this );
	m_studies->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( mainFrame::OnStudiesSelected ), NULL, this );
	m_series->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( mainFrame::OnSeriesSelected ), NULL, this );
	m_preview->Disconnect( wxEVT_PAINT, wxPaintEventHandler( mainFrame::OnPaintPreview ), NULL, this );
	m_instances->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( mainFrame::OnInstancesSelected ), NULL, this );
	m_update->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnUpdate ), NULL, this );
	m_send->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnSend ), NULL, this );
	m_about->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnAbout ), NULL, this );
	m_exit->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnExit ), NULL, this );
	
}

about::about( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer28;
	bSizer28 = new wxBoxSizer( wxVERTICAL );
	
	m_version = new wxStaticText( this, wxID_ANY, _("Version"), wxDefaultPosition, wxDefaultSize, 0 );
	m_version->Wrap( -1 );
	bSizer28->Add( m_version, 0, wxALL, 5 );
	
	m_buildinfo = new wxStaticText( this, wxID_ANY, _("Build Info"), wxDefaultPosition, wxDefaultSize, 0 );
	m_buildinfo->Wrap( -1 );
	bSizer28->Add( m_buildinfo, 0, wxALL, 5 );
	
	m_copyright = new wxStaticText( this, wxID_ANY, _("Copyright (C) 2007-2016 Ing-Long Eric Kuo"), wxDefaultPosition, wxDefaultSize, 0 );
	m_copyright->Wrap( -1 );
	bSizer28->Add( m_copyright, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer27;
	bSizer27 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText13 = new wxStaticText( this, wxID_ANY, _("Homepage:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText13->Wrap( -1 );
	bSizer27->Add( m_staticText13, 0, wxALL, 5 );
	
	m_hyperlink1 = new wxHyperlinkCtrl( this, wxID_ANY, wxEmptyString, wxT("http://www.draconpern.com/software/dovo"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
	bSizer27->Add( m_hyperlink1, 0, wxALL, 5 );
	
	
	bSizer28->Add( bSizer27, 1, wxEXPAND, 5 );
	
	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton( this, wxID_OK );
	m_sdbSizer1->AddButton( m_sdbSizer1OK );
	m_sdbSizer1->Realize();
	
	bSizer28->Add( m_sdbSizer1, 0, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer28 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

about::~about()
{
}

changePatientInfo::changePatientInfo( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );
	
	wxBoxSizer* bSizer23;
	bSizer23 = new wxBoxSizer( wxVERTICAL );
	
	bSizer23->SetMinSize( wxSize( 353,243 ) ); 
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer2->AddGrowableCol( 1 );
	fgSizer2->SetFlexibleDirection( wxVERTICAL );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText3 = new wxStaticText( this, wxID_ANY, _("Patient ID:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	fgSizer2->Add( m_staticText3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer261;
	bSizer261 = new wxBoxSizer( wxHORIZONTAL );
	
	m_patientIDCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer261->Add( m_patientIDCtrl, 1, wxALL, 5 );
	
	
	fgSizer2->Add( bSizer261, 1, wxEXPAND, 5 );
	
	m_staticText4 = new wxStaticText( this, wxID_ANY, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	fgSizer2->Add( m_staticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer271;
	bSizer271 = new wxBoxSizer( wxHORIZONTAL );
	
	m_patientNameCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer271->Add( m_patientNameCtrl, 1, wxALL, 5 );
	
	
	fgSizer2->Add( bSizer271, 1, wxEXPAND, 5 );
	
	m_staticText5 = new wxStaticText( this, wxID_ANY, _("Birthday:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText5->Wrap( -1 );
	fgSizer2->Add( m_staticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer29;
	bSizer29 = new wxBoxSizer( wxHORIZONTAL );
	
	m_birthdayCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer29->Add( m_birthdayCtrl, 1, wxALL, 5 );
	
	
	fgSizer2->Add( bSizer29, 1, wxEXPAND, 5 );
	
	
	bSizer23->Add( fgSizer2, 1, wxALL|wxEXPAND, 5 );
	
	
	bSizer23->Add( 0, 5, 0, wxEXPAND, 5 );
	
	m_enableEdit = new wxCheckBox( this, wxID_ANY, _("Change Patient Info"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer23->Add( m_enableEdit, 0, wxALL, 5 );
	
	m_sdbSizer3 = new wxStdDialogButtonSizer();
	m_sdbSizer3OK = new wxButton( this, wxID_OK );
	m_sdbSizer3->AddButton( m_sdbSizer3OK );
	m_sdbSizer3Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer3->AddButton( m_sdbSizer3Cancel );
	m_sdbSizer3->Realize();
	
	bSizer23->Add( m_sdbSizer3, 0, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer23 );
	this->Layout();
	bSizer23->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( changePatientInfo::OnInitDialog ) );
	m_enableEdit->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( changePatientInfo::m_enableEditOnCheckBox ), NULL, this );
	m_sdbSizer3OK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( changePatientInfo::OnOK ), NULL, this );
}

changePatientInfo::~changePatientInfo()
{
	// Disconnect Events
	this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( changePatientInfo::OnInitDialog ) );
	m_enableEdit->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( changePatientInfo::m_enableEditOnCheckBox ), NULL, this );
	m_sdbSizer3OK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( changePatientInfo::OnOK ), NULL, this );
	
}

sendStatus::sendStatus( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 600,400 ), wxDefaultSize );
	
	wxBoxSizer* bSizer30;
	bSizer30 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer29;
	bSizer29 = new wxBoxSizer( wxHORIZONTAL );
	
	m_progress = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	m_progress->SetValue( 0 ); 
	bSizer29->Add( m_progress, 1, wxBOTTOM|wxEXPAND|wxLEFT|wxTOP, 15 );
	
	m_stop = new wxButton( this, wxID_ANY, _("Stop"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer29->Add( m_stop, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxTOP, 15 );
	
	
	bSizer30->Add( bSizer29, 0, wxEXPAND, 5 );
	
	m_log = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	bSizer30->Add( m_log, 1, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer30 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_stop->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( sendStatus::OnStop ), NULL, this );
}

sendStatus::~sendStatus()
{
	// Disconnect Events
	m_stop->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( sendStatus::OnStop ), NULL, this );
	
}

searchStatus::searchStatus( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );
	
	wxBoxSizer* bSizer28;
	bSizer28 = new wxBoxSizer( wxHORIZONTAL );
	
	m_progress = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL|wxGA_SMOOTH );
	m_progress->SetValue( 0 ); 
	bSizer28->Add( m_progress, 1, wxBOTTOM|wxEXPAND|wxLEFT|wxTOP, 15 );
	
	m_stop = new wxButton( this, wxID_ANY, _("Stop"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer28->Add( m_stop, 0, wxALIGN_CENTER_VERTICAL|wxALL, 15 );
	
	
	this->SetSizer( bSizer28 );
	this->Layout();
	bSizer28->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_stop->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( searchStatus::OnStop ), NULL, this );
}

searchStatus::~searchStatus()
{
	// Disconnect Events
	m_stop->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( searchStatus::OnStop ), NULL, this );
	
}

updateCheck::updateCheck( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer28;
	bSizer28 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer29;
	bSizer29 = new wxBoxSizer( wxVERTICAL );
	
	m_needtoupgrade = new wxStaticText( this, wxID_ANY, _("A new version of dovo is available:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_needtoupgrade->Wrap( -1 );
	bSizer29->Add( m_needtoupgrade, 0, wxALL, 5 );
	
	m_remoteversion = new wxStaticText( this, wxID_ANY, _("x.x.x.x"), wxDefaultPosition, wxDefaultSize, 0 );
	m_remoteversion->Wrap( -1 );
	bSizer29->Add( m_remoteversion, 0, wxALL, 5 );
	
	m_remotemessage = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	bSizer29->Add( m_remotemessage, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer27;
	bSizer27 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText13 = new wxStaticText( this, wxID_ANY, _("Download at:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText13->Wrap( -1 );
	bSizer27->Add( m_staticText13, 0, wxALL, 5 );
	
	m_hyperlink11 = new wxHyperlinkCtrl( this, wxID_ANY, wxEmptyString, wxT("http://www.draconpern.com/software/dovo"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
	bSizer27->Add( m_hyperlink11, 0, wxALL, 5 );
	
	
	bSizer29->Add( bSizer27, 0, wxEXPAND, 5 );
	
	
	bSizer28->Add( bSizer29, 1, wxEXPAND, 5 );
	
	m_sdbSizer4 = new wxStdDialogButtonSizer();
	m_sdbSizer4OK = new wxButton( this, wxID_OK );
	m_sdbSizer4->AddButton( m_sdbSizer4OK );
	m_sdbSizer4->Realize();
	
	bSizer28->Add( m_sdbSizer4, 0, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer28 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( updateCheck::updateCheckOnInitDialog ) );
}

updateCheck::~updateCheck()
{
	// Disconnect Events
	this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( updateCheck::updateCheckOnInitDialog ) );
	
}
