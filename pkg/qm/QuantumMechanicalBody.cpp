// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QuantumMechanicalBody.hpp"
#include <yade/core/Scene.hpp>

YADE_PLUGIN(
	(QuantumMechanicalBody)
	);

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   B O D Y
*
*********************************************************************************/
CREATE_LOGGER(QuantumMechanicalBody);
// !! at least one virtual function in the .cpp file
QuantumMechanicalBody::~QuantumMechanicalBody(){};

