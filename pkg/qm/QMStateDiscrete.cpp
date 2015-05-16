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
/////////////////////////////////////// St1_QMStateAnalytic
	// dynamic means that it takes part in calculations
	// not dynamic means it's either pure analytical solution or a potential
	if( not b->isDynamic() ) { // anlytical
		QMStateAnalytic*   stAn = dynamic_cast<QMStateAnalytic*>(state.get());
		if(!stAn) { std::cerr << "ERROR: St1_QMStateDiscrete::go : QMStateAnalytic not found."; exit(1);};
		size_t dim = par->dim;
		if(dim > 3) { throw std::runtime_error("ERROR: St1_QMStateAnalytic::go does not work with dim > 3.");};
		std::vector<size_t> gridSizeNew(dim);
		std::vector<Real>   sizeNew(dim);
		for(size_t i=0 ; i<dim ; i++) {
			if(qmg->step[i]==0) { throw std::runtime_error("ERROR: St1_QMStateAnalytic::go: step is ZERO!");};
			sizeNew    [i]=(         qmg->extents[i]*2.0              );
			gridSizeNew[i]=((size_t)(qmg->extents[i]*2.0/qmg->step[i]));
		}
		if(    (stAn->lastOptimisationIter == scene->iter) 
		   and (gridSizeNew==qmstate->gridSize) and (sizeNew==qmstate->size))
			return;
		qmstate->firstRun = true;
		qmstate->gridSize = gridSizeNew;
		qmstate->size     = sizeNew;
		this->calculateTableValuesPosition(par,stAn);
		stAn->lastOptimisationIter = scene->iter;
/////////////////////////////////////// St1_QMStateAnalytic
	} else { // discrete

	// to było zanim zrobiłem St1_QMStateDiscrete ←⋯← St1_QMPacketGaussianWave         shared_ptr<StateDispatcher> st;
	// to było zanim zrobiłem St1_QMStateDiscrete ←⋯← St1_QMPacketGaussianWave         FOREACH(shared_ptr<Engine>& e, scene->engines){ st=YADE_PTR_DYN_CAST<StateDispatcher>(e); if(st) break; }
	// to było zanim zrobiłem St1_QMStateDiscrete ←⋯← St1_QMPacketGaussianWave         if(!st) { throw std::runtime_error("St1_QMStateDiscrete::go : StateDispatcher is missing.\n"); };
	// to było zanim zrobiłem St1_QMStateDiscrete ←⋯← St1_QMPacketGaussianWave         shared_ptr<St1_QMStateAnalytic> fa;
	// to było zanim zrobiłem St1_QMStateDiscrete ←⋯← St1_QMPacketGaussianWave         FOREACH(auto& f, st->functors){ if(f->get1DFunctorType1() == qmstate->creator->getClassName() ) { fa=YADE_PTR_DYN_CAST<St1_QMStateAnalytic>(f); break; } }
	// to było zanim zrobiłem St1_QMStateDiscrete ←⋯← St1_QMPacketGaussianWave         if(not fa) { throw std::runtime_error("ERROR: St1_QMStateDiscrete::go can't find St1_"+qmstate->creator->getClassName()+"() for its `creator` in StateDispatcher([...]). Did you forget to add it there?\n"); };

		this->calculateTableValuesPosition(par,qmstate);

	}
} /////////////////////////////////////////     MERGE

// FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME wywalić to do St1_*
//
// może w ten sposób, że:             QMState
//                                       ↑
//                                QMStateDiscrete     
//                                       ↑                ??
//                                QMStateAnalytic
//                                   ↑        ↑
//                  QMPacketGaussianWave  QMPacketHarmonicEigenFunc        
//


void St1_QMStateDiscrete::calculateTableValuesPosition(const QMParameters* par, QMStateDiscrete* qms)
{// initialize from this   //////////////////////////////////////////// MERGE
	if(not qms->firstRun) return;
	qms->firstRun=false;
	if(par->dim == 1) {
		if (qms->gridSize.size() != 1) throw std::out_of_range("QMStateDiscrete: should be dimension 1\n");
		qms->tableValuesPosition.resize(qms->gridSize,5); // initialize with obviously wrong value, eg. 5, so that mistakes are easy to spot
		for(size_t i=0 ; i<qms->gridSize[0] ; i++)
			qms->tableValuesPosition.at(i) = this->getValPos(Vector3r(qms->iToX(i,0),0,0),par,qms);
	} else if(par->dim == 2) {
		if (qms->gridSize.size() != 2) throw std::out_of_range("QMStateDiscrete: should be dimension 2\n");
		qms->tableValuesPosition.resize(qms->gridSize,5);  // initialize with obviously wrong value, eg. 5, so that mistakes are easy to spot
		for(size_t i=0 ; i<qms->gridSize[0] ; i++)
		for(size_t j=0 ; j<qms->gridSize[1] ; j++)
			qms->tableValuesPosition.at    ( i,j ) = this->getValPos(Vector3r(qms->iToX(i,0),qms->iToX(j,1),0),par,qms);
		//OK - that was just to be safe
		//	tableValuesPosition.atSafe({i,j}) = this->getValPos(Vector3r(iToX(i,0),iToX(j,1),0),par);
	} else if(par->dim == 3) {
		if (qms->gridSize.size() != 3) throw std::out_of_range("QMStateDiscrete: should be dimension 3\n");
		qms->tableValuesPosition.resize(qms->gridSize,5); // initialize with obviously wrong value, eg. 5, so that mistakes are easy to spot
		for(size_t i=0 ; i<qms->gridSize[0] ; i++)
		for(size_t j=0 ; j<qms->gridSize[1] ; j++)
		for(size_t k=0 ; k<qms->gridSize[2] ; k++)
			qms->tableValuesPosition.at    ( i,j,k ) = this->getValPos(Vector3r(qms->iToX(i,0),qms->iToX(j,1),qms->iToX(k,2)),par,qms);
	} else {
		throw std::runtime_error("QMStateDiscrete() supports only 1,2 or 3 dimensions, so far.");
	}
};


//FIXME - a może tutaj kontrakcję??
//
//FIXME, delete /// return complex quantum aplitude at given positional representation coordinates
//FIXME, delete Complexr QMStateDiscrete::getValPos(Vector3r xyz, const QMParameters* par) // FIXME - should take   std::vector<Real> (probably? - maybe useful for 4D?)
//FIXME, delete {
//FIXME, delete 	switch(par->dim) {
//FIXME, delete 	// FIXME(2) - should instead give just `const ref&` to this table, but GLDraw has problem - draws one too much!!
//FIXME, delete 	// FIXME - must work for all dimensions, with contractions !!!!!!
//FIXME, delete 		case 1 : return tableValuesPosition.atSafe({xToI(xyz[0],0)                              }); break;
//FIXME, delete 		case 2 : return tableValuesPosition.atSafe({xToI(xyz[0],0),xToI(xyz[1],1)               }); break;
//FIXME, delete 		case 3 : return tableValuesPosition.atSafe({xToI(xyz[0],0),xToI(xyz[1],1),xToI(xyz[2],2)}); break;
//FIXME, delete 		default: throw std::runtime_error("Wrong dimension! This code in QMStateDiscrete::getValPos should be unreachable");
//FIXME, delete 	}
//FIXME, delete };

