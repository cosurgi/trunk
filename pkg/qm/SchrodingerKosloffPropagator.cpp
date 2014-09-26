// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "SchrodingerKosloffPropagator.hpp"
#include <yade/core/Scene.hpp>

YADE_PLUGIN(
	(SchrodingerKosloffPropagator)
	);

/*********************************************************************************
*
* K O S L O F F   P R O P A G A T I O N   o f   S H R O D I N G E R   E Q.
*
*********************************************************************************/
CREATE_LOGGER(SchrodingerKosloffPropagator);
// !! at least one virtual function in the .cpp file
SchrodingerKosloffPropagator::~SchrodingerKosloffPropagator(){};

