// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMPotential.hpp"
#include "QMStateDiscreteGlobal.hpp"

YADE_PLUGIN(
	(QMStateDiscreteGlobal)
	);

/*********************************************************************************
*
* Q M   G L O B A L   S T A T E   in   D I S C R E T E   E N T A N G L E D   form
*
*********************************************************************************/

CREATE_LOGGER(QMStateDiscreteGlobal);
// !! at least one virtual function in the .cpp file
QMStateDiscreteGlobal::~QMStateDiscreteGlobal(){};

