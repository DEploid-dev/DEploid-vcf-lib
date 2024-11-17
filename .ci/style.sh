#!/bin/bash

for file in $(ls src/*pp); 
#for file in $(ls src/*pp tests/unittest/*pp); 
  do echo $file 
  cpplint $file 2>&1 > lint_tmp
  if [ $? -ne 0 ]; then
    echo "Coding style check:"
    echo "cpplint ${file} failed"
    cat lint_tmp
    exit 1
  fi
done

echo "Coding style check: PASS"

