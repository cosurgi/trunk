// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMBody.hpp"
#include <core/Scene.hpp>

YADE_PLUGIN(
	(QMBody)
	);

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   B O D Y
*
*********************************************************************************/
CREATE_LOGGER(QMBody);
// !! at least one virtual function in the .cpp file
QMBody::~QMBody(){};

