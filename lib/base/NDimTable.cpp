// 2015 © Janek Kozicki <cosurgi@gmail.com>

#include "FFTW3_Allocator.hpp"
#include "NDimTable.hpp"

#ifdef DEBUG_NDIM_RAM
	//template<> cpp_int Zcc<int>::NDimTable_SIZE_TOTAL=0;
	template<> cpp_int Zcc<int>::NDimTable_Instances=0;
	template<> cpp_int Zcc<int>::NDimTable_Alloc=0;
	template<> std::map<void*,cpp_int/*,cmpr*/> Zcc<int>::NDimTable_Allocated={};
#endif

std::ostream & operator<<(std::ostream &os, const std::vector<std::size_t>& dim)
{
	for(size_t i=0 ; i<dim.size() ; i++ )
	{
		os << dim[i] ;
		if(i!=dim.size()-1) os << ",";
	}
	return os;
};

