// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QuantumMechanicalMaterial.hpp"
#include <yade/core/Scene.hpp>

YADE_PLUGIN(
	(WavePacketParameters)
	);

/*********************************************************************************
*
* W A V E   F U N C T I O N   M A T E R I A L
*
*********************************************************************************/
CREATE_LOGGER(WavePacketParameters);
// !! at least one virtual function in the .cpp file
WavePacketParameters::~WavePacketParameters(){};


