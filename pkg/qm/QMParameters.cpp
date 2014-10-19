// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMParameters.hpp"
#include <core/Scene.hpp>

YADE_PLUGIN(
	(QMParameters)
	);

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   P A R A M E T E R S
*
*********************************************************************************/
CREATE_LOGGER(QMParameters);
// !! at least one virtual function in the .cpp file
QMParameters::~QMParameters(){};


