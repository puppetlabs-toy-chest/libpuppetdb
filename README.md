# PuppetDB Query

## Intro

Connection: ssl or not
Multiple queries
Query: assumptions

## Using puppetdbquery

Create connection
Create query
Perform query
Secure connection

### Build Requirements

- C++11

- testing: googletest, googlemock (?)

- libcurl


### Creating a connection

does libcurl manage the whole authentication?

### Query

components
The query string must be a url-encoded string.

### Executing a query

connector.performQuery(query)

### Results

The query results (json) are returned as a  string.
