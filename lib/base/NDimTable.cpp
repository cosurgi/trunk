// 2015 © Janek Kozicki <cosurgi@gmail.com>

#include "FFTW3_Allocator.hpp"
#include "NDimTable.hpp"

#ifdef DEBUG_NDIM_RAM
	//template<> cpp_int Zcc<int>::NDimTable_SIZE_TOTAL=0;
	template<> cpp_int Zcc<int>::NDimTable_Instances=0;
	template<> cpp_int Zcc<int>::NDimTable_Alloc=0;
	template<> std::map<void*,cpp_int/*,cmpr*/> Zcc<int>::NDimTable_Allocated={};
#endif

#ifndef FORCE_N_THREADS
template<> int Thr_FIXME_linker<int>::number=16;
#endif

template<> std::ostream & operator<<(std::ostream &os, const std::vector<std::size_t>& dim);
template<> std::ostream & operator<<(std::ostream &os, const std::vector<double>& dim);

template<> bool operator>=(const std::vector<std::size_t>& a, const std::vector<std::size_t>& b);
template<> bool operator< (const std::vector<std::size_t>& a, const std::vector<std::size_t>& b);

