// 2015 © Janek Kozicki <cosurgi@gmail.com>

#include "QMPotentialHarmonic.hpp"

#include <lib/time/TimeLimit.hpp>
TimeLimit timeLimitH; // FIXME - remove when finshed fixing

YADE_PLUGIN(
	(QMParametersHarmonic)
	(QMParticleHarmonic)
	(QMStPotentialHarmonic)
	(St1_QMStPotentialHarmonic)
	(QMIPhysHarmonic)
	(QMIPhysHarmonicParticles)
	(Ip2_QMParameters_QMParametersHarmonic_QMIPhysHarmonic)
	(Ip2_QMParticleHarmonic_QMParticleHarmonic_QMIPhysHarmonicParticles)
	(Law2_QMIGeom_QMIPhysHarmonic)
	(Law2_QMIGeom_QMIPhysHarmonicParticles)
	);

/*********************************************************************************
*
* Q M   P O T E N T I A L   H A R M O N I C   parameters      QMParametersHarmonic
*
*********************************************************************************/
CREATE_LOGGER(QMParametersHarmonic);
// !! at least one virtual function in the .cpp file
QMParametersHarmonic::~QMParametersHarmonic(){};

/*********************************************************************************
*
* Q M   P A R T I C L E   with   H A R M O N I C   ,,charge''   QMParticleHarmonic
*
*********************************************************************************/
CREATE_LOGGER(QMParticleHarmonic);
// !! at least one virtual function in the .cpp file
QMParticleHarmonic::~QMParticleHarmonic(){};

/*********************************************************************************
*
* Q M   S T A T E    H A R M O N I C                         QMStPotentialHarmonic
*
*********************************************************************************/
CREATE_LOGGER(QMStPotentialHarmonic);
// !! at least one virtual function in the .cpp file
QMStPotentialHarmonic::~QMStPotentialHarmonic(){};

/*********************************************************************************
*
* Q M   S T A T E    H A R M O N I C   F U N C T O R         QMStPotentialHarmonic
*
*********************************************************************************/

Complexr St1_QMStPotentialHarmonic::getValPos(Vector3r pos , const QMParameters* pm, const QMState* qms)
{
	//const QMStPotentialHarmonic*        state = static_cast <const QMStPotentialHarmonic*>(qms);
	const QMParametersHarmonic* harmonic = dynamic_cast<const QMParametersHarmonic*>(pm);
	if(not harmonic) { throw std::runtime_error("\n\nERROR: St1_QMStPotentialHarmonic nas no QMParametersHarmonic, but rather `"
		+std::string(pm?pm->getClassName():"")+"`.\n\n");};
	Real x =pos[0]                  ,y =pos[1]                  ,z =pos[2];
	Real Cx=harmonic->coefficient[0],Cy=harmonic->coefficient[1],Cz=harmonic->coefficient[2];
	switch(harmonic->dim) {
// FIXME: it's only for display, so this should go to Gl1_QMIGeom or Gl1_QMIGeomHarmonic (?) or Gl1_QMIPhys or Gl1_QMIPhysHarmonic
//        but then - the potential itself shall be drawn just like before: as a Box ??
		case 1 : return Cx*x*x;
		case 2 : return Cx*x*x+Cy*y*y;
		case 3 : return Cx*x*x+Cy*y*y+Cz*z*z;
		default: break;
	};
	throw std::runtime_error("\n\nSt1_QMStPotentialHarmonic::getValPos - wrong number of dimensions.\n\n");
};

/*********************************************************************************
*
* Q M   interaction   H A R M O N I C   P O T E N T I A L          QMIPhysHarmonic
*
*********************************************************************************/
CREATE_LOGGER(QMIPhysHarmonic);
// !! at least one virtual function in the .cpp file
QMIPhysHarmonic::~QMIPhysHarmonic(){};

/*********************************************************************************
*
* Q M particle interaction via harmonic potential         QMIPhysHarmonicParticles
*
*********************************************************************************/
CREATE_LOGGER(QMIPhysHarmonicParticles);
// !! at least one virtual function in the .cpp file
QMIPhysHarmonicParticles::~QMIPhysHarmonicParticles(){};

/*********************************************************************************
*
* Ip2   QMParameters   QMParametersHarmonic  →  QMIPhysHarmonic
*
*********************************************************************************/

CREATE_LOGGER(Ip2_QMParameters_QMParametersHarmonic_QMIPhysHarmonic);

void Ip2_QMParameters_QMParametersHarmonic_QMIPhysHarmonic::go(
	  const shared_ptr<Material>& m1, const shared_ptr<Material>& m2
	, const shared_ptr<Interaction>& I)
{
	if(timeLimitH.messageAllowed(12)) std::cerr << "####### Ip2_QMParameters_QMParametersHarmonic_QMIPhysHarmonic::go  START!\n";

	shared_ptr<QMIPhysHarmonic> pot;
	bool isNew = !I->phys;
	if(!isNew) pot=YADE_PTR_CAST<QMIPhysHarmonic>(I->phys);
	else { pot=shared_ptr<QMIPhysHarmonic>(new QMIPhysHarmonic()); I->phys=pot; }

	Ip2_2xQMParameters_QMIPhys::go(m1,m2,I);

	//const QMParameters*        qm1 = static_cast<QMParameters*       >(m1.get());
	const QMParametersHarmonic* qm2 = static_cast<QMParametersHarmonic*>(m2.get());

	pot->coefficient = qm2->coefficient;
}

void Ip2_QMParameters_QMParametersHarmonic_QMIPhysHarmonic::goReverse(
	  const shared_ptr<Material>& m1, const shared_ptr<Material>& m2
	, const shared_ptr<Interaction>& I)
{
	std::cerr << "\n\n####### Ip2_QMParameters_QMParametersHarmonic_QMIPhysHarmonic::goReverse <----------------- START!\n\n";
	I->swapOrder();
	go(m2,m1,I);
};

/*********************************************************************************
*
* Ip2   QMParticleHarmonic   QMParticleHarmonic  →  QMIPhysHarmonicParticles
*
*********************************************************************************/

CREATE_LOGGER(Ip2_QMParticleHarmonic_QMParticleHarmonic_QMIPhysHarmonicParticles);

void Ip2_QMParticleHarmonic_QMParticleHarmonic_QMIPhysHarmonicParticles::go(
	  const shared_ptr<Material>& m1, const shared_ptr<Material>& m2
	, const shared_ptr<Interaction>& I)
{
	if(timeLimitH.messageAllowed(12)) std::cerr << "####### Ip2_QMParticleHarmonic_QMParticleHarmonic_QMIPhysHarmonicParticles::go  START!\n";

	shared_ptr<QMIPhysHarmonicParticles> pot;
	bool isNew = !I->phys;
	if(!isNew) pot=YADE_PTR_CAST<QMIPhysHarmonicParticles>(I->phys);
	else { pot=shared_ptr<QMIPhysHarmonicParticles>(new QMIPhysHarmonicParticles()); I->phys=pot; }

	Ip2_2xQMParameters_QMIPhys::go(m1,m2,I);

	const QMParticleHarmonic* qm1 = static_cast<QMParticleHarmonic*>(m1.get());
	const QMParticleHarmonic* qm2 = static_cast<QMParticleHarmonic*>(m2.get());

	pot->coefficient1 = qm1->coefficient;
	pot->coefficient2 = qm2->coefficient;
}

void Ip2_QMParticleHarmonic_QMParticleHarmonic_QMIPhysHarmonicParticles::goReverse(
	  const shared_ptr<Material>& m1, const shared_ptr<Material>& m2
	, const shared_ptr<Interaction>& I)
{
	std::cerr << "\n\n####### Ip2_QMParameters_QMParametersHarmonic_QMIPhysHarmonic::goReverse <-------------------- START!\n\n";
	I->swapOrder();
	go(m2,m1,I);
};

/*********************************************************************************
*
* Law2   QMIGeom + QMIPhysHarmonic  :  H A R M O N I C   P O T E N T I A L
*
*********************************************************************************/

CREATE_LOGGER(Law2_QMIGeom_QMIPhysHarmonic);

bool Law2_QMIGeom_QMIPhysHarmonic::go(shared_ptr<IGeom>& g, shared_ptr<IPhys>& p, Interaction* I)
{
	if(timeLimitH.messageAllowed(12)) std::cerr << "####### Law2_QMIGeom_QMIPhysHarmonic::go  START!\n";

	QMIGeom*         qmigeom  = static_cast<QMIGeom*        >(g.get());
	QMIPhysHarmonic* harmonic = static_cast<QMIPhysHarmonic*>(p.get());

	// FIXME, but how??
	QMParametersHarmonic FIXME_param;
	FIXME_param.dim=harmonic->dim; FIXME_param.hbar = harmonic->hbar; FIXME_param.coefficient = harmonic->coefficient;
	St1_QMStPotentialHarmonic FIXME_equation;

	//FIXME
	QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[I->id1]->state.get());
	NDimTable<Complexr>& val(qmigeom->potentialValues);
	val.resize(psi->tableValuesPosition,0);                   // FIXME (1↓) problem zaczyna się tutaj, ponieważ robiąc resize tak żeby pasowały do siebie, zakładam jednocześnie się się idealnie nakrywają.
	if(timeLimitH.messageAllowed(2) and qmigeom->relPos21!=Vector3r(0,0,0)) std::cerr << "Law2_QMIGeom_QMIPhysHarmonic::go  potencjał się nie nakrywa z funkcją falową!\n";
	if(psi->gridSize.size()==1) {
		size_t startI=psi->xToI(qmigeom->relPos21[0]-qmigeom->extents2[0],0);
		size_t endI  =psi->xToI(qmigeom->relPos21[0]+qmigeom->extents2[0],0);
		for(size_t i=startI ; i<=endI ; i++) {
			if(i>=0 and i<val.size0(0)) {              // ↓ FIXME? (2↑) teraz SchrodingerKosloffPropagator po prostu dodaje NDimTable, nie patrzy na ich względne położenia. Czyli nie mogę ich tak po prostu dodawać.
				Real x = psi->iToX(i,0) /* -qmigeom->relPos21[0] */;
				val.at(i)= FIXME_equation.getValPos(Vector3r(x,0,0),&FIXME_param,NULL);
				//              std::pow(psi->iToX(i,0) /* -qmigeom->relPos21[0] */,2) *harmonic->coefficient[0];
			}
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
			if(j>=0 and j<val.size0(1)) {
				Real x = psi->iToX(i,0) /* -qmigeom->relPos21[0] */;
				Real y = psi->iToX(j,1) /* -qmigeom->relPos21[1] */;
				val.at(i,j)= FIXME_equation.getValPos(Vector3r(x,y,0),&FIXME_param,NULL);
				//              std::pow(psi->iToX(i,0) /* -qmigeom->relPos21[0] */ ,2)*harmonic->coefficient[0]
				//             +std::pow(psi->iToX(j,1) /* -qmigeom->relPos21[1] */ ,2)*harmonic->coefficient[1];
			}
		}
	}
	if(psi->gridSize.size()==3) {
		size_t startI=psi->xToI(qmigeom->relPos21[0]-qmigeom->extents2[0],0);
		size_t endI  =psi->xToI(qmigeom->relPos21[0]+qmigeom->extents2[0],0);
		size_t startJ=psi->xToI(qmigeom->relPos21[1]-qmigeom->extents2[1],1);
		size_t endJ  =psi->xToI(qmigeom->relPos21[1]+qmigeom->extents2[1],1);
		size_t startK=psi->xToI(qmigeom->relPos21[2]-qmigeom->extents2[2],2);
		size_t endK  =psi->xToI(qmigeom->relPos21[2]+qmigeom->extents2[2],2);

		for(size_t i=startI ; i<=endI ; i++)
		for(size_t j=startJ ; j<=endJ ; j++)
		for(size_t k=startK ; k<=endK ; k++)
		{
			if(i>=0 and i<val.size0(0))
			if(j>=0 and j<val.size0(1))
			if(k>=0 and k<val.size0(2)) {
				Real x = psi->iToX(i,0) /* -qmigeom->relPos21[0] */;
				Real y = psi->iToX(j,1) /* -qmigeom->relPos21[1] */;
				Real z = psi->iToX(k,2) /* -qmigeom->relPos21[2] */;
				val.at(i,j,k)= FIXME_equation.getValPos(Vector3r(x,y,z),&FIXME_param,NULL);
				//              std::pow(psi->iToX(i,0) /* -qmigeom->relPos21[0] */ ,2)*harmonic->coefficient[0]
				//             +std::pow(psi->iToX(j,1) /* -qmigeom->relPos21[1] */ ,2)*harmonic->coefficient[1]
				//             +std::pow(psi->iToX(k,2) /* -qmigeom->relPos21[2] */ ,2)*harmonic->coefficient[2];
			}
		}
	}
	if(psi->gridSize.size() > 3) { std::cerr << "Law2_QMIGeom_QMIPhysHarmonic::go, dim>3"; exit(1); };
	return true;
};

/*********************************************************************************
*
* Law2   QMIGeom + QMIPhysHarmonicParticles  :  H A R M O N I C   I N T E R A C T I O N
*
*********************************************************************************/

CREATE_LOGGER(Law2_QMIGeom_QMIPhysHarmonicParticles);

bool Law2_QMIGeom_QMIPhysHarmonicParticles::go(shared_ptr<IGeom>& g, shared_ptr<IPhys>& p, Interaction* I)
{
	return false;


/*
	if(timeLimitH.messageAllowed(12)) std::cerr << "####### Law2_QMIGeom_QMIPhysHarmonicParticles::go  START!\n";

	QMIGeom*                  qmigeom  = static_cast<QMIGeom*                 >(g.get());
	QMIPhysHarmonicParticles* harmonic = static_cast<QMIPhysHarmonicParticles*>(p.get());

	//FIXME
	QMStateDiscrete* psi1=dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[I->id1]->state.get());
	QMStateDiscrete* psi2=dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[I->id2]->state.get());
	NDimTable<Complexr>& val(qmigeom->potentialValues);
	val.resize(psi->tableValuesPosition,0);                   // FIXME (1↓) problem zaczyna się tutaj, ponieważ robiąc resize tak żeby pasowały do siebie, zakładam jednocześnie się się idealnie nakrywają.
	if(timeLimitH.messageAllowed(2) and qmigeom->relPos21!=Vector3r(0,0,0)) std::cerr << "Law2_QMIGeom_QMIPhysHarmonicParticles::go  potencjał się nie nakrywa z funkcją falową!\n";
	if(psi->gridSize.size()==1) {
		size_t startI=psi->xToI(qmigeom->relPos21[0]-qmigeom->extents2[0],0);
		size_t endI  =psi->xToI(qmigeom->relPos21[0]+qmigeom->extents2[0],0);
		for(size_t i=startI ; i<=endI ; i++) {
			if(i>=0 and i<val.size0(0))               // ↓ FIXME? (2↑) teraz SchrodingerKosloffPropagator po prostu dodaje NDimTable, nie patrzy na ich względne położenia. Czyli nie mogę ich tak po prostu dodawać.
				val.at(i)=std::pow(psi->iToX(i,0) // -qmigeom->relPos21[0] 
				,2) *harmonic->coefficient[0];
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
				val.at(i,j)=  std::pow(psi->iToX(i,0) // -qmigeom->relPos21[0]
				 ,2)*harmonic->coefficient[0]
				             +std::pow(psi->iToX(j,1) // -qmigeom->relPos21[1]
				 ,2)*harmonic->coefficient[1];
		}
	}
	return true;
*/
};

