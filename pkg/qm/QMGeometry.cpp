// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMGeometry.hpp"
#include <core/Scene.hpp>

YADE_PLUGIN(
	(QMGeometry)
	);


/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   G E O M E T R Y   D I S P L A Y
*
*********************************************************************************/
CREATE_LOGGER(QMGeometry);
// !! at least one virtual function in the .cpp file
QMGeometry::~QMGeometry(){};

