#!/bin/sh
#
# This is an example how to start a TURN Server in
# secure 'dynamic' 'secret' mode (see TURNServerRESTAPI.pdf)
# with MySQL database for users information
# with the long-term credentials mechanism.
#
# We start here a TURN Server listening on IPv4 address
# 127.0.0.1 and on IPv6 address ::1. We use 127.0.0.1 as
# IPv4 relay address, and we use ::1 as IPv6 relay address.
#
# Other options:
#
# 1) set bandwidth limit on client session 3000000 bytes per second (--max-bps).
# 2) use fingerprints (-f)
# 3) use 3 relay threads (-m 3)
# 4) use min UDP relay port 32355 and max UDP relay port 65535
# 5) --use-auth-secret means that we are using 'secret' authentication mode.
# Absense of --static-auth-secret value means that we will be taking the secret value
# from the database ('dynamic' mode).
# 6) --secret-ts-exp-time=3600 means that the timestamp in secret authentication is expiring
# in 3600 seconds (1 hour). This is 'TTL' in terms of TURNServerRESTAPI.pdf document.
# 7) --realm=north.gov sets realm value as "north.gov".
# 8) --mysql-userdb="host=localhost dbname=turn user=turn password=turn connect_timeout=30" 
# means that local MySQL database "turn" will be used, with database user "turn" and 
# with database user password "turn", and connection timeout 30 seconds.
# 9) "--cert=example_turn_server_cert.pem" sets the OpenSSL certificate file name. 
# 10) "--pkey=example_turn_server_pkey.pem" sets the OpenSSL private key name.
# 11) "--log-file=stdout" means that all log output will go to the stdout. 
# Other parameters (config file name, etc) are default.

if [ -d examples ] ; then
       cd examples
fi

export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/local/lib/

PATH="./bin/:../bin/:../../bin/:${PATH}" turnserver -v --syslog -a -L 127.0.0.1 -L ::1 -E 127.0.0.1 -E ::1 --max-bps=3000000 -f -m 3 --min-port=32355 --max-port=65535 --use-auth-secret --secret-ts-exp-time=3600 --realm=north.gov --mysql-userdb="host=localhost dbname=turn user=turn password=turn connect_timeout=30" --cert=turn_server_cert.pem --pkey=turn_server_pkey.pem --log-file=stdout 
