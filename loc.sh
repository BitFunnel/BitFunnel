#!/bin/sh

# Count lines
find examples inc src test tools NativeJIT/Examples NativeJIT/inc NativeJIT/src NativeJIT/test -type f | xargs wc -l

# Count files
find examples inc src test tools NativeJIT/Examples NativeJIT/inc NativeJIT/src NativeJIT/test -type f | wc -l
