// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMGeometryDisplay.hpp"
#include <yade/core/Scene.hpp>

YADE_PLUGIN(
	(QMGeometryDisplay)
	);


/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   G E O M E T R Y   D I S P L A Y
*
*********************************************************************************/
CREATE_LOGGER(QMGeometryDisplay);
// !! at least one virtual function in the .cpp file
QMGeometryDisplay::~QMGeometryDisplay(){};

