DEPLOIDvcfVERSION = $(shell git show HEAD | head -1 | sed -e "s/commit //g" | cat)
EXTRA_DIST = bootstrap

COMPILEDATE = $(shell date -u | sed -e "s/ /-/g")
distdir = $(PACKAGE)-$(VERSION)

bin_PROGRAMS = vcf vcf_dbg

TESTS = unit_tests
check_PROGRAMS = unit_tests vcf_dbg vcf_prof
PROG = DEPLOID

common_flags = -std=c++11 -Isrc/ -DDEPLOIDvcfVERSION=\"${DEPLOIDvcfVERSION}\" -DCOMPILEDATE=\"${COMPILEDATE}\"

common_LDADD = -lz

common_src = src/variantIndex.cpp \
             src/vcfReader.cpp \ 
			 src/txtReader.cpp \
			 src/gzstream/gzstream.cpp

debug_src = src/vcfReaderDebug.cpp

vcf_SOURCES = main.cpp $(common_src)
vcf_dbg_SOURCES =  $(debug_src) $(vcf_SOURCES)
vcf_prof_SOURCES = $(vcf_SOURCES)

vcf_CXXFLAGS = $(common_flags) -DNDEBUG -O3
vcf_dbg_CXXFLAGS = -g $(common_flags) -O3
vcf_prof_CXXFLAGS = $(common_flags) -DNDEBUG -fno-omit-frame-pointer -pg -O1

vcf_LDADD = $(common_LDADD)
vcf_dbg_LDADD = $(common_LDADD)
vcf_prof_LDADD = $(common_LDADD)

unit_tests_SOURCES = $(common_src) \
					 tests/unittest/test_runner.cpp \
					 tests/unittest/test_vcfReader.cpp \
					 tests/unittest/test_txtReader.cpp

unit_tests_CXXFLAGS = $(common_flags) -DNDEBUG -DUNITTEST -Wno-write-strings --coverage
unit_tests_LDADD    = -lcppunit -ldl $(common_LDADD)

clean-local: clean-local-check
.PHONY: clean-local-check utilities
clean-local-check:
	-rm -rf src/*.gc* tests/unittest/*.gc* src/*.gcov
