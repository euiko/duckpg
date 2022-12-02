# PG-Wire
is a library to write a server that speaks with PostgreSQL wire protocol.
It was forked from [convergence](https://github.com/returnString/convergence).
However any changes on this repo will not be merged into upstream, because the goals is different.
Here the server implementation doesn't wrap with specific interface, unlike in the original [convergence](https://github.com/returnString/convergence) it does a multi-stage query execution that consist of :
- Prepare query plan
- Execute query

PG-Wire will just passthrough any query that received from the client and up to the Engine implementation that will do the process. 