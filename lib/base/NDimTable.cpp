// 2015 © Janek Kozicki <cosurgi@gmail.com>

#include "NDimTable.hpp"

std::ostream & operator<<(std::ostream &os, const std::vector<std::size_t>& dim)
{
	for(size_t i=0 ; i<dim.size() ; i++ )
	{
		os << dim[i] ;
		if(i!=dim.size()-1) os << ",";
	}
	return os;
};

