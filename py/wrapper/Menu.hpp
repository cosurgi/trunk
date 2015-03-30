// 2015 © Janek Kozicki <cosurgi@gmail.com>
// This typedef is used to make drop down menu QComboBox possible in qt4 interface
//
// an example usage is following: in the macro that declares the class, declare a variable like this:
//
//	YADE_CLASS_BASE_DOC_ATTRS(ThisClass,BaseClass,"class documentation",
//	        ((Menu          ,drawStyle   ,Menu({"default surface","hidden","points","wire","surface"}),,"Select drawing style in the menu")));
//
// Then the displayed options in qt4 interface are all except the first one.
// The first one is used to select a default setting.
//
// Then later, in the program use this command to see which one is selected:
//
//	if(menuSelection(drawStyle)=="wire") ....

#pragma once
#include<string>
#include<vector>
typedef            std::vector<std::string>   Menu;
std::string menuSelection(const Menu& m);

// NOTE: it is simplest way to do this just by typedef. The caveat is that any
// vector<string> type might potentially become a dropdown menu in the qt4
// interface, provided that first 7 characters in the zeroth element in the
// vector are "default". If anybody runs into this problem, please let me know.
// This problem arises from the fact that static attributes lose docstring when
// exposed to python, see http://stackoverflow.com/questions/25386370/docstrings-for-static-properties-in-boostpython
// so the 'extra' info had to be stored inside the actual data.
// Janek

