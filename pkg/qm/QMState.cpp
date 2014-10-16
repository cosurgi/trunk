// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMState.hpp"
#include <yade/core/Scene.hpp>

YADE_PLUGIN(
	(QMState)
	);

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   S T A T E
*
*********************************************************************************/
CREATE_LOGGER(QMState);
// !! at least one virtual function in the .cpp file
QMState::~QMState(){};

