// 2015 © Janek Kozicki <cosurgi@gmail.com>

#include "QMPotentialBarrier.hpp"

#include <lib/time/TimeLimit.hpp>
TimeLimit timeLimitB; // FIXME - remove when finshed fixing

YADE_PLUGIN(
	(QMParametersBarrier)
	(QMStPotentialBarrier)
	(St1_QMStPotentialBarrier)
	(QMIPhysBarrier)
	(Ip2_QMParameters_QMParametersBarrier_QMIPhysBarrier)
	(Law2_QMIGeom_QMIPhysBarrier)
	);

/*********************************************************************************
*
* Q M   P O T E N T I A L   B A R R I E R   parameters         QMParametersBarrier
*
*********************************************************************************/
CREATE_LOGGER(QMParametersBarrier);
// !! at least one virtual function in the .cpp file
QMParametersBarrier::~QMParametersBarrier(){};

/*********************************************************************************
*
* Q M   S T A T E    B A R R I E R                            QMStPotentialBarrier
*
*********************************************************************************/
CREATE_LOGGER(QMStPotentialBarrier);
// !! at least one virtual function in the .cpp file
QMStPotentialBarrier::~QMStPotentialBarrier(){};

/*********************************************************************************
*
* Q M   S T A T E    B A R R I E R   F U N C T O R            QMStPotentialBarrier
*
*********************************************************************************/

Complexr St1_QMStPotentialBarrier::getValPos(Vector3r pos , const QMParameters* pm, const QMState* qms)
{
	//const QMStPotentialBarrier*        state = static_cast <const QMStPotentialBarrier*>(qms);
	const QMParametersBarrier* barrier = dynamic_cast<const QMParametersBarrier*>(pm);
	if(not barrier) { throw std::runtime_error("\n\nERROR: St1_QMStPotentialBarrier::getValPos() nas no QMParametersBarrier, but rather `"
		+std::string(pm?pm->getClassName():"")+"`.\n\n");};
	return barrier->height;
};

/*********************************************************************************
*
* Q M   interaction   P O T E N T I A L   B A R R I E R             QMIPhysBarrier
*
*********************************************************************************/
CREATE_LOGGER(QMIPhysBarrier);
// !! at least one virtual function in the .cpp file
QMIPhysBarrier::~QMIPhysBarrier(){};

/*********************************************************************************
*
* Ip2   QMParameters   QMParametersBarrier  →  QMIPhysBarrier
*
*********************************************************************************/

CREATE_LOGGER(Ip2_QMParameters_QMParametersBarrier_QMIPhysBarrier);

void Ip2_QMParameters_QMParametersBarrier_QMIPhysBarrier::go(
	  const shared_ptr<Material>& m1, const shared_ptr<Material>& m2
	, const shared_ptr<Interaction>& I)
{
	if(timeLimitB.messageAllowed(12)) std::cerr << "####### Ip2_QMParameters_QMParametersBarrier_QMIPhysBarrier::go  START!\n";

	shared_ptr<QMIPhysBarrier> pot;
	bool isNew = !I->phys;
	if(!isNew) pot=YADE_PTR_CAST<QMIPhysBarrier>(I->phys);
	else { pot=shared_ptr<QMIPhysBarrier>(new QMIPhysBarrier()); I->phys=pot; }

	Ip2_2xQMParameters_QMIPhys::go(m1,m2,I);

	//const QMParameters*        qm1 = static_cast<QMParameters*       >(m1.get());
	const QMParametersBarrier* qm2 = static_cast<QMParametersBarrier*>(m2.get());

// FIXME: it's only for display, so this should go to Gl1_QMIGeom or Gl1_QMIGeomHarmonic (?) or Gl1_QMIPhys or Gl1_QMIPhysHarmonic
//        but then - the potential itself shall be drawn just like before: as a Box ??
	pot->height = qm2->height;

// FIXME: create here QMIPhys::potentialInteractionGlobal
//        then call 
//		Ip2_2xQMParameters_QMIPhys::go(m1,m2,I);
//        to assign (or calcMarginalDistribution??? ← no, better leave that for Gl1_drawer), so I don't actually need to assign!
//		barrier->potentialInteractionGlobal->psiGlobalTable = val;

}

void Ip2_QMParameters_QMParametersBarrier_QMIPhysBarrier::goReverse(
	  const shared_ptr<Material>& m1, const shared_ptr<Material>& m2
	, const shared_ptr<Interaction>& I)
{
	if(timeLimitB.messageAllowed(12)) std::cerr << "####### Ip2_QMParameters_QMParametersBarrier_QMIPhysBarrier::goReverse  START!\n";
	I->swapOrder();
	go(m2,m1,I);
};

/*********************************************************************************
*
* Law2   QMIGeom + QMIPhysBarrier  :  B A R R I E R
*
*********************************************************************************/

#include <Eigen/Dense>
//#include <Eigen/src/Core/Functors.h>

CREATE_LOGGER(Law2_QMIGeom_QMIPhysBarrier);

bool Law2_QMIGeom_QMIPhysBarrier::go(shared_ptr<IGeom>& ig, shared_ptr<IPhys>& ip, Interaction* I)
{
	if(timeLimitB.messageAllowed(12)) std::cerr << "####### Law2_QMIGeom_QMIPhysBarrier::go  START!\n";
	
	QMIGeom*        qmigeom = static_cast<QMIGeom*       >(ig.get());
	QMIPhysBarrier* barrier = static_cast<QMIPhysBarrier*>(ip.get());

	// FIXME, but how?? I need this equation somehow.
	QMParametersBarrier FIXME_param;
	FIXME_param.dim=barrier->dim; FIXME_param.hbar = barrier->hbar; FIXME_param.height = barrier->height;
	St1_QMStPotentialBarrier FIXME_equation;

	//FIXME - how to avoid getting Body from scene?
	QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[I->id1]->state.get());
HERE;
//	NDimTable<Complexr>& val(qmigeom->potentialMarginalDistribution);

	if(not  barrier->potentialInteractionGlobal) {
		barrier->potentialInteractionGlobal = boost::shared_ptr<QMStateDiscreteGlobal>(new QMStateDiscreteGlobal);
		barrier->potentialInteractionGlobal->wasGenerated=false;
	}
		// FIXME ! - tensorProduct !!
HERE;
	NDimTable<Complexr>& val(barrier->potentialInteractionGlobal->psiGlobalTable);




	if(psi->gridSize.size() <= 3) {
// FIXME (1↓) problem zaczyna się tutaj, ponieważ robiąc resize tak żeby pasowały do siebie, zakładam jednocześnie że siatki się idealnie nakrywają.
//            hmm... ale nawet gdy mam iloczyn tensorowy to one muszą się idealnie nakrywać !
		val.resize(psi->getPsiGlobalExisting()->psiGlobalTable);
		val.fill1WithFunction( psi->gridSize.size()
			, [&](Real i, int d)->Real    { return psi->iToX(i,d) - qmigeom->relPos21[d];}           // xyz position function
			, [&](Vector3r& xyz)->Complexr{
				//Vector3r tmp = xyz.cwiseAbs() - qmigeom->extents2;
				//Vector3r zero(0,0,0);
				//if( tmp[0] <= 0 and tmp[1] <=0 and tmp[1] <= 0)
				// http://eigen.tuxfamily.org/dox/group__TutorialReductionsVisitorsBroadcasting.html
				if(  ((xyz.cwiseAbs() - qmigeom->extents2).array() <= Vector3r::Zero().array()).count() == 3  )
					return FIXME_equation.getValPos(xyz,&FIXME_param,NULL);                  // function value at xyz
				return 0;
			  }
			);
	} else { std::cerr << "\nLaw2_QMIGeom_QMIPhysBarrier::go, dim>3\n"; exit(1); };

// FIXME - this should go to Ip2_::go (parent, toplevel)
/*FIXME*///	if(not barrier->potentialInteractionGlobal)
/*FIXME*///		barrier->potentialInteractionGlobal = boost::shared_ptr<QMStateDiscreteGlobal>(new QMStateDiscreteGlobal);
/*FIXME*///		// FIXME ! - tensorProduct !!
/*FIXME*///	barrier->potentialInteractionGlobal->psiGlobalTable = val;

	return true;
};

