// 2015 © Janek Kozicki <cosurgi@gmail.com>

#include "QMPotentialBarrier.hpp"

#include <lib/time/TimeLimit.hpp>
TimeLimit timeLimitB; // FIXME - remove when finshed fixing

YADE_PLUGIN(
	(QMParametersBarrier)
	(QMStateBarrier)
	(St1_QMStateBarrier)
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
* Q M   S T A T E    B A R R I E R                                  QMStateBarrier
*
*********************************************************************************/
CREATE_LOGGER(QMStateBarrier);
// !! at least one virtual function in the .cpp file
QMStateBarrier::~QMStateBarrier(){};

/*********************************************************************************
*
* Q M   S T A T E    B A R R I E R   F U N C T O R                  QMStateBarrier
*
*********************************************************************************/

Complexr St1_QMStateBarrier::getValPos(Vector3r pos , const QMParameters* pm, const QMState* qms)
{
	//const QMStateBarrier*        state = static_cast <const QMStateBarrier*>(qms);
	const QMParametersBarrier* barrier = dynamic_cast<const QMParametersBarrier*>(pm);
	if(not barrier) { throw std::runtime_error("\n\nERROR: St1_QMStateBarrier::getValPos() nas no QMParametersBarrier, but rather `"
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

CREATE_LOGGER(Law2_QMIGeom_QMIPhysBarrier);

bool Law2_QMIGeom_QMIPhysBarrier::go(shared_ptr<IGeom>& g, shared_ptr<IPhys>& p, Interaction* I)
{
	if(timeLimitB.messageAllowed(12)) std::cerr << "####### Law2_QMIGeom_QMIPhysBarrier::go  START!\n";
	
	QMIGeom*        qmigeom = static_cast<QMIGeom*       >(g.get());
	QMIPhysBarrier* barrier = static_cast<QMIPhysBarrier*>(p.get());
	
	//FIXME
	QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[I->id1]->state.get());
	NDimTable<Complexr>& val(qmigeom->potentialValues);
	val.resize(psi->tableValuesPosition,0);
	if(psi->gridSize.size()==1) {
		size_t startI=psi->xToI(qmigeom->relPos21[0]-qmigeom->extents2[0],0);
		size_t endI  =psi->xToI(qmigeom->relPos21[0]+qmigeom->extents2[0],0);
		for(size_t i=startI ; i<=endI ; i++) {
			if(i>=0 and i<val.size0(0))
				val.at(i)=barrier->height;
		}
	}
	if(psi->gridSize.size()==2) {
		size_t startI=psi->xToI(qmigeom->relPos21[0]-qmigeom->extents2[0],0);
		size_t endI  =psi->xToI(qmigeom->relPos21[0]+qmigeom->extents2[0],0);
		size_t startJ=psi->xToI(qmigeom->relPos21[1]-qmigeom->extents2[1],1);
		size_t endJ  =psi->xToI(qmigeom->relPos21[1]+qmigeom->extents2[1],1);

		for(size_t i=startI ; i<=endI ; i++)
		for(size_t j=startJ ; j<=endJ ; j++)
		{
			if(i>=0 and i<val.size0(0))
			if(j>=0 and j<val.size0(1))
				val.at(i,j)=barrier->height;
		}
	}
	if(psi->gridSize.size() > 2) { std::cerr << "Law2_QMIGeom_QMIPhysBarrier::go, dim>2"; exit(1); };
	return true;
};

