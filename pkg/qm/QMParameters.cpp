// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMParameters.hpp"

YADE_PLUGIN(
	(QMParameters)
	(QMParticle)
	);

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   P A R A M E T E R S
*
*********************************************************************************/
CREATE_LOGGER(QMParameters);
// !! at least one virtual function in the .cpp file
QMParameters::~QMParameters(){};

/*********************************************************************************
*
* Q M   P A R T I C L E   with mass
*
*********************************************************************************/
CREATE_LOGGER(QMParticle);
// !! at least one virtual function in the .cpp file
QMParticle::~QMParticle(){};

