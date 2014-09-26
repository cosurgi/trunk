// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QuantumMechanicalGeometry.hpp"
#include <yade/core/Scene.hpp>

YADE_PLUGIN(
	(QuantumMechanicalGeometry)
	);


/*********************************************************************************
*
* W A V E   F U N C T I O N   G E O M E T R Y
*
*********************************************************************************/
CREATE_LOGGER(QuantumMechanicalGeometry);
// !! at least one virtual function in the .cpp file
QuantumMechanicalGeometry::~QuantumMechanicalGeometry(){};

