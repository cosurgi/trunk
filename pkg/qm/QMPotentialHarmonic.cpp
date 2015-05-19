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
	if(timeLimitH.messageAllowed(8)) std::cerr << "####### Law2_QMIGeom_QMIPhysHarmonic::go  START!  r̳e̳c̳a̳l̳c̳u̳l̳a̳t̳i̳n̳g̳ ̳w̳h̳o̳l̳e̳ ̳p̳o̳t̳e̳n̳t̳i̳a̳l̳!̳!̳!̳\n";

	QMIGeom*         qmigeom  = static_cast<QMIGeom*        >(g.get());
	QMIPhysHarmonic* harmonic = static_cast<QMIPhysHarmonic*>(p.get());

	// FIXME, but how?? I need this equation somehow.
	QMParametersHarmonic FIXME_param;
	FIXME_param.dim=harmonic->dim; FIXME_param.hbar = harmonic->hbar; FIXME_param.coefficient = harmonic->coefficient;
	St1_QMStPotentialHarmonic FIXME_equation;

	//FIXME - how to avoid getting Body from scene?
	QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[I->id1]->state.get());
	NDimTable<Complexr>& val(qmigeom->potentialValues);

	if(psi->gridSize.size() <= 3) {
// FIXME (1↓) problem zaczyna się tutaj, ponieważ robiąc resize tak żeby pasowały do siebie, zakładam jednocześnie że siatki się idealnie nakrywają.
//            hmm... ale nawet gdy mam iloczyn tensorowy to one muszą się idealnie nakrywać !
		val.resize(psi->tableValuesPosition);
		val.fill1WithFunction( psi->gridSize.size()
			, [&](Real i, int d)->Real    { return psi->iToX(i,d) - qmigeom->relPos21[d];}           // xyz position function
			, [&](Vector3r& xyz)->Complexr{ return FIXME_equation.getValPos(xyz,&FIXME_param,NULL);} // function value at xyz
			);
	} else { std::cerr << "\nLaw2_QMIGeom_QMIPhysHarmonic::go, dim>3\n"; exit(1); };
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


	if(timeLimitH.messageAllowed(12)) std::cerr << "####### Law2_QMIGeom_QMIPhysHarmonicParticles::go  r̳e̳c̳a̳l̳c̳u̳l̳a̳t̳i̳n̳g̳ ̳w̳h̳o̳l̳e̳ ̳̲P̲A̲R̲T̲I̲C̲L̲E̲ ̲I̲N̲T̲E̲R̲A̲C̲T̲I̲O̲N̲ ̲p̳o̳t̳e̳n̳t̳i̳a̳l̳\n";

	QMIGeom*                  qmigeom  = static_cast<QMIGeom*                 >(g.get());
	QMIPhysHarmonicParticles* harmonic = static_cast<QMIPhysHarmonicParticles*>(p.get());

	// FIXME, but how?? I need this equation somehow.
	QMParametersHarmonic FIXME_param;
	FIXME_param.dim=harmonic->dim; FIXME_param.hbar = harmonic->hbar; FIXME_param.coefficient = harmonic->coefficient1.cwiseProduct(harmonic->coefficient2);
	St1_QMStPotentialHarmonic FIXME_equation;

	//FIXME - how to avoid getting Body from scene?
	QMStateDiscrete* psi1=dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[I->id1]->state.get());
	QMStateDiscrete* psi2=dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[I->id2]->state.get());
	NDimTable<Complexr>& val(qmigeom->potentialValues);
/*
	if(psi1->gridSize.size() == psi2->gridSize.size() and psi1->gridSize.size()<= 3) {
// FIXME (1↓) problem zaczyna się tutaj, ponieważ robiąc resize tak żeby pasowały do siebie, zakładam jednocześnie że siatki się idealnie nakrywają.
//            hmm... ale nawet gdy mam iloczyn tensorowy to one muszą się idealnie nakrywać !
		val.resize(psi->tableValuesPosition); /// FIXME: product !!
		val.fillNWithFunction( psi->gridSize.size()              // ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ sprawdzić to jeszcze!!!!!
			, [&](Real i1, Real i2, int d)->Real    { return psi1->iToX(i,d) - psi2->iToX(i,d) - qmigeom->relPos21[d];} // xyz position function
			, [&](Vector3r& xyz)->Complexr{ return FIXME_equation.getValPos(xyz,&FIXME_param,NULL);} // function value at xyz
			);
	} else { std::cerr << "\nLaw2_QMIGeom_QMIPhysHarmonic::go, dim>3 or psi1->gridSize.size() != psi2->gridSize.size()\n"; exit(1); };
	return true;
*/
	return false;
};

