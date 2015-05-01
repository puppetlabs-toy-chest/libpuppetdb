#ifndef LIBPUPPETDB_INCLUDE_PUPPETDB_QUERY_H_
#define LIBPUPPETDB_INCLUDE_PUPPETDB_QUERY_H_

/*
 *                        libpuppetdb
 *
 * Simple library to execute Puppet DB queries in a synchronous.
 * To use it, create a PuppetdbConnector instance (SSL-enabled or not)
 * and call performQuery() on Query instances.
 * Multiple queries can be executed once a connection is set.
 * See example1.cpp and refer to README.
 *
 *
 * ASSUMPTIONS
 *
 * - C++11
 * - query format (query string is optional):
 *   {prot}://{hostname}:{port}/{version}/{endpoint}?query=<query_string>
 * - it is up to the client program to provide the query string in
 *   the puppetdb language and to process results, i.e.:
 *      - the interface expects the query string as a string argument;
 *      - the interface returns the json result data as a string.
 * - secure connection requires ca_cert, node_cert, and private_key.
 *
 */

#include <string>
#include <vector>
#include <map>
#include <curl/curl.h>
#include <iostream>
#include <fstream>

namespace LibPuppetdb {

// puppedbquery version
static const std::string VERSION_STRING { "0.1.0" };

// Protocol and ports
static const int PUPPETDB_HTTP_PORT { 8080 };
static const int PUPPETDB_SECURE_PORT { 8081 };

// PuppetDB api version
enum class ApiVersion { v2, v3 };
static std::map<ApiVersion, const std::string> ApiVersionsMap {
    { ApiVersion::v2, "v2" },
    { ApiVersion::v3, "v3" }
};
static const ApiVersion API_VERSION_DEFAULT { ApiVersion::v3 };

// Error codes (NB: starting at 100 to avoid clashing with curl ones)
enum class ErrorCode {
    OK = 100,
    INVALID_CONNECTION = 101,
    INVALID_QUERY = 102,
    URL_ENCODING_FAILURE = 103
};

// Utility function
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

    /// The query string must be URL-encoded
    Query(std::string endpoint, std::string query_string = "")
            : endpoint_ { endpoint },
              query_string_ { query_string } {
        if (endpoint.empty()) {
            error_code_ = static_cast<int>(ErrorCode::INVALID_QUERY);
        } else {
            error_code_ = static_cast<int>(ErrorCode::OK);
        }
    }

    bool isValid() const {
        return error_code_ == static_cast<int>(ErrorCode::OK);
    }

    int getErrorCode() const {
        return error_code_;
    }

    void setErrorCode(int error_code) {
        error_code_ = error_code;
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

    // Keeps track of invalid initialization and query status
    int error_code_;
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
// Connector
//

class PuppetdbConnector {
  public:
    PuppetdbConnector() = delete;

    /// Constructor for HTTP connector
    PuppetdbConnector(const std::string& hostname,
                      const int port = PUPPETDB_HTTP_PORT,
                      const ApiVersion api_version = API_VERSION_DEFAULT)
            : hostname_ { hostname },
              port_ { port },
              api_version_ { api_version },
              is_secure_ { false },
              is_valid_ { true },
              performed_query_url_ {} {
        checkHostname();
    }

    /// Constructor for SSL connector
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
              is_valid_ { true },
              performed_query_url_ {} {
        if (!checkHostname()) {
            return;
        }
        if (!checkSSLSupport()) {
            return;
        }
        checkCertificates();
    }

    bool isValid() const {
        return is_valid_;
    }

    bool isSecure() const {
        return is_secure_;
    }

    std::string getErrorMessage() const {
        return error_msg_;
    }

    std::string getPerformedQueryUrl() const {
        return performed_query_url_;
    }

    /// Returns the PuppetDB query result (json format) as a string.
    /// Returns an empty string in case of an invalid query or
    /// connector.
    std::string performQuery(Query& query) {
        if (isValid()) {
            if (query.isValid()) {
                return setupAndPerform(query);
            }
        } else {
            query.setErrorCode(static_cast<int>(ErrorCode::INVALID_CONNECTION));
        }
        return "";
    }

    /// In case a query string is specified, the method returns an
    /// empty string if the encoding fails.
    virtual std::string getQueryUrl(Query& query, CURL* curl) {
        std::string protocol { isSecure() ? "https" : "http" };
        std::string endpoint_and_query = query.getEndpoint();
        std::string query_str = query.getQueryString();

        if (!query_str.empty()) {
            // URL encode
            char* encoded_query = curl_easy_escape(
                curl, query_str.c_str(), 0);

            if (encoded_query == nullptr) {
                return "";
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

    // This mimics RAII (no exception handling - flags failures)
    bool is_valid_;
    std::string error_msg_;

    // The URL of the performed query
    std::string performed_query_url_;

    bool checkHostname() {
        if (hostname_.empty()) {
            is_valid_ = false;
            error_msg_ = "No hostname was specified.";
            return false;
        }
        return true;
    }

    bool checkSSLSupport() {
        curl_version_info_data* info_data = curl_version_info(CURLVERSION_NOW);
        if (info_data->features & CURL_VERSION_SSL) {
            return true;
        } else {
            is_valid_ = false;
            error_msg_ = "libcurl is not SSL enabled.";
            return false;
        }
    }

    bool checkCertificates() {
        for (auto attr : std::vector<std::string> { hostname_, ca_crt_path_,
                client_crt_path_, client_key_path_ }) {
            if (attr.empty()) {
                is_valid_ = false;
                error_msg_ = "Not all certificates were specified.";
                return false;
            }
        }

        for (auto attr : std::vector<std::string> { ca_crt_path_,
                client_crt_path_, client_key_path_ }) {
            if (!fileExists(attr)) {
                is_valid_ = false;
                error_msg_ = "Invalid certificate: " + attr;
                return false;
            }
        }
        return true;
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

            if (performed_query_url_.empty()) {
                query.setErrorCode(static_cast<int>(
                    ErrorCode::URL_ENCODING_FAILURE));
            } else {
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
                CURLcode return_code {curl_easy_perform(curl)};

                if (return_code != CURLE_OK) {
                    query.setErrorCode(static_cast<int>(return_code));
                }
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
