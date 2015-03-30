// 2015 © Janek Kozicki <cosurgi@gmail.com>
#include<py/wrapper/Menu.hpp>
std::string menuSelection(const Menu& m)
{
	if(m.size()==0)
		return "";
	else
		return m[0].substr(8);
};

