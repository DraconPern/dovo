#ifndef _DESTINATIONENTRY_
#define _DESTINATIONENTRY_

class DestinationEntry
{
public:
	DestinationEntry(wxString name, wxString destinationhost, int destinationport, wxString destinationaetitle, wxString ouraetitle)
	{
		this->name = name;		
		this->destinationHost = destinationhost;
		this->destinationPort = destinationport;
		this->destinationAETitle = destinationaetitle;
		this->ourAETitle = ouraetitle;
	}

	wxString name;	
	wxString destinationHost;
	int destinationPort;
	wxString destinationAETitle;
	wxString ourAETitle;
};

#endif