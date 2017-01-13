import csv
import math

text_freq = {}

with open("/home/danluu/dev/wikipedia.1000.1500/config/DocFreqTable-0.csv") as f:
    reader = csv.reader(f)
    header = next(reader)
    assert header == ['hash','gramSize','streamId','frequency','text']
    for row in reader:
        freq = float(row[3])
        text = row[4]
        text_freq[text] = freq

temp = []

with open("/home/danluu/dev/wikipedia.100.150/config/DocFreqTable-0.csv") as f:
    reader = csv.reader(f)
    header = next(reader)
    assert header == ['hash','gramSize','streamId','frequency','text']
    for row in reader:
        this_freq = float(row[3])
        text = row[4]
        other_freq = 1e-8
        if text in text_freq:
            other_freq = text_freq[text]
            del text_freq[text]
        temp.append([text, -math.log10(this_freq), -math.log10(other_freq)])

# items that are still in text_freq are in the first DocFreqTable but not the second.
for text, freq in text_freq.items():
    inf_idf = math.log10(1e-8)
    temp.append([text, -inf_idf, -math.log10(freq)])
        
output = sorted(temp, key=lambda x: x[1])

outf = open('/tmp/compare-docfreqs.csv', 'w')
writer = csv.writer(outf)

writer.writerow(['text','idf1','idf2'])
for row in output:
    writer.writerow(row)
