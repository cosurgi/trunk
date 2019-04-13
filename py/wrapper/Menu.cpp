// 2015 © Janek Kozicki <cosurgi@gmail.com>
#include<py/wrapper/Menu.hpp>

std::ostream & operator<<(std::ostream &os, const Menu& menu)  { for(const std::string& s : menu) os << s << ' '; return os; };

std::string menuSelection(const Menu& m)
{
	if(m.size()==0)
		return "";
	else
		if(m[0].size()>7) {
			return m[0].substr(8);
		} else {
			std::cerr << "Menu.cpp has problems\n";
			return "";
		}
};

