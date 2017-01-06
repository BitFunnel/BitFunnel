#!/bin/bash

# Note: this isn't ideal with current code because slice sizes change with TermTreatment.

set -e
set -x

tools/BitFunnel/src/BitFunnel termtable /home/danluu/dev/wikipedia.100.150/config/ 0.1 PrivateSharedRank0 
tools/BitFunnel/src/BitFunnel repl /home/danluu/dev/wikipedia.100.150/config/  -script /home/danluu/dev/wikipedia.100.150/script.query.docfreq.threads

cp -r /tmp/threads /tmp/rank0/


# tools/BitFunnel/src/BitFunnel termtable /home/danluu/dev/wikipedia.100.150/config/ PrivateSharedRank0And3
# tools/BitFunnel/src/BitFunnel repl /home/danluu/dev/wikipedia.100.150/config/  -script /home/danluu/dev/script.threads
# cp -r /home/danluu/dev/threads /home/danluu/dev/rank3rank0.100.150/

tools/BitFunnel/src/BitFunnel termtable /home/danluu/dev/wikipedia.100.150/config/ 0.1 Optimal
tools/BitFunnel/src/BitFunnel repl /home/danluu/dev/wikipedia.100.150/config/  -script /home/danluu/dev/wikipedia.100.150/script.query.docfreq.threads

cp -r /tmp/threads /tmp/rankN/
