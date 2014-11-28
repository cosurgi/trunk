// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMStateBarrier.hpp"
#include <core/Scene.hpp>

YADE_PLUGIN(
	(QMStateBarrier)
	);

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   P O T E N T I A L   B A R R I E R
*
*********************************************************************************/
CREATE_LOGGER(QMStateBarrier);
// !! at least one virtual function in the .cpp file
QMStateBarrier::~QMStateBarrier(){};

