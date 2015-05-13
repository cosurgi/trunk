// 2014 © Janek Kozicki <cosurgi@gmail.com>

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

void St1_QMStateAnalytic::go(const shared_ptr<State>& state, const shared_ptr<Material>& mat, const Body* b)
{
	QMStateAnalytic*   stAn = dynamic_cast<QMStateAnalytic*>(state.get());
	QMParameters*      par  = dynamic_cast<QMParameters*   >(mat.get());
	QMGeometry*        qmg  = dynamic_cast<QMGeometry*     >(b->shape.get());

	if(!stAn or !par or !qmg) { std::cerr << "ERROR: No state, no material. Cannot proceed."; exit(1);};

	size_t dim = par->dim;
	if(dim > 3) { throw std::runtime_error("ERROR: St1_QMStateAnalytic::go does not work with dim > 3.");};
	std::vector<size_t> gridSize(dim);
	std::vector<Real>   size(dim);
	for(size_t i=0 ; i<dim ; i++) {
		if(qmg->step[i]==0) { throw std::runtime_error("ERROR: St1_QMStateAnalytic::go: step is ZERO!");};
		size    [i]=(         qmg->extents[i]*2.0              );
		gridSize[i]=((size_t)(qmg->extents[i]*2.0/qmg->step[i]));
	}
	if(    (stAn->lastOptimisationIter == scene->iter) and (stAn->stateDiscreteOptimised)
	   and (stAn->stateDiscreteOptimised->gridSize==gridSize) and (stAn->stateDiscreteOptimised->size==size))
		return;
	if(not stAn->stateDiscreteOptimised)
		stAn->stateDiscreteOptimised = boost::shared_ptr<QMStateDiscrete>(new QMStateDiscrete);

	stAn->stateDiscreteOptimised->firstRun = true;
	stAn->stateDiscreteOptimised->creator  = boost::shared_ptr<QMStateAnalytic>();
	stAn->stateDiscreteOptimised->gridSize = gridSize;
	stAn->stateDiscreteOptimised->size     = size;
	stAn->stateDiscreteOptimised->calculateTableValuesPosition(this,par,stAn);

	stAn->lastOptimisationIter = scene->iter;
};

