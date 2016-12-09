# Take a query log with no phrases and generate a copy of the log with each query duplicated as a phrase query.

with open("/tmp/wikipedia/config/QueryLogNoPhrases.txt") as f:
    lines = f.read().splitlines()
    lines = [line for line in lines if len(line.split()) > 1]
    quoted_lines = ['"' + line + '"' for line in lines]

    for line in lines:
        print(line)
    for line in quoted_lines:
        print(line)
    
