# libpuppetdb

## Intro

libpuppetdb is a simple library for the execution of Puppet DB queries.
The library uses libcurl and relies on the Puppet DB REST API.
Both both SSL (HTTP) and non-SSL (HTTPS) queries are supported.

## Using libpuppetdb

To use libpuppetdb in your code, you first create a PuppetdbConnector object.
You do that by specifying the Puppet DB host and, for secure connections, the
SSL certificates. Then you execute queries by instantiating Query objects and
calling PuppetdbConnector::performQuery, that returns the query results
(json format) as a string.

## Errors

libpuppetdb does not raise exceptions. You can check the validity of a
connection or a query by calling the isValid method; both classes expose it.

To provide more information, the PuppetdbConnector class has the getErrorMessage
method that returns a string message about why the connector instantiation
failed.
Also, the Query class provides getErrorCode, a method that gives the return code
provided by libcurl after the query execution. Note that the libcurl codes are
in the [0, 99] range (please refer to the libcurl documentation) whereas an
integer greater than 100 indicates a failure during the creation of the
connection or query.

## Create a connection

To create a connection, you have to specify the Puppet DB host name. The service
port (default 8080) and the Puppet DB API version (default 'v3') are optional
arguments.
To create a secure connection, you also need to provide the file paths of the
required certificates, as descrbied in the Puppet DB online documenation:
the CA certificate, the client SSL certificate, and the client private key.

## Create a query

To create a query, you simply specify the Puppet DB endpoint (mandatory) and
the query string (optional).

## Build Requirements

libpuppetdb is written in C++11, so you must call your compiler accordingly.

The libcurl dev library is required. It must be SSL-enabled for secure
connections.

## Testing

Inside the test directory, run 1) "cmake .", 2) "make", and 3) "make test".
A simple non-SSL example is provided in the example directory.
