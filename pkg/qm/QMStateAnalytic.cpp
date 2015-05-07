// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMStateAnalytic.hpp"
#include "QMStateDiscrete.hpp"
#include "QMGeometry.hpp"
#include <core/Omega.hpp>

YADE_PLUGIN(
	(QMStateAnalytic)
	);

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   A N A L Y T I C A L   S T A T E
*
*********************************************************************************/
CREATE_LOGGER(QMStateAnalytic);
// !! at least one virtual function in the .cpp file
QMStateAnalytic::~QMStateAnalytic(){};

boost::shared_ptr<QMStateDiscrete>& QMStateAnalytic::prepareReturnStateDiscreteOptimised(QMGeometry* qmg)
{
	if(dim > 3) {
		std::cerr << "ERROR: QMStateAnalytic::prepareReturnStateDiscreteOptimised does not work with dim > 3\n";
		exit(1);
	}
	std::vector<size_t> gridSize(dim);
	std::vector<Real>   size(dim);
	for(size_t i=0 ; i<dim ; i++) {
		if(qmg->step[i]==0) { std::cerr << "ERROR: QMStateAnalytic::prepareReturnStateDiscreteOptimised: step is ZERO!\n"; exit(1);};
		size    [i]=(         qmg->halfSize[i]*2.0              );
		gridSize[i]=((size_t)(qmg->halfSize[i]*2.0/qmg->step[i]));
	}
	Scene* scene(Omega::instance().getScene().get());	// get scene
	if(    lastOptimisationIter == scene->iter and stateDiscreteOptimised
	   and stateDiscreteOptimised->gridSize==gridSize and stateDiscreteOptimised->size==size)
		return stateDiscreteOptimised;
	if(not stateDiscreteOptimised)
		stateDiscreteOptimised = boost::shared_ptr<QMStateDiscrete>(new QMStateDiscrete);

	stateDiscreteOptimised->firstRun = false;
	stateDiscreteOptimised->creator  = boost::shared_ptr<QMStateAnalytic>();
	stateDiscreteOptimised->dim      = dim;
	stateDiscreteOptimised->gridSize = gridSize;
	stateDiscreteOptimised->size     = size;
	stateDiscreteOptimised->calculateTableValuesPosition(this);

	lastOptimisationIter = scene->iter;

	return stateDiscreteOptimised;
};

