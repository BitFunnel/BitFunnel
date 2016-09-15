# StatisticsBuilder

The StatisticsBuilder program scans a corpus of documents in order to produce a
set of corpus statistics used to configure the BitFunnel index.

Usage
-----

StatisticsBuilder has two required command line arguments:

* Chunk file manifest. File containing list of chunk files to be processed.
Each chunk file name should appear on its own line. There should be no blank
lines. Recommend using absolute paths in chunk file names.
Chunk files for Wikipedia can be produced using Workbench.
The chunk file format is documented [here](http://bitfunnel.org).

* Output file directory. This directory must exist.
Statistics builder will overwrite files in the output directory, but it will
not clean up extraneous files from previous runs.

StatisticsBuilder takes a number of optional command line arguments:

* -statistics. Generate corpus statistics like the document length histogram,
document frequency tables, etc.

* -gramsize n. Sets the maximum lenght of phrases to be included in the
analysis. Value should be from 1 to Term::c_maxGramSize.

* -text. Generate Term::Hash to text mapping for diagnostic purposes.
When the -text flag is in effect, the document frequency table will include
a column with the term text. Note that the -text flag will slow the analysis
considerably.

Input Files
-----------

TODO: Write this section.

Output Files
------------

* CumulativeTermCounts-[SHARD].csv
* DocumentLenthHistogram.csv
* DocFreqTable-[SHARD].csv
* IndexedIdfTable-[SHARD].bin
* TermToText.bin


