all:
	colorgcc NDimTable_MarginalDistribution_Test.cpp ../../lib/base/NDimTable.cpp -o NDimTable_MarginalDistribution_Test -std=c++11 -I ../../ -lfftw3f -lfftw3l -lfftw3 -DYADE_FFTW3 -g -I /usr/include/eigen3/ -lboost_system
	colorgcc NDimTableTest.cpp                       ../../lib/base/NDimTable.cpp -o NDimTableTest                       -std=c++11 -I ../../ -lfftw3f          -lfftw3 -DYADE_FFTW3 -g -I /usr/include/eigen3/ -lboost_system
	colorgcc NDimTable_Fill_Test.cpp                 ../../lib/base/NDimTable.cpp -o NDimTable_Fill_Test                 -std=c++11 -I ../../ -lfftw3f          -lfftw3 -DYADE_FFTW3 -g -I /usr/include/eigen3/ -lboost_system
run:
	NDimTable_MarginalDistribution_Test
