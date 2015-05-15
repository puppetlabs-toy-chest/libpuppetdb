#ifndef LIBPUPPETDB_INCLUDE_PUPPETDB_QUERY_H_
#define LIBPUPPETDB_INCLUDE_PUPPETDB_QUERY_H_

/*
 *                        libpuppetdb
 *
 * Simple library to execute Puppet DB queries in a synchronous.
 * To use it, create a PuppetdbConnector instance (SSL-enabled or not)
 * and call performQuery() on Query instances.
 *
 * The user can spcecify the puppetdb API version (v2, v3, or v4, as
 * for the ApiVersion enumeration) when instantiating the connector;
 * this will only affect the api URL since the query structure does
 * not change change across the supported versions. Also, the user is
 * responsible for specifying an endpoint compatible with the version.
 *
 * Multiple queries can be executed once a connection is set.
 * See example1.cpp and refer to README.
 *
 *
 * REQUIREMENTS
 *
 * - C++11 compiler
 * - libcurl, SSL-enabled for secure connections
 *
 *
 * ASSUMPTIONS
 *
 * - query format (query string is optional):
 *   {prot}://{hostname}:{port}/{version}/{endpoint}?query=<query_string>
 * - it is up to the client program to provide an endpoint compatible
 *   with the specified API version
 * - it is up to the client program to provide the query string in
 *   the puppetdb language and to process results, i.e.:
 *      - the interface expects the query string as a string argument
 *      - the interface returns the json result data as a string
 * - secure connection requires ca_cert, node_cert, and private_key
 *
 */

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include <curl/curl.h>

namespace LibPuppetdb {

//
// Tokens
//

// puppedbquery version
static const std::string VERSION_STRING { "0.2.0" };

// Protocol and ports
static const int PUPPETDB_HTTP_PORT { 8080 };
static const int PUPPETDB_SECURE_PORT { 8081 };

// PuppetDB api version
enum class ApiVersion { v2, v3, v4 };
static std::map<ApiVersion, const std::string> ApiVersionsMap {
    { ApiVersion::v2, "v2" },
    { ApiVersion::v3, "v3" },
    { ApiVersion::v4, "v4" }
};
static const ApiVersion API_VERSION_DEFAULT { ApiVersion::v4 };

//
// Errors
//

// Parent error class
class libpuppetdb_error : public std::runtime_error {
  public:
    explicit libpuppetdb_error(std::string const& msg) : std::runtime_error(msg) {}
};

// Invalid query
class query_error : public libpuppetdb_error {
  public:
    explicit query_error(std::string const& msg) : libpuppetdb_error(msg) {}
};

// Invalid connector
class connector_error : public libpuppetdb_error {
  public:
    explicit connector_error(std::string const& msg) : libpuppetdb_error(msg) {}
};

// Error due to a query processing failure
class processing_error : public libpuppetdb_error {
  public:
    explicit processing_error(std::string const& msg) : libpuppetdb_error(msg) {}
};

//
// Utility functions
//

inline bool fileExists (const std::string& file_path) {
    bool exists { false };
    if (!file_path.empty()) {
        std::ifstream f_s { file_path };
        exists = f_s.good();
        f_s.close();
    }
    return exists;
}

//
// Query
//

class Query {
  public:
    Query() = delete;

    /// The query string must be URL-encoded.
    /// Throws a query_error in case the endpoint is an empty string.
    Query(std::string endpoint, std::string query_string = "")
            : endpoint_ { endpoint },
              query_string_ { query_string } {
        if (endpoint.empty()) {
            throw query_error { "no endpoint specified" };
        }
    }

    std::string getEndpoint() {
        return endpoint_;
    }

    std::string getQueryString() {
        return query_string_;
    }

  private:
    // Endpoint
    std::string endpoint_;

    // Query string
    std::string query_string_;
};

//
// Query result
//

struct QueryResult {
    char* content;
    size_t size;

    static size_t callback(void* contents, size_t size, size_t nmemb,
                           void *userp) {
        static_cast<std::string*>(userp)->append(static_cast<char*>(contents),
                                                 size * nmemb);
        return size * nmemb;
    }
};

//
// PuppetdbConnector
//

class PuppetdbConnector {
  public:
    PuppetdbConnector() = delete;

    /// Constructor for HTTP connector
    /// Throws a connector_error in case if the hostname is an empty
    /// string
    PuppetdbConnector(const std::string& hostname,
                      const int port = PUPPETDB_HTTP_PORT,
                      const ApiVersion api_version = API_VERSION_DEFAULT)
            : hostname_ { hostname },
              port_ { port },
              api_version_ { api_version },
              is_secure_ { false },
              performed_query_url_ {} {
        checkHostname();
    }

    /// Constructor for SSL connector
    /// Throws a connector_error in case if the hostname is an empty
    /// string, if libcurl is not SSL enabled, or if any of the
    /// specified certificates is not an existent file
    PuppetdbConnector(const std::string& hostname,
                      const std::string& ca_crt_path,
                      const std::string& client_crt_path,
                      const std::string& client_key_path,
                      const int port = PUPPETDB_SECURE_PORT,
                      const ApiVersion api_version = API_VERSION_DEFAULT)
            : hostname_ { hostname },
              port_ { port },
              api_version_ { api_version },
              ca_crt_path_ { ca_crt_path },
              client_crt_path_ { client_crt_path },
              client_key_path_ { client_key_path },
              is_secure_ { true },
              performed_query_url_ {} {
        checkHostname();
        checkSSLSupport();
        checkCertificates();
    }

    /// Returns true if the Connector instance uses SSL, false
    /// otherwise
    bool isSecure() const {
        return is_secure_;
    }

    /// Returns the PuppetDB query result (json format) as a string
    /// Throws a processing_error in case of failure
    std::string performQuery(Query& query) {
        return setupAndPerform(query);
    }

    /// Returns the URL used to perform the PuppetDB query
    std::string getPerformedQueryUrl() const {
        return performed_query_url_;
    }

    // NB: this method is public and virtual for testing purposes
    virtual std::string getQueryUrl(Query& query, CURL* curl) {
        std::string protocol { isSecure() ? "https" : "http" };
        std::string endpoint_and_query = query.getEndpoint();
        std::string query_str = query.getQueryString();

        if (!query_str.empty()) {
            // URL encode
            char* encoded_query = curl_easy_escape(curl, query_str.c_str(), 0);

            if (encoded_query == nullptr) {
                throw processing_error { "failed to encode the query URL" };
            }

            endpoint_and_query += "?query=" + std::string { encoded_query };

            // Free the memory used by curl_easy_escape()
            curl_free(encoded_query);
        }

        return protocol + "://" + hostname_ + ":" + std::to_string(port_)
               + "/" + getApiVersion() + "/" + endpoint_and_query;
    }

  private:
    // PuppetDB host, port, and api version
    std::string hostname_;
    int port_;
    ApiVersion api_version_;

    // File paths used for the SSL connection
    std::string ca_crt_path_;
    std::string client_crt_path_;
    std::string client_key_path_;

    // SSL flag
    bool is_secure_;

    // The URL of the performed query
    std::string performed_query_url_;

    void checkHostname() {
        if (hostname_.empty()) {
            throw connector_error { "no hostname specified" };
        }
    }

    void checkSSLSupport() {
        curl_version_info_data* info_data = curl_version_info(CURLVERSION_NOW);
        if (!(info_data->features & CURL_VERSION_SSL)) {
            throw connector_error { "libcurl is not SSL enabled" };
        }
    }

    void checkCertificates() {
        std::vector<std::string> certificates { ca_crt_path_,
                                                client_crt_path_,
                                                client_key_path_ };
        for (auto cert : certificates) {
            if (cert.empty()) {
                throw connector_error { "not all certificates were specified" };
            }
        }

        for (auto cert : certificates) {
            if (!fileExists(cert)) {
                throw connector_error { "invalid certificate file: " + cert };
            }
        }
    }

    std::string getApiVersion() {
        return ApiVersionsMap[api_version_];
    }

    // NB: this is virtual to enable mocking
    virtual std::string setupAndPerform(Query& query) {
        std::string result_buffer {};

        // libcurl handle
        CURL* curl { curl_easy_init() };

        if(curl) {
            performed_query_url_ = getQueryUrl(query, curl);

            // Configure the libcurl handle
            curl_easy_setopt(curl, CURLOPT_URL,
                             performed_query_url_.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                             QueryResult::callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result_buffer);

            if (isSecure()) {
                setSSLOptions(curl);
            }

            // Perform the query
            CURLcode return_code { curl_easy_perform(curl) };

            if (return_code != CURLE_OK) {
                throw processing_error {
                    std::string(curl_easy_strerror(return_code)) };
            }

            // Close all libcurl connections
            curl_easy_cleanup(curl);
        }
        return result_buffer;
    }

    void setSSLOptions(CURL* curl) {
        curl_easy_setopt(curl, CURLOPT_SSLCERT, client_crt_path_.c_str());
        curl_easy_setopt(curl, CURLOPT_SSLKEY, client_key_path_.c_str());
        curl_easy_setopt(curl, CURLOPT_CAINFO, ca_crt_path_.c_str());
    }
};

}  // namespace LibPuppetdb

#endif  // LIBPUPPETDB_INCLUDE_PUPPETDB_QUERY_H_
