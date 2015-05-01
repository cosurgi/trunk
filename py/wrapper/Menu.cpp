// 2015 © Janek Kozicki <cosurgi@gmail.com>
#include<py/wrapper/Menu.hpp>

std::ostream & operator<<(std::ostream &os, const Menu& menu)  { for(const std::string& s : menu) os << s << ' '; return os; };

std::string menuSelection(const Menu& m)
{
	if(m.size()==0)
		return "";
	else
		return m[0].substr(8);
};

