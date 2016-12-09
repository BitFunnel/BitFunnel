#!/bin/bash

# Note: this isn't ideal with current code because slice sizes change with TermTreatment.

set -e
set -x

tools/BitFunnel/src/BitFunnel termtable /tmp/wikipedia.100.200/config/ PrivateSharedRank0 
tools/BitFunnel/src/BitFunnel repl /tmp/wikipedia.100.200/config/  -script /tmp/script.threads

cp -r /tmp/threads /tmp/rank0.100.200/


tools/BitFunnel/src/BitFunnel termtable /tmp/wikipedia.100.200/config/ PrivateSharedRank0And3
tools/BitFunnel/src/BitFunnel repl /tmp/wikipedia.100.200/config/  -script /tmp/script.threads

cp -r /tmp/threads /tmp/rank3rank0.100.200/

tools/BitFunnel/src/BitFunnel termtable /tmp/wikipedia.100.200/config/ Experimental
tools/BitFunnel/src/BitFunnel repl /tmp/wikipedia.100.200/config/  -script /tmp/script.threads

cp -r /tmp/threads /tmp/rankn.100.200/
