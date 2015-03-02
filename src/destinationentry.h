#include <string>
/*#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_escape_char.hpp>
using namespace boost::spirit::classic;
*/
class DestinationEntry
{
public:
	DestinationEntry(std::string name, std::string destinationhost, int destinationport, std::string destinationaetitle, std::string ouraetitle)
	{
		this->Name = name;		
		this->DestinationHost = destinationhost;
		this->DestinationPort = destinationport;
		this->DestinationAETitle = destinationaetitle;
		this->OurAETitle = ouraetitle;
	}

	std::string Name;	
	std::string DestinationHost;
	int DestinationPort;
	std::string DestinationAETitle;
	std::string OurAETitle;
};