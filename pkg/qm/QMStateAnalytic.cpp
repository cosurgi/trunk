// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMPotential.hpp"
#include "QMStateAnalytic.hpp"
#include "QMStateDiscrete.hpp"
#include "QMGeometry.hpp"
#include <core/Omega.hpp>

YADE_PLUGIN(
	(QMStateAnalytic)
	(St1_QMStateAnalytic)
	);

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   A N A L Y T I C A L   S T A T E
*
*********************************************************************************/
CREATE_LOGGER(QMStateAnalytic);
// !! at least one virtual function in the .cpp file
QMStateAnalytic::~QMStateAnalytic(){};

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   A N A L Y T I C A L   S T A T E   F U N C T O R
*
*********************************************************************************/
CREATE_LOGGER(St1_QMStateAnalytic);
// !! at least one virtual function in the .cpp file

Complexr St1_QMStateAnalytic::getValPos(Vector3r xyz , const QMParameters* par, const QMState* qms)
{
	throw std::runtime_error("\n\nSt1_QMStateAnalytic::getValPos was called directly.\n\n");
};

