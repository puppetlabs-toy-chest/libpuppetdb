/*
    puppetdbquery_test.cpp
    ======================

    PuppetDB Query unit tests.
    Requires: googletest, googlemock.

*/


#ifndef PUPPETDBQUERY_TEST_PUPPETDB_QUERY_TEST_H_
#define PUPPETDBQUERY_TEST_PUPPETDB_QUERY_TEST_H_

#include "../include/puppetdbquery/puppetdbquery.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <chrono>

// To test SSL with puppet agent certificates
// #define PUPET_AGENT_TEST

namespace PuppetdbQuery {


// Mock classes

class MockURLConnector : public PuppetdbConnector {
  public:
    using PuppetdbConnector::PuppetdbConnector;
    MOCK_METHOD1(getQueryUrl, std::string(Query& query));
};


class MockSetupConnector : public PuppetdbConnector {
  public:
    using PuppetdbConnector::PuppetdbConnector;
    MOCK_METHOD1(setupAndPerform, std::string(Query& query));
};



// Testing the PuppetDB query

class QueryTest : public ::testing::Test {
};

TEST_F(QueryTest, validQuery) {
    Query query {"facter"};
    Query query_with_query {"nodes", "puppetdb_query"};

    EXPECT_TRUE(query.isValid());
    EXPECT_EQ(static_cast<int>(ErrorCode::OK), query.getErrorCode());
    EXPECT_TRUE(query_with_query.isValid());
    EXPECT_EQ(static_cast<int>(ErrorCode::OK), query_with_query.getErrorCode());
}

TEST_F(QueryTest, invalidQuery) {
    Query query {""};
    Query query_with_query {"", "puppetdb_query"};

    EXPECT_FALSE(query.isValid());
    EXPECT_EQ(static_cast<int>(ErrorCode::INVALID_QUERY), query.getErrorCode());
    EXPECT_FALSE(query_with_query.isValid());
    EXPECT_EQ(static_cast<int>(ErrorCode::INVALID_QUERY),
              query_with_query.getErrorCode());
}

TEST_F(QueryTest, setAndGetErrorCode) {
    Query query {"spam"};
    int error_code {42};
    query.setErrorCode(error_code);

    EXPECT_EQ(error_code, query.getErrorCode());
}

TEST_F(QueryTest, toString) {
    std::string endpoint {"eggs"};
    Query query {endpoint};

    EXPECT_EQ(endpoint, query.toString());
}

TEST_F(QueryTest, toStringWithQueryString) {
    std::string endpoint {"foo"};
    std::string query_string {"bar"};
    std::string expected_query {endpoint + "?query=" + query_string};
    Query query {endpoint, query_string};

    EXPECT_EQ(expected_query , query.toString());
}


// Testing the PuppetDB connector

class ConnectionTest : public ::testing::Test {
};


TEST_F(ConnectionTest, ConnectWithoutSSL) {
    PuppetdbConnector connector {"spam", 42, ApiVersion::v2};

    EXPECT_FALSE(connector.isSecure());
}

TEST_F(ConnectionTest, ValidConnectionWithoutSSL) {
    PuppetdbConnector connector {"eggs", 42, ApiVersion::v3};

    EXPECT_TRUE(connector.isValid());
    EXPECT_EQ("", connector.getErrorMessage());
}

TEST_F(ConnectionTest, ConnectDefaultPortAndVersion) {
    PuppetdbConnector connector {"spam"};
    Query query("facts");
    std::string expected_url {"http://spam:8080/v3/facts"};

    EXPECT_FALSE(connector.isSecure());
    EXPECT_TRUE(connector.isValid());
    EXPECT_EQ(expected_url, connector.getQueryUrl(query));
}

TEST_F(ConnectionTest, ConnectWithoutHost) {
    PuppetdbConnector connector {""};

    EXPECT_FALSE(connector.isValid()); // No hostname!
    EXPECT_EQ("No hostname was specified.", connector.getErrorMessage());
}

TEST_F(ConnectionTest, ConnectWithSSL) {
    PuppetdbConnector connector {"fake_host",
                                 "/fake/path/ca.cer",
                                 "/fake/path/host.cer",
                                 "/fake/path/host.key"};

    EXPECT_TRUE(connector.isSecure());
    EXPECT_FALSE(connector.isValid()); // Certificates are fake!
    EXPECT_EQ("Invalid certificate: /fake/path/ca.cer",
              connector.getErrorMessage());
}

TEST_F(ConnectionTest, performQueryWithInvalidConnection) {
    PuppetdbConnector connector {""};
    Query query {"spam"};
    std::string result = connector.performQuery(query);

    EXPECT_EQ("", result);
}

TEST_F(ConnectionTest, performQueryWithSimpleResult) {
    MockSetupConnector mock_connector {"bar"};
    Query query {"foo"};

    EXPECT_TRUE(mock_connector.isValid());

    EXPECT_CALL(mock_connector, setupAndPerform(testing::_))
        .WillOnce(testing::Return("simple_result"));

    std::string result = mock_connector.performQuery(query);

    EXPECT_EQ("simple_result", result);
    EXPECT_EQ("", mock_connector.getErrorMessage());
}

TEST_F(ConnectionTest, httpQuery) {
    MockURLConnector connector {"bar"};
    Query query {"foo"};
    EXPECT_CALL(connector, getQueryUrl(testing::_))
        .WillOnce(testing::Return("http://example.com"));

    std::string result = connector.performQuery(query);

    EXPECT_FALSE(result.size() == 0);
}

TEST_F(ConnectionTest, multipleHttpQueries) {
    MockURLConnector connector {"spam"};
    Query first_query {"eggs"};
    Query second_query {"beans"};
    EXPECT_CALL(connector, getQueryUrl(testing::_))
        .WillOnce(testing::Return("http://example.com"))
        .WillOnce(testing::Return("http://goolge.com"));

    std::string first_result = connector.performQuery(first_query);
    std::string second_result = connector.performQuery(second_query);

    EXPECT_FALSE(first_result.size() == 0);
    EXPECT_FALSE(second_result.size() == 0);
}

#ifdef PUPET_AGENT_TEST
    TEST_F(ConnectionTest, testingPuppetSSL) {
        PuppetdbConnector connector {
            "master",
            "/etc/puppetlabs/puppet/ssl/certs/ca.pem",
            "/etc/puppetlabs/puppet/ssl/certs/centos65a.pem",
            "/etc/puppetlabs/puppet/ssl/private_keys/centos65a.pem"};
        Query query {"facts"};
        std::string result = connector.performQuery(query);
        std::cout << "Facter result:\n" << result << "\n\n";
        EXPECT_FALSE(result.size() == 0);
    }
#endif

}  // namespace PuppetdbQuery

#endif  // PUPPETDBQUERY_TEST_PUPPETDB_QUERY_TEST_H_
