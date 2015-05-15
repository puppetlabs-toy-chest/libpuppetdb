/*
    libpuppetdb_test.cpp
    ======================

    libpuppetdb unit tests.
    Requires: googletest, googlemock.

*/

#include "../include/libpuppetdb/libpuppetdb.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <chrono>

// To test SSL with puppet agent certificates
// #define PUPET_AGENT_TEST

namespace LibPuppetdb {

// Mock classes

class MockURLConnector : public PuppetdbConnector {
  public:
    MockURLConnector(std::string host_name)
        : PuppetdbConnector::PuppetdbConnector(host_name) {}
    MOCK_METHOD2(getQueryUrl, std::string(Query& query, CURL* curl));
};


class MockSetupConnector : public PuppetdbConnector {
  public:
    MockSetupConnector(std::string host_name)
        : PuppetdbConnector::PuppetdbConnector(host_name) {}
    MOCK_METHOD1(setupAndPerform, std::string(Query& query));
};

// Testing the PuppetDB query

class QueryTest : public ::testing::Test {};

TEST_F(QueryTest, validQuery) {
    EXPECT_NO_THROW(Query("facter"));
}

TEST_F(QueryTest, validQueryWithQueryString) {
    EXPECT_NO_THROW(Query("nodes", "puppetdb_query"));
}

TEST_F(QueryTest, invalidQuery) {
    EXPECT_THROW(Query(""), query_error);
}

TEST_F(QueryTest, invalidQueryWithQueryString) {
    EXPECT_THROW(Query("", "puppetdb_query"), query_error);
}

TEST_F(QueryTest, getQueryComponents) {
    std::string endpoint { "eggs" };
    Query query { endpoint };

    EXPECT_EQ(endpoint, query.getEndpoint());
    EXPECT_EQ("", query.getQueryString());
}

TEST_F(QueryTest, getQueryComponentsWithQueryString) {
    std::string endpoint { "foo" };
    std::string query_string { "bar" };
    std::string expected_query { endpoint + "?query=" + query_string };
    Query query { endpoint, query_string };

    std::string obtained_query { query.getEndpoint() + "?query="
                                 + query.getQueryString() };
    EXPECT_EQ(expected_query, obtained_query);
}

// Testing the PuppetDB connector

class ConnectionTest : public ::testing::Test {
  protected:
    CURL* curl_handle_;

    virtual void SetUp() {
        curl_handle_ = curl_easy_init();
    }

    virtual void TearDown() {
        curl_easy_cleanup(curl_handle_);
    }
};

TEST_F(ConnectionTest, ValidConnector) {
    EXPECT_NO_THROW(PuppetdbConnector("eggs"));
}

TEST_F(ConnectionTest, ValidConnectorWithoutSSL) {
    EXPECT_NO_THROW(PuppetdbConnector("eggs", 42, ApiVersion::v3));
}

TEST_F(ConnectionTest, isSecure) {
    PuppetdbConnector connector { "spam", 42, ApiVersion::v2 };

    EXPECT_FALSE(connector.isSecure());
}

TEST_F(ConnectionTest, ConnectDefaultPortAndVersion) {
    PuppetdbConnector connector { "spam" };
    Query query { "facts" };
    std::string expected_url { "http://spam:8080/v4/facts" };

    EXPECT_FALSE(connector.isSecure());
    EXPECT_EQ(expected_url, connector.getQueryUrl(query, curl_handle_));
}

TEST_F(ConnectionTest, ConnectWithoutHost) {
    EXPECT_THROW(PuppetdbConnector(""), connector_error);
}

TEST_F(ConnectionTest, ConnectWithSSLNonExistentCerts) {
    auto ctor = []() { PuppetdbConnector("fake_host",
                                         "/fake/path/ca.cer",
                                         "/fake/path/host.cer",
                                         "/fake/path/host.key");
                     };

    EXPECT_THROW(ctor(), connector_error);
}

TEST_F(ConnectionTest, ConnectWithSSLValid) {
    auto ctor = []() { PuppetdbConnector("fake_host",
                                         "./resources/ca_crt.pem",
                                         "./resources/test_crt.pem",
                                         "./resources/test_key.pem");
                     };

    EXPECT_NO_THROW(ctor());
}

TEST_F(ConnectionTest, ConnectWithSSLIsSecure) {
    PuppetdbConnector connector { "fake_host",
                                  "./resources/ca_crt.pem",
                                  "./resources/test_crt.pem",
                                  "./resources/test_key.pem" };

    EXPECT_TRUE(connector.isSecure());
}

TEST_F(ConnectionTest, performQueryWithSimpleResult) {
    MockSetupConnector mock_connector { "bar" };
    Query query { "foo" };

    EXPECT_CALL(mock_connector, setupAndPerform(testing::_))
        .WillOnce(testing::Return("simple_result"));

    std::string result { mock_connector.performQuery(query) };

    EXPECT_EQ("simple_result", result);
}

TEST_F(ConnectionTest, httpQuery) {
    MockURLConnector mock_connector { "bar" };
    Query query { "foo" };
    EXPECT_CALL(mock_connector, getQueryUrl(testing::_, testing::_))
        .WillOnce(testing::Return("http://example.com"));

    std::string result { mock_connector.performQuery(query) };

    EXPECT_FALSE(result.size() == 0);
}

TEST_F(ConnectionTest, multipleHttpQueries) {
    MockURLConnector mock_connector { "spam" };
    Query first_query { "eggs" };
    Query second_query { "beans" };
    EXPECT_CALL(mock_connector, getQueryUrl(testing::_, testing::_))
        .WillOnce(testing::Return("http://example.com"))
        .WillOnce(testing::Return("http://goolge.com"));

    std::string first_result { mock_connector.performQuery(first_query) };
    std::string second_result { mock_connector.performQuery(second_query) };

    EXPECT_FALSE(first_result.size() == 0);
    EXPECT_FALSE(second_result.size() == 0);
}

#ifdef PUPET_AGENT_TEST
    TEST_F(ConnectionTest, testingPuppetSSL) {
        PuppetdbConnector connector {
            "master",
            "/etc/puppetlabs/puppet/ssl/certs/ca.pem",
            "/etc/puppetlabs/puppet/ssl/certs/centos65a.pem",
            "/etc/puppetlabs/puppet/ssl/private_keys/centos65a.pem" };
        Query query { "facts" };
        std::string result = connector.performQuery(query);
        std::cout << "Facter result:\n" << result << "\n\n";

        EXPECT_FALSE(result.size() == 0);
    }
#endif

}  // namespace LibPuppetdb
