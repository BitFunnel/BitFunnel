import sys

args = sys.argv[1:]
if len(args) != 1:
    print("Required args: [input filename]")
    sys.exit(1)

filename = args[0]

line_num = 0
with open(filename) as f:
    for line in f:
        if (line_num % 20 == 0):
            print(line, end='')
        line_num += 1
