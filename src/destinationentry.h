#ifndef _DESTINATIONENTRY_
#define _DESTINATIONENTRY_

#include <string>

class DestinationEntry
{
public:
	DestinationEntry() {};

	DestinationEntry(std::string name, std::string destinationhost, int destinationport, std::string destinationaetitle, std::string ouraetitle)
	{
		this->name = name;		
		this->destinationHost = destinationhost;
		this->destinationPort = destinationport;
		this->destinationAETitle = destinationaetitle;
		this->ourAETitle = ouraetitle;
	}

	std::string name;
	std::string destinationHost;
	int destinationPort;
	std::string destinationAETitle;
	std::string ourAETitle;
};

#endif