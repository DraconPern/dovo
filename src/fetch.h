#include <string>
#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>

namespace http = boost::beast::http;

typedef struct FetchOptions {
    std::string useragent = "dovo/" DOVO_VERSION;
    std::string token;
    int retry = 20;
    http::verb method = http::verb::get;
    const std::string body;
} FetchOptions;

std::string fetch_string_withretry(std::string uri, const FetchOptions& options, std::function< bool() > shouldstop = NULL);
bool fetch_file_withretry(boost::filesystem::path path, std::string uri, const FetchOptions& options, std::function< bool() > shouldstop = NULL);

