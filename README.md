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

lippuppetdb errors follow a class hierarchy where the parent class is
`libpuppetdb_error`. Specific errors are:

| name | description
|------|------|------------
| query_error | thrown by the Query constructor
| connector_error | thrown by the PuppetdbConnector constructor
| processing_error | thrown by the PuppetdbConnector::performQuery method

Note that `processing_error` objects contain an indication of possible
libcurl errors.

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
