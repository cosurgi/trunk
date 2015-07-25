// 2015 © Janek Kozicki <cosurgi@gmail.com>

#include "NDimTable.hpp"

template<> cpp_int Zcc<int>::NDimTable_SIZE_TOTAL=0;
template<> cpp_int Zcc<int>::NDimTable_Instances=0;

std::ostream & operator<<(std::ostream &os, const std::vector<std::size_t>& dim)
{
	for(size_t i=0 ; i<dim.size() ; i++ )
	{
		os << dim[i] ;
		if(i!=dim.size()-1) os << ",";
	}
	return os;
};

