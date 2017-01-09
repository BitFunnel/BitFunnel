This directory contains scripts that are often user specific. Unlike all other code in the BitFunnel repository, little or no effort has gone into making this code cross-platform, or for that matter, even runnable on more than one specific machine.

Many of these scripts are throwaway scripts intended to automate a short-term task (e.g., producing a set of graphs for a paper).

### bf-density.r

Takes summaries from `verify log` or `verify.py` and generates plots of false positives.

#### convert.go

Helper script to convert BitFunnel code from Windows to Linux. Not kept up to date.

#### dependencies.go

Get transitive closure of dependencies from old BitFunnel code.

#### get-random-hashes.py

Generate some random 64-bit (8-byte) hashes.

#### plot-history.r

Plot LOC history.

#### plot-sim.r

Plot simulation results from `simulate.cpp`

#### row-correlation.py

Quick script to find unusual row correlations. TODO: replace with C++ code because this is too slow.

#### show-term-convert.py

Helper script for `row-correlation.py`

#### simulate.cpp

Quick and dirty simulation for finding number of memory accesses per block.

#### slides.go

Convert talk slides to HTML.