// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMStateDiscrete.hpp"
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
	QMStateDiscrete* stDi = static_cast<QMStateDiscrete*>(state.get());
	QMParameters*    par  = dynamic_cast<QMParameters*   >(mat.get());
	if(not par) { throw std::runtime_error("ERROR: St1_QMStateDiscrete nas no QMParameters, but rather `"+std::string(mat?mat->getClassName():"")+"`");};

	shared_ptr<StateDispatcher> st;
	FOREACH(shared_ptr<Engine>& e, scene->engines){ st=YADE_PTR_DYN_CAST<StateDispatcher>(e); if(st) break; }
	if(!st) { throw std::runtime_error("St1_QMStateDiscrete::go : StateDispatcher is missing.\n"); };
	shared_ptr<St1_QMStateAnalytic> fa;
	FOREACH(auto& f, st->functors){ if(f->get1DFunctorType1() == stDi->creator->getClassName() ) { fa=YADE_PTR_DYN_CAST<St1_QMStateAnalytic>(f); break; } }
	if(not fa) { throw std::runtime_error("ERROR: St1_QMStateDiscrete::go can't find St1_"+stDi->creator->getClassName()+"() for its `creator` in StateDispatcher([...]). Did you forget to add it there?\n"); };

	stDi->calculateTableValuesPosition(fa.get(),par,stDi->creator.get());
}

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
void QMStateDiscrete::calculateTableValuesPosition(St1_QMStateAnalytic* localCreator, const QMParameters* par, const QMStateAnalytic* qms)
{// initialize from localCreator
	if(not firstRun) return;
	firstRun=false;
	if(par->dim == 1) {
		if (gridSize.size() != 1) throw std::out_of_range("QMStateDiscrete: should be dimension 1\n");
		tableValuesPosition.resize(gridSize,5); // initialize with obviously wrong value, eg. 5, so that mistakes are easy to spot
		for(size_t i=0 ; i<gridSize[0] ; i++)
			tableValuesPosition.at(i) = localCreator->getValPos(Vector3r(iToX(i,0),0,0),par,qms);
	} else if(par->dim == 2) {
		if (gridSize.size() != 2) throw std::out_of_range("QMStateDiscrete: should be dimension 2\n");
		tableValuesPosition.resize(gridSize,5);  // initialize with obviously wrong value, eg. 5, so that mistakes are easy to spot
		for(size_t i=0 ; i<gridSize[0] ; i++)
		for(size_t j=0 ; j<gridSize[1] ; j++)
			tableValuesPosition.at    ( i,j ) = localCreator->getValPos(Vector3r(iToX(i,0),iToX(j,1),0),par,qms);
		//OK - that was just to be safe
		//	tableValuesPosition.atSafe({i,j}) = localCreator->getValPos(Vector3r(iToX(i,0),iToX(j,1),0),par);
	} else if(par->dim == 3) {
		if (gridSize.size() != 3) throw std::out_of_range("QMStateDiscrete: should be dimension 3\n");
		tableValuesPosition.resize(gridSize,5); // initialize with obviously wrong value, eg. 5, so that mistakes are easy to spot
		for(size_t i=0 ; i<gridSize[0] ; i++)
		for(size_t j=0 ; j<gridSize[1] ; j++)
		for(size_t k=0 ; k<gridSize[2] ; k++)
			tableValuesPosition.at    ( i,j,k ) = localCreator->getValPos(Vector3r(iToX(i,0),iToX(j,1),iToX(k,2)),par,qms);
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

