// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMPotential.hpp"
#include "QMBody.hpp"
#include "QMStateDiscrete.hpp"
#include "QMStateDiscreteGlobal.hpp"
#include "QMStateAnalytic.hpp"
#include "QMParameters.hpp"
#include "QMGeometry.hpp"
#include <core/Scene.hpp>

#include<lib/serialization/ObjectIO.hpp>

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

std::complex<Real> QMStateDiscrete::braKet(boost::shared_ptr<QMStateDiscrete> o)
{
	if( not (getPsiGlobalExists() and o->getPsiGlobalExists()) ) {
		std::cerr << "ERROR braKet(other): psiGlobal doesn't exist\n";
		return 0;
	}
	if( getPsiGlobalExisting()->getSpatialSizeGlobal() != o->getPsiGlobalExisting()->getSpatialSizeGlobal() ) {
		std::cerr << "ERROR braKet(other): different spatial sizes: "<<getPsiGlobalExisting()->getSpatialSizeGlobal()<<" and "<<o->getPsiGlobalExisting()->getSpatialSizeGlobal()<< "\n";
		return 0;
	}
	return getPsiGlobalExisting()->psiGlobalTable.integrateWithOther_BraKet(o->getPsiGlobalExisting()->psiGlobalTable, getPsiGlobalExisting()->getSpatialSizeGlobal());
};

std::vector<Real> QMStateDiscrete::search(boost::shared_ptr<QMStateDiscrete> o)
{
	
	if( not (getPsiGlobalExists() and o->getPsiGlobalExists()) ) {
		std::cerr << "ERROR search(other): psiGlobal doesn't exist\n";
		return {};
	}
	if( getPsiGlobalExisting()->getSpatialSizeGlobal() != o->getPsiGlobalExisting()->getSpatialSizeGlobal() ) {
		std::cerr << "ERROR search(other): different spatial sizes: "<<getPsiGlobalExisting()->getSpatialSizeGlobal()<<" and "<<o->getPsiGlobalExisting()->getSpatialSizeGlobal()<< "\n";
		return {};
	}
	return getPsiGlobalExisting()->psiGlobalTable.integrateWithOther_search(o->getPsiGlobalExisting()->psiGlobalTable, getPsiGlobalExisting()->getSpatialSizeGlobal());
};

std::vector<Real> QMStateDiscrete::searchRange(boost::shared_ptr<QMStateDiscrete> o,std::vector<Real> start,std::vector<Real> end)
{
	
	if( not (getPsiGlobalExists() and o->getPsiGlobalExists()) ) {
		std::cerr << "ERROR search(other): psiGlobal doesn't exist\n";
		return {};
	}
	if( getPsiGlobalExisting()->getSpatialSizeGlobal() != o->getPsiGlobalExisting()->getSpatialSizeGlobal() ) {
		std::cerr << "ERROR search(other): different spatial sizes: "<<getPsiGlobalExisting()->getSpatialSizeGlobal()<<" and "<<o->getPsiGlobalExisting()->getSpatialSizeGlobal()<< "\n";
		return {};
	}
	return getPsiGlobalExisting()->psiGlobalTable.integrateWithOther_searchRange(start,end,o->getPsiGlobalExisting()->psiGlobalTable, getPsiGlobalExisting()->getSpatialSizeGlobal());
};

void QMStateDiscrete::zeroRange(std::vector<Real> start,std::vector<Real> end, bool outside,bool debug)
{
	
	if( not (getPsiGlobalExists() ) ) {
		std::cerr << "ERROR: psiGlobal doesn't exist\n";
		return;
	}
	getPsiGlobalExisting()->psiGlobalTable.zeroRange(start,end,getPsiGlobalExisting()->getSpatialSizeGlobal(),outside,debug);
	return;
};

std::complex<Real> QMStateDiscrete::searchAt(boost::shared_ptr<QMStateDiscrete> o,std::vector<size_t> where)
{
	
	if( not (getPsiGlobalExists() and o->getPsiGlobalExists()) ) {
		std::cerr << "ERROR search(other): psiGlobal doesn't exist\n";
		return {};
	}
	if( getPsiGlobalExisting()->getSpatialSizeGlobal() != o->getPsiGlobalExisting()->getSpatialSizeGlobal() ) {
		std::cerr << "ERROR search(other): different spatial sizes: "<<getPsiGlobalExisting()->getSpatialSizeGlobal()<<" and "<<o->getPsiGlobalExisting()->getSpatialSizeGlobal()<< "\n";
		return {};
	}
	return getPsiGlobalExisting()->psiGlobalTable.integrateWithOther_searchAt(where,o->getPsiGlobalExisting()->psiGlobalTable, getPsiGlobalExisting()->getSpatialSizeGlobal());
};

boost::shared_ptr<QMStateDiscrete> QMStateDiscrete::subtract(boost::shared_ptr<QMStateDiscrete> other)
{
	if( not (getPsiGlobalExists() and other->getPsiGlobalExists()) ) {
		std::cerr << "ERROR subtract(other): psiGlobal doesn't exist\n";
		return boost::shared_ptr<QMStateDiscrete>();
	}
	if( getPsiGlobalExisting()->getSpatialSizeGlobal() != other->getPsiGlobalExisting()->getSpatialSizeGlobal() ) {
		std::cerr << "ERROR subtract(other): different spatial sizes: "<<getPsiGlobalExisting()->getSpatialSizeGlobal()<<" and "<<other->getPsiGlobalExisting()->getSpatialSizeGlobal()<< "\n";
		return boost::shared_ptr<QMStateDiscrete>();
	}
	if(    (
		     (getPsiGlobalExisting()->psiGlobalTable.rank() != other->getPsiGlobalExisting()->psiGlobalTable.rank()) 
		 or  (getPsiGlobalExisting()->psiGlobalTable.dim()  != other->getPsiGlobalExisting()->psiGlobalTable.dim() ) 
		 or  (getPsiGlobalExisting()->psiGlobalTable.size_total()  != other->getPsiGlobalExisting()->psiGlobalTable.size_total() ) 
		)) {
		std::cerr << "\n\n ERROR: bad sizes of two wavefunctions:\n";
		std::cerr << " spatial_sizes : " << getPsiGlobalExisting()->getSpatialSizeGlobal()        << "\n";
		std::cerr << " rank_d        :( " << getPsiGlobalExisting()->psiGlobalTable.rank()        <<" ) vs. ( " << other->getPsiGlobalExisting()->psiGlobalTable.rank()       <<" ) \n";
		std::cerr << " dim_n         :( " << getPsiGlobalExisting()->psiGlobalTable.dim()         <<" ) vs. ( " << other->getPsiGlobalExisting()->psiGlobalTable.dim()        <<" ) \n";
		std::cerr << " total         :( " << getPsiGlobalExisting()->psiGlobalTable.size_total()  <<" ) vs. ( " << other->getPsiGlobalExisting()->psiGlobalTable.size_total() <<" ) \n";
		return boost::shared_ptr<QMStateDiscrete>();
	}
	auto ret = boost::shared_ptr<QMStateDiscrete>(new QMStateDiscrete);
	ret->spatialSize          = this->spatialSize;
	ret->whichPartOfpsiGlobal = this->whichPartOfpsiGlobal;

	ret->getPsiGlobalNew()      ->psiGlobalTable  = this ->getPsiGlobalExisting()->psiGlobalTable;
	ret->getPsiGlobalExisting() ->psiGlobalTable -= other->getPsiGlobalExisting()->psiGlobalTable;
	ret->getPsiGlobalExisting() ->setSpatialSizeGlobal( this->getPsiGlobalExisting()->getSpatialSizeGlobal() );
	return ret;
};

boost::shared_ptr<QMStateDiscrete> QMStateDiscrete::copy()
{
	if( not (getPsiGlobalExists())) {
		std::cerr << "ERROR copy(): psiGlobal doesn't exist\n";
		return boost::shared_ptr<QMStateDiscrete>();
	}
	auto ret = boost::shared_ptr<QMStateDiscrete>(new QMStateDiscrete);
	ret->spatialSize          = this->spatialSize;
	ret->whichPartOfpsiGlobal = this->whichPartOfpsiGlobal;

	ret->getPsiGlobalNew()      ->psiGlobalTable  = this ->getPsiGlobalExisting()->psiGlobalTable;
	ret->getPsiGlobalExisting() ->setSpatialSizeGlobal( this->getPsiGlobalExisting()->getSpatialSizeGlobal() );
	return ret;
};

boost::shared_ptr<QMStateDiscrete> QMStateDiscrete::load(std::string fname)
{
	auto ret = boost::shared_ptr<QMStateDiscrete>(new QMStateDiscrete);
	
	if(fname.size()==0) throw runtime_error("empty filename !.");
	yade::ObjectIO::load(fname,"ret",ret);
//	ret->spatialSize          = this->spatialSize;
//	ret->whichPartOfpsiGlobal = this->whichPartOfpsiGlobal;
//
//	ret->getPsiGlobalNew()      ->psiGlobalTable  = this ->getPsiGlobalExisting()->psiGlobalTable;
//	ret->getPsiGlobalExisting() ->setSpatialSizeGlobal( this->getPsiGlobalExisting()->getSpatialSizeGlobal() );
	return ret;
};

void QMStateDiscrete::save(std::string fname)
{
	if( not (getPsiGlobalExists())) {
		std::cerr << "ERROR copy(): psiGlobal doesn't exist\n";
		return;
	}
	if(fname.size()==0) throw runtime_error("empty filename !.");

	auto ret = boost::shared_ptr<QMStateDiscrete>(new QMStateDiscrete);
	ret->spatialSize          = this->spatialSize;
	ret->whichPartOfpsiGlobal = this->whichPartOfpsiGlobal;

	ret->getPsiGlobalNew()      ->psiGlobalTable  = this ->getPsiGlobalExisting()->psiGlobalTable;
	ret->getPsiGlobalExisting() ->setSpatialSizeGlobal( this->getPsiGlobalExisting()->getSpatialSizeGlobal() );

	yade::ObjectIO::save(fname,"ret",ret);
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
//HERE2;
		this->calculateTableValuesPosition(par,dynamic_cast<QMState*>(qms));
		try {
			qms->setMaterialAndGenerator(par,shared_from_this());
		} catch(const boost::bad_weak_ptr& e) {
			// FIXME - serialization of StateDispatcher gone wrong?
			std::cerr << "St1_QMStateDiscrete::go() exception boost::bad_weak_ptr, there's no shared_ptr to this="<<this<<" class. FIXME:I see it in StateDispatcher, why??. Serialization gone wrong?\n";
		}
	} else if(qms->isAnalytic() and this->changesWithTime()) {
//HERE2;
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
bool really_needs_recalculation=false;
	QMStateDiscrete* qms = dynamic_cast<QMStateDiscrete*>(qm);
	if(not qms) {std::cerr<< "\nERROR:no QMStateDiscrete\n\n"; exit(1);};
	if(not qms->wasGenerated) {
		qms->wasGenerated=true;
		if(par->dim <= 3) {
			if (qms->gridSize.size() != par->dim) {
				std::cerr<< "\nERROR: St1_QMStateDiscrete::calculateTableValuesPosition doesn't have gridSize\n";
				exit(1);
			}
if((not qms->getPsiGlobalExists()) or qms->getPsiGlobalExisting()->psiGlobalTable.rank() == par->dim ) {
really_needs_recalculation = true;
			qms->getPsiGlobalNew()     ->psiGlobalTable.resize(qms->gridSize);
//HERE2;
			qms->getPsiGlobalExisting()->psiGlobalTable.fill1WithFunction( par->dim
				, [&](Real i, int d)->Real    { return qms->iToX(i,d);}                       // xyz position function
				, [&](Vector3r& xyz)->Complexr{ return this->getValPos(xyz,par.get(),qms);}   // function value at xyz
				);
}
		} else {
			throw std::runtime_error("\n\nQMStateDiscrete() supports only 1,2 or 3 dimensions, so far.\n\n");
		}
//FIXME - jak skończę (ale co? wywalać psiGlobal?) to będą wszystkie poniższe niepotrzebne!
// FIXME --- muszę regenerować bool Law2_QMIGeom_QMIPhysCoulombParticlesFree::go(shared_ptr<IGeom>& ig, shared_ptr<IPhys>& ip, Interaction* I)
if(really_needs_recalculation) {
/*FIXING?*/	qms->getPsiGlobalExisting()->wasGenerated = true;
/*FIXING?*/	qms->getPsiGlobalExisting()->gridSize = qms->gridSize;
/*FIXING?*/	qms->getPsiGlobalExisting()->setSpatialSizeGlobal(qms->getSpatialSize());
}
	}
///    else {//                    LICZENIE marginalDistribution POSZŁO DO Gl1_NDimTable
};

