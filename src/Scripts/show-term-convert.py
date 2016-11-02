# Convert from show term to list of rows associated with each term.

import re
term_regex = re.compile("Term\(\"(\S+)\"\)")
rowid_regex = re.compile("\s+RowId\((\S+),\s+(\S+)\)")

this_term = ""
with open("/tmp/show.results.txt") as f:
    for line in f:
        rowid_match = rowid_regex.match(line)
        if rowid_match:
            this_term += "," + rowid_match.group(1) + "-" + rowid_match.group(2)

        term_match = term_regex.match(line)
        if term_match:
            print(this_term)
            this_term = term_match.group(1)
