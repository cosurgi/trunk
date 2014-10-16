// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMStateDiscrete.hpp"
#include <yade/core/Scene.hpp>

YADE_PLUGIN(
	(QMStateDiscrete)
	(WaveFunctionState)
	);

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   S T A T E
*
*********************************************************************************/
CREATE_LOGGER(QMStateDiscrete);
// !! at least one virtual function in the .cpp file
QMStateDiscrete::~QMStateDiscrete(){};

/*********************************************************************************
*
* W A V E   F U N C T I O N   S T A T E
*
*********************************************************************************/
CREATE_LOGGER(WaveFunctionState);
// !! at least one virtual function in the .cpp file
WaveFunctionState::~WaveFunctionState(){};

