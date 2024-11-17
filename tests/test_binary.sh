#!/bin/bash

function test_noRepeat {
  echo -n " vcf $@ "
  # Test using vcf self-checks
  ./vcf_dbg $@ > /dev/null
  if [ $? -ne 0 ]; then
    echo ""
    echo "Executing \"./vcf_dbg $@ \" failed."
    echo "Debug Call: make -mj2 vcf_dbg && ./vcf_dbg $@ 2>&1 | less"
    exit 1
  fi

  # Test for memory leaks
  valgrind --error-exitcode=1 --leak-check=full -q --gen-suppressions=yes ./vcf $@ > /dev/null
  if [ $? -ne 0 ]; then
    echo ""
    echo "Valgrind check of \"valgrind --error-exitcode=1 --leak-check=full -q --gen-suppressions=yes ./vcf $@ \" failed."
    exit 1
  fi

  echo " done."
}

sameFlags="data/testData/PG0390-C.test.vcf"

echo "Testing examples"
 # vcf.gz test wouldn't work, due to zlib version is 1.2.3.4 on travis and circle. it is difficult to update.
 #test_vcf ${sameFlags} -vcf data/testData/PG0390-C.test.vcf.gz -noPanel -o tmp1 || exit 1
 test_noRepeat ${sameFlags}
 # The following test takes long time, turn off on travis for now ...
 #test_vcf ${sameFlags} -vcf data/testData/PG0390-C.test.vcf -panel data/testData/labStrains.test.panel.txt -o tmp2 -painting tmp1.hap || exit 1
 #test_vcf ${sameFlags} -vcf data/testData/PG0390-C.test.vcf -panel data/testData/labStrains.test.panel.txt -o tmp1 || exit 1
 #test_vcf ${sameFlags} -ref data/testData/PG0390-C.test.ref -alt data/testData/PG0390-C.test.alt -noPanel -o tmp1 || exit 1
 #test_vcf ${sameFlags} -ref data/testData/PG0390-C.test.ref -alt data/testData/PG0390-C.test.alt -panel data/testData/labStrains.test.panel.txt -o tmp1 || exit 1
echo ""
