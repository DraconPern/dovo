#include <string>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_escape_char.hpp>

class DestinationEntry
{
public:
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