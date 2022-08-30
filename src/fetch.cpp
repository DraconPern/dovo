#include "fetch.h"

#include <boost/beast/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#ifdef _WIN32
#include <cryptuiapi.h>
#elif defined(__APPLE__)
#include <Security/Security.h>
#endif

using boost::asio::ip::tcp;
namespace http = boost::beast::http;

std::string fetch_string(std::string uri, const FetchOptions& options)
{
	boost::asio::io_context io_context;

	std::string protocol = uri.substr(0, 5);
	std::transform(protocol.begin(), protocol.end(), protocol.begin(), [](unsigned char c) { return std::tolower(c); });
	if (protocol != "http:" && protocol != "https")
		throw std::invalid_argument("bad protocol");

	if (protocol == "http:")
		protocol = "http";

	if (uri.substr(protocol.length(), 3) != "://")
		throw std::invalid_argument("bad uri");

	uri = uri.substr(protocol.length() + 3);

	std::string host = uri.substr(0, uri.find('/'));

	std::string path = uri.substr(host.length());

	// Get a list of endpoints corresponding to the server name.
	tcp::resolver resolver(io_context);
	tcp::resolver::results_type endpoint = resolver.resolve(host, protocol);

	http::request<http::string_body> req{ options.method, path, 11 };
	req.set(http::field::host, host);
	req.set(http::field::user_agent, options.useragent);
	if(options.token.length())
		req.set(http::field::authorization, "Bearer " + options.token);

	if(options.body.length())
		req.body() = options.body;

	boost::beast::flat_buffer buffer;
	http::response<http::string_body> res;

	// Try each endpoint until we successfully establish a connection.
	if (protocol == "https")
	{
		boost::asio::ssl::context ssl_context(boost::asio::ssl::context::tlsv12_client);
		ssl_context.set_verify_mode(boost::asio::ssl::verify_peer);
#ifdef _WIN32
		// use windows cert store for openssl
		HCERTSTORE hStore;
		PCCERT_CONTEXT pContext = NULL;
		X509* x509;
		X509_STORE* store = SSL_CTX_get_cert_store(ssl_context.native_handle());

		hStore = CertOpenSystemStore(NULL, L"ROOT");

		if (hStore)
		{
			while (pContext = CertEnumCertificatesInStore(hStore, pContext))
			{
				x509 = NULL;
				const unsigned char* encoded_cert = pContext->pbCertEncoded;
				x509 = d2i_X509(NULL, (const unsigned char**)&encoded_cert, pContext->cbCertEncoded);
				if (x509)
				{					
					int i = X509_STORE_add_cert(store, x509);
					X509_free(x509);
				}
			}

			CertFreeCertificateContext(pContext);
			CertCloseStore(hStore, 0);
		}
#elif defined(__APPLE__)
		CFMutableDictionaryRef search;
		CFArrayRef result;
		SecKeychainRef keychain;
		SecCertificateRef item;
		CFDataRef dat;

		X509* x509;
		X509_STORE* store = SSL_CTX_get_cert_store(ssl_context.native_handle());

		// Load keychain
		if (SecKeychainOpen("/System/Library/Keychains/SystemRootCertificates.keychain", &keychain) == errSecSuccess)
		{
			// Search for certificates
			search = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
			CFDictionarySetValue(search, kSecClass, kSecClassCertificate);
			CFDictionarySetValue(search, kSecMatchLimit, kSecMatchLimitAll);
			CFDictionarySetValue(search, kSecReturnRef, kCFBooleanTrue);
			CFDictionarySetValue(search, kSecMatchSearchList, CFArrayCreate(NULL, (const void**)&keychain, 1, NULL));
			if (SecItemCopyMatching(search, (CFTypeRef*)&result) == errSecSuccess) {
				CFIndex n = CFArrayGetCount(result);
				for (CFIndex i = 0; i < n; i++) {
					item = (SecCertificateRef)CFArrayGetValueAtIndex(result, i);

					// Get certificate in DER format
					dat = SecCertificateCopyData(item);
					x509 = NULL;
					const unsigned char* encoded_cert = CFDataGetBytePtr(dat);
					x509 = d2i_X509(NULL, (const unsigned char**)&encoded_cert, CFDataGetLength(dat));
					if (x509)
					{
						int i = X509_STORE_add_cert(store, x509);
						X509_free(x509);
					}

					CFRelease(dat);
				}
			}
			CFRelease(keychain);
		}
#else
		ssl_context.set_default_verify_paths();
#endif
		boost::beast::ssl_stream<boost::beast::tcp_stream> socket(io_context, ssl_context);
		// Set SNI Hostname (many hosts need this to handshake successfully)
		if (!SSL_set_tlsext_host_name(socket.native_handle(), host.c_str()))
		{
			boost::beast::error_code ec{ static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category() };
			throw boost::beast::system_error{ ec };
		}
		boost::beast::get_lowest_layer(socket).connect(endpoint);
		socket.handshake(boost::asio::ssl::stream_base::client);
		http::write(socket, req);
		http::read(socket, buffer, res);
		boost::system::error_code ec;
		socket.shutdown(ec);
	}
	else
	{
		tcp::socket socket(io_context);
		boost::asio::connect(socket, endpoint);
		http::write(socket, req);
		http::read(socket, buffer, res);
		boost::system::error_code ec;
		socket.shutdown(tcp::socket::shutdown_both, ec);
	}

	if(res.result() != boost::beast::http::status::ok)
		throw std::runtime_error("Bad result from server.");

	return res.body();
}

std::string fetch_string_withretry(const std::string uri, const FetchOptions& options, std::function< bool() > shouldstop)
{
	// retry can be < 0 which means infinte tries'
	int retry = options.retry;
	while (retry != 0)
	{
		try {
			return fetch_string(uri, options);
		}
		catch (std::exception &ec)
		{

		}

		if(shouldstop)
			if (shouldstop())
				return "";

		retry--;
	}

	return "";
}

bool fetch_file(boost::filesystem::path outputpath, std::string uri, const FetchOptions& options)
{
	boost::asio::io_context io_context;

	std::string protocol = uri.substr(0, 5);
	std::transform(protocol.begin(), protocol.end(), protocol.begin(), [](unsigned char c) { return std::tolower(c); });
	if (protocol != "http:" && protocol != "https")
		throw std::invalid_argument("bad protocol");

	if (protocol == "http:")
		protocol = "http";

	if (uri.substr(protocol.length(), 3) != "://")
		throw std::invalid_argument("bad uri");

	uri = uri.substr(protocol.length() + 3);

	std::string host = uri.substr(0, uri.find('/'));

	uri = uri.substr(host.length());

	// Get a list of endpoints corresponding to the server name.
	tcp::resolver resolver(io_context);
	tcp::resolver::results_type endpoint = resolver.resolve(host, protocol);

	http::request<http::string_body> req{ options.method, uri, 11 };
	req.set(http::field::host, host);
	req.set(http::field::user_agent, options.useragent);
	if (options.token.length())
		req.set(http::field::authorization, "Bearer " + options.token);

	if (options.body.length())
		req.body() = options.body;

	boost::beast::flat_buffer buffer;
	http::response_parser<http::file_body> parser;
	parser.body_limit((std::numeric_limits<std::uint64_t>::max)());	
	
	// Try each endpoint until we successfully establish a connection.
	if (protocol == "https")
	{
		boost::asio::ssl::context ssl_context(boost::asio::ssl::context::tlsv12_client);
		ssl_context.set_verify_mode(boost::asio::ssl::verify_peer);
#ifdef _WIN32
		// use windows cert store for openssl
		HCERTSTORE hStore;
		PCCERT_CONTEXT pContext = NULL;
		X509* x509;
		X509_STORE* store = SSL_CTX_get_cert_store(ssl_context.native_handle());

		hStore = CertOpenSystemStore(NULL, L"ROOT");

		if (hStore)
		{
			while (pContext = CertEnumCertificatesInStore(hStore, pContext))
			{
				x509 = NULL;
				const unsigned char* encoded_cert = pContext->pbCertEncoded;
				x509 = d2i_X509(NULL, (const unsigned char**)&encoded_cert, pContext->cbCertEncoded);
				if (x509)
				{
					int i = X509_STORE_add_cert(store, x509);
					X509_free(x509);
				}
			}

			CertFreeCertificateContext(pContext);
			CertCloseStore(hStore, 0);
		}
#elif defined(__APPLE__)
		CFMutableDictionaryRef search;
		CFArrayRef result;
		SecKeychainRef keychain;
		SecCertificateRef item;
		CFDataRef dat;

		X509* x509;
		X509_STORE* store = SSL_CTX_get_cert_store(ssl_context.native_handle());

		// Load keychain
		if (SecKeychainOpen("/System/Library/Keychains/SystemRootCertificates.keychain", &keychain) == errSecSuccess)
		{
			// Search for certificates
			search = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
			CFDictionarySetValue(search, kSecClass, kSecClassCertificate);
			CFDictionarySetValue(search, kSecMatchLimit, kSecMatchLimitAll);
			CFDictionarySetValue(search, kSecReturnRef, kCFBooleanTrue);
			CFDictionarySetValue(search, kSecMatchSearchList, CFArrayCreate(NULL, (const void**)&keychain, 1, NULL));
			if (SecItemCopyMatching(search, (CFTypeRef*)&result) == errSecSuccess) {
				CFIndex n = CFArrayGetCount(result);
				for (CFIndex i = 0; i < n; i++) {
					item = (SecCertificateRef)CFArrayGetValueAtIndex(result, i);

					// Get certificate in DER format
					dat = SecCertificateCopyData(item);
					x509 = NULL;
					const unsigned char* encoded_cert = CFDataGetBytePtr(dat);
					x509 = d2i_X509(NULL, (const unsigned char**)&encoded_cert, CFDataGetLength(dat));
					if (x509)
					{
						int i = X509_STORE_add_cert(store, x509);
						X509_free(x509);
					}

					CFRelease(dat);
				}
			}
			CFRelease(keychain);
		}
#else
		ssl_context.set_default_verify_paths();
#endif
		boost::beast::ssl_stream<boost::beast::tcp_stream> socket(io_context, ssl_context);
		// Set SNI Hostname (many hosts need this to handshake successfully)
		if (!SSL_set_tlsext_host_name(socket.native_handle(), host.c_str()))
		{
			boost::beast::error_code ec{ static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category() };
			throw boost::beast::system_error{ ec };
		}		
		boost::beast::get_lowest_layer(socket).connect(endpoint);
		socket.handshake(boost::asio::ssl::stream_base::client);
		http::write(socket, req);
		boost::system::error_code ec;
		parser.get().body().open(outputpath.string().c_str(), boost::beast::file_mode::write, ec);
		boost::beast::http::read(socket, buffer, parser);
		socket.shutdown();
	}
	else
	{
		tcp::socket socket(io_context);
		boost::asio::connect(socket, endpoint);
		http::write(socket, req);
		boost::system::error_code ec;
		parser.get().body().open(outputpath.string().c_str(), boost::beast::file_mode::write, ec);
		boost::beast::http::read(socket, buffer, parser);
		socket.shutdown(tcp::socket::shutdown_both);
	}

	if (parser.get().result() != boost::beast::http::status::ok)
		throw std::runtime_error("Bad result from server.");

	return true;
}

bool fetch_file_withretry(boost::filesystem::path outputpath, const std::string uri, const FetchOptions& options, std::function< bool() > shouldstop)
{
	int retry = options.retry;
	while (retry != 0)
	{
		try {
			return fetch_file(outputpath, uri, options);
		}
		catch (std::exception &ec)
		{
			std::string wow = ec.what();
		}


		if (shouldstop)
			if (shouldstop())
				return false;
		retry--;
	}

	return false;
}
