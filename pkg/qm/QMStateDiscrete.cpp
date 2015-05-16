// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMStateDiscrete.hpp"
#include "QMStateAnalytic.hpp"
#include "QMParameters.hpp"
#include "QMGeometry.hpp"
#include <core/Scene.hpp>

YADE_PLUGIN(
	(QMStateDiscrete)
	(St1_QMStateDiscrete)
	);

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   S T A T E
*
*********************************************************************************/
CREATE_LOGGER(QMStateDiscrete);
// !! at least one virtual function in the .cpp file
QMStateDiscrete::~QMStateDiscrete(){};

void St1_QMStateDiscrete::go(const shared_ptr<State>& state, const shared_ptr<Material>& mat, const Body* b)
{
	QMStateDiscrete* qmstate = static_cast <QMStateDiscrete*>(state.get());
	QMParameters*    par     = dynamic_cast<QMParameters*   >(mat.get());
	QMGeometry*      qmg     = dynamic_cast<QMGeometry*     >(b->shape.get());
	if(!qmstate or !par or !qmg) { std::cerr << "ERROR: St1_QMStateDiscrete::go : No state, no material. Cannot proceed."; exit(1);};
	size_t dim = par->dim;
	if(dim > 3) { throw std::runtime_error("\n\nERROR: St1_QMStateDiscrete::go does not work with dim > 3.\n\n");};
	std::vector<size_t> gridSizeNew(dim);
	std::vector<Real>   sizeNew(dim);
	for(size_t i=0 ; i<dim ; i++) {

	// FIXME - bardzo dziwne miejsce do resetowanie step() w Gl1_QMGeometry, przecież tym się zajmuje St1_QMStateDiscrete::go !!!
	//if(pd->gridSize.size() > 0) { g->step.x()=pd->stepInPositionalRepresentation(0); start.x() = pd->start(0); end.x() = pd->end(0)- g->step.x(); } else { return; }
	//if(pd->gridSize.size() > 1) { g->step.y()=pd->stepInPositionalRepresentation(1); start.y() = pd->start(1); end.y() = pd->end(1)- g->step.y(); }
	//if(pd->gridSize.size() > 2) { g->step.z()=pd->stepInPositionalRepresentation(2); start.z() = pd->start(2); end.z() = pd->end(2)- g->step.z(); }

		if(qmg->step[i]==0) { throw std::runtime_error("\n\nERROR: St1_QMStateDiscrete::go: step is ZERO!\n\n");};
		sizeNew    [i]=(         qmg->extents[i]*2.0              );
		gridSizeNew[i]=((size_t)(qmg->extents[i]*2.0/qmg->step[i]));
	}
	if( not b->isDynamic() ) { // anlytical
	// not dynamic means it's either pure analytical solution or a potential
		QMStateAnalytic*   stAn = dynamic_cast<QMStateAnalytic*>(state.get());
		if(!stAn) { std::cerr << "ERROR: St1_QMStateDiscrete::go : QMStateAnalytic not found."; exit(1);};
		if((stAn->lastOptimisationIter == scene->iter) and (gridSizeNew==qmstate->gridSize) and (sizeNew==qmstate->size))
			return;
		qmstate->firstRun = true; // so it is always generated from the analytical formula
		qmstate->gridSize = gridSizeNew;
		stAn->lastOptimisationIter = scene->iter;
	}
	// dynamic means that it takes part in calculations
	if(qmstate->firstRun) {
		qmstate->size     = sizeNew;
	}
	this->calculateTableValuesPosition(par,qmstate);
}

void St1_QMStateDiscrete::calculateTableValuesPosition(const QMParameters* par, QMStateDiscrete* qms)
{// initialize from this   //////////////////////////////////////////// MERGE
	if(not qms->firstRun) return;
	qms->firstRun=false;
	if(par->dim == 1) {
		if (qms->gridSize.size() != 1) throw std::out_of_range("\n\nQMStateDiscrete: should be dimension 1\n\n");
		qms->tableValuesPosition.resize(qms->gridSize,5); // initialize with obviously wrong value, eg. 5, so that mistakes are easy to spot
		for(size_t i=0 ; i<qms->gridSize[0] ; i++)
			qms->tableValuesPosition.at(i) = this->getValPos(Vector3r(qms->iToX(i,0),0,0),par,qms);
	} else if(par->dim == 2) {
		if (qms->gridSize.size() != 2) throw std::out_of_range("\n\nQMStateDiscrete: should be dimension 2\n\n");
		qms->tableValuesPosition.resize(qms->gridSize,5);  // initialize with obviously wrong value, eg. 5, so that mistakes are easy to spot
		for(size_t i=0 ; i<qms->gridSize[0] ; i++)
		for(size_t j=0 ; j<qms->gridSize[1] ; j++)
			qms->tableValuesPosition.at    ( i,j ) = this->getValPos(Vector3r(qms->iToX(i,0),qms->iToX(j,1),0),par,qms);
		//OK - that was just to be safe
		//	tableValuesPosition.atSafe({i,j}) = this->getValPos(Vector3r(iToX(i,0),iToX(j,1),0),par);
	} else if(par->dim == 3) {
		if (qms->gridSize.size() != 3) throw std::out_of_range("\n\nQMStateDiscrete: should be dimension 3\n\n");
		qms->tableValuesPosition.resize(qms->gridSize,5); // initialize with obviously wrong value, eg. 5, so that mistakes are easy to spot
		for(size_t i=0 ; i<qms->gridSize[0] ; i++)
		for(size_t j=0 ; j<qms->gridSize[1] ; j++)
		for(size_t k=0 ; k<qms->gridSize[2] ; k++)
			qms->tableValuesPosition.at    ( i,j,k ) = this->getValPos(Vector3r(qms->iToX(i,0),qms->iToX(j,1),qms->iToX(k,2)),par,qms);
	} else {
		throw std::runtime_error("\n\nQMStateDiscrete() supports only 1,2 or 3 dimensions, so far.\n\n");
	}
};

