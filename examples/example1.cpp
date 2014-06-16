/*
    example1.cpp
    ============

    Basic usage of the puppetdbquery

    Compile with:
        c++ -std=c++11 -lcurl example1.cpp -o example

    By default, puppetdb only accepts http over 8080 as localhost.
    Check http://goo.gl/HGV4Mr.

    Run with:
        ./example hostname puppetdb_endpoint puppetdb_query
    For instance:
        ./exmple localhost facts ""
    after running:
        ssh -NL 8080:localhost:8080 {user}@{puppetdb host} -f
*/


#include "../include/puppetdbquery/puppetdbquery.h"
#include <string>

using namespace PuppetdbQuery;

int main(int argc, char* argv[]) {
    if (argc != 4)
        return 1;

    std::string hostname {argv[1]};
    std::string endpoint {argv[2]};
    std::string query_string {argv[3]};

    PuppetdbConnector connector {hostname};
    Query query {endpoint, query_string};

    std::string result {connector.performQuery(query)};
    std::cout << "Return code: " << query.getErrorCode() << "\n";
    std::cout << "Result:\n" << result << "\n\n";
    return 0;
}
