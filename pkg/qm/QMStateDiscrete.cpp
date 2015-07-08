// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMBody.hpp"
#include "QMStateDiscrete.hpp"
#include "QMStateDiscreteGlobal.hpp"
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
boost::shared_ptr<QMStateDiscreteGlobal>& QMStateDiscrete::getPsiGlobalNew()
{
	if(not psiGlobal)
		psiGlobal = boost::shared_ptr<QMStateDiscreteGlobal>(new QMStateDiscreteGlobal);
	return psiGlobal;
};

Complexr QMStateDiscrete::atPsiGlobalExisting(std::vector<size_t> pos)
{
	return getPsiGlobalExisting()->psiGlobalTable.at(pos);
};

Real QMStateDiscrete::integratePsiGlobal()
{
	return getPsiGlobalExisting()->psiGlobalTable.integrateAllNormSquared(getPsiGlobalExisting()->getSpatialSizeGlobal());
};

void St1_QMStateDiscrete::go(const shared_ptr<State>& state, const shared_ptr<Material>& mat, const Body* b)
{
	QMStateDiscrete*         qms = static_cast <QMStateDiscrete*>(state.get());
	shared_ptr<QMParameters> par = YADE_PTR_DYN_CAST<QMParameters>(mat);
	QMGeometry*              qmg = dynamic_cast<QMGeometry*     >(b->shape.get());
	if(!par or !qmg) { std::cerr << "ERROR: St1_QMStateDiscrete::go : No material, no geometry. Cannot proceed."; exit(1);};
	size_t dim = par->dim; if(dim > 3) { throw std::runtime_error("\n\nERROR: St1_QMStateDiscrete::go does not work with dim > 3.\n\n");};
	if(not qms->wasGenerated) {
		std::vector<Real>   sizeNew(dim);
		for(size_t i=0 ; i<dim ; i++) sizeNew[i]=(qmg->extents[i]*2.0);
		qms->setSpatialSize(sizeNew);
		if(qms->isAnalytic() and qms->gridSize.size() == 0) {
			qms->gridSize.resize(par->dim,7); // fist run on analytic may need to pick arbitrarily some gridSize, if it wasn't defined by user
			// use '7' as default to make it apparent        ↓ if it was defined, then use it
			for(size_t i=0 ; i < qms->gridSize.size() ; i++) if(qmg->step[i]>0) qms->gridSize[i]=size_t(qms->getSpatialSize()[i]/qmg->step[i]);
			// need to recalculate step, because of rounding errors for gridSize
			for(size_t i=0 ; i < qms->gridSize.size() ; i++) qmg->step[i]=qms->getSpatialSize()[i]/((Real)(qms->gridSize[i]));
		}
		this->calculateTableValuesPosition(par,dynamic_cast<QMState*>(qms));
		qms->setMaterialAndGenerator(par,shared_from_this());
	} else if(qms->isAnalytic()) {
		qms->wasGenerated = false;// keep on calculating, since it is analytic
		this->calculateTableValuesPosition(par,qms);
	}
}

void St1_QMStateDiscrete::calculateTableValuesPosition(const shared_ptr<QMParameters>& par, QMState* qm)
{
	// ↓ patrz też uwagi w QMState.hpp
	// FIXME - aha, chyba wywalenie psiGlobal będzie polegało na tym, że to co było QMStateDiscrete teraz będzie QMState, ale bez eKin(), eMax() itp
	// z kolei QMState będzie zawierało QMStateDiscreteGlobal z tymi wszystkimi eKin(), NDimTable itp.
	// A póki co muszę zrobić taki cast:
	QMStateDiscrete* qms = dynamic_cast<QMStateDiscrete*>(qm);
	if(not qms) {std::cerr<< "\nERROR:no QMStateDiscrete\n\n"; exit(1);};
	if(not qms->wasGenerated) {
		qms->wasGenerated=true;
		if(par->dim <= 3) {
			if (qms->gridSize.size() != par->dim) {
				std::cerr<< "\nERROR: St1_QMStateDiscrete::calculateTableValuesPosition doesn't have gridSize\n";
				exit(1);
			}
			qms->getPsiGlobalNew()     ->psiGlobalTable.resize(qms->gridSize);
			qms->getPsiGlobalExisting()->psiGlobalTable.fill1WithFunction( par->dim
				, [&](Real i, int d)->Real    { return qms->iToX(i,d);}                       // xyz position function
				, [&](Vector3r& xyz)->Complexr{ return this->getValPos(xyz,par.get(),qms);}   // function value at xyz
				);
		} else {
			throw std::runtime_error("\n\nQMStateDiscrete() supports only 1,2 or 3 dimensions, so far.\n\n");
		}
//FIXME - jak skończę (ale co? wywalać psiGlobal?) to będą wszystkie poniższe niepotrzebne!
// FIXME --- muszę regenerować bool Law2_QMIGeom_QMIPhysCoulombParticlesFree::go(shared_ptr<IGeom>& ig, shared_ptr<IPhys>& ip, Interaction* I)
/*FIXING?*/	qms->getPsiGlobalExisting()->wasGenerated = true;
/*FIXING?*/	qms->getPsiGlobalExisting()->gridSize = qms->gridSize;
/*FIXING?*/	qms->getPsiGlobalExisting()->setSpatialSizeGlobal(qms->getSpatialSize());
	}
///    else {//                    LICZENIE marginalDistribution POSZŁO DO Gl1_NDimTable
};

