/*
    example1.cpp
    ============

    Basic usage of the libpuppetdb (without SSL)

    On OS X, compile with:
        c++ -std=c++11 -lcurl example1.cpp -o example

    By default, puppetdb only accepts http over 8080 as localhost.
    Check http://goo.gl/HGV4Mr.

    Run with:
        ./example hostname puppetdb_endpoint puppetdb_query
    For instance:
        ./exmple localhost facts ""
        ./example localhost nodes '["=", "name", "master"]'
    after running:
        ssh -NL 8080:localhost:8080 {user}@{puppetdb host} -f
*/


#include "../include/libpuppetdb/libpuppetdb.h"
#include <string>

using namespace LibPuppetdb;

int main(int argc, char* argv[]) {
    if (argc != 4)
        return 1;

    std::string hostname { argv[1] };
    std::string endpoint { argv[2] };
    std::string query_string { argv[3] };

    try {
        PuppetdbConnector connector { hostname, 8080, ApiVersion::v3 };

        Query query { endpoint, query_string };

        auto result = connector.performQuery(query);

        std::cout << "Result:\n" << result << "\n\n";
        std::cout << "Performed query: " << connector.getPerformedQueryUrl()
                  << "\n";
        return 0;
    } catch (connector_error& e) {
        std::cout << "Failed to initialize the connector: " << e.what() << "\n";
        return 2;
    } catch (query_error& e) {
        std::cout << "Failed to initialize the query: " << e.what() << "\n";
        return 3;
    } catch (processing_error& e) {
        std::cout << "Failed to perform the query: " << e.what() << "\n";
        return 4;
    }
}
