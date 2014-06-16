# PuppetDB Query

## Intro

Connection: ssl or not
Multiple queries
Parse query

## Using puppetdbquery

Create connection
Create query
Perform query
Secure connection

### Build Requirements

- C++11

TODO: url, http, json, ssl

- testing: googletest, googlemock (?)

- libcurl: url, http, ssl

$ curl-config --cflags
-I/usr/local/include

$ curl-config --libs
-L/usr/local/lib -lcurl -lssh2 -lssl -lcrypto -lssl -lcrypto -lz
-lldap -lz

$ curl-config --feature
SSL
IPv6
libz
NTLM
NTLM_WB


- json (?)
- does libcurl need openssl or similar?
  check http://stackoverflow.com/questions/12636536/install-curl-with-openssl
- C++11

### API

create a connection
    - can't connect (is it possible to test?) --> returns error?
perform a query --> returns results
    - different result containers --> different methods?


### Creating a connection

does libcurl manage the whole authentication?

### Query

components
data --> will be a string --> will be converted to a json object

### Executing a query

### Results
