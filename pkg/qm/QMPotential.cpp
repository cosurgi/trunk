// 2015 © Janek Kozicki <cosurgi@gmail.com>

#include "QMPotential.hpp"
#include "QMGeometry.hpp"

#include <lib/time/TimeLimit.hpp>
TimeLimit timeLimitP; // FIXME - remove when finshed fixing

YADE_PLUGIN(
	(QMStatePotential)
	(QMIGeom)
	(QMIPhys)
	(Ig2_2xQMGeometry_QMIGeom)
	(Ip2_2xQMParameters_QMIPhys)
	(Law2_QMIGeom_QMIPhys_GlobalWavefunction)
	);

/*********************************************************************************
*
* Q M   P O T E N T I A L   B A R R I E R   state                 QMStatePotential
*
*********************************************************************************/
CREATE_LOGGER(QMStatePotential);
// !! at least one virtual function in the .cpp file
QMStatePotential::~QMStatePotential(){};

/*********************************************************************************
*
* Q M   interaction   P O T E N T I A L   G E O M E T R Y            QMIGeom
*
*********************************************************************************/
CREATE_LOGGER(QMIGeom);
// !! at least one virtual function in the .cpp file
QMIGeom::~QMIGeom(){};

/*********************************************************************************
*
* Q M   interaction   P O T E N T I A L   P H Y S I C S               QMIPhys
*
*********************************************************************************/
CREATE_LOGGER(QMIPhys);
// !! at least one virtual function in the .cpp file
QMIPhys::~QMIPhys(){};

/*********************************************************************************
*
* Ig2   QMGeometry   QMGeometry  →  QMIGeom
*
*********************************************************************************/

CREATE_LOGGER(Ig2_2xQMGeometry_QMIGeom);

bool Ig2_2xQMGeometry_QMIGeom::go(
	  const shared_ptr<Shape>&    cm1, const shared_ptr<Shape>&    cm2
	, const State&                st1, const State&                st2
	, const Vector3r& /*dem legacy*/ , const bool& forceRecalculation
	, const shared_ptr<Interaction>& I)
{
	if(timeLimitP.messageAllowed(12)) std::cerr << "####### Ig2_2xQMGeometry_QMIGeom::go  START!\n";
// będę tutaj przygotowywał dwie siatki, możliwe, że o różnych gridSize. Dobrze by było przelecieć wszystkie węzły siatki i dać ostrzeżenie
// jeżli któreś wezły są od siebie w odległości mniejszej niż 1/10 mniejszego step()
// to po to aby uniknąć dzielenia przez zero

	shared_ptr<QMIGeom> qmigeom;
	bool isNew = !I->geom;
	if(!isNew) qmigeom=YADE_PTR_CAST<QMIGeom>(I->geom);
	else { qmigeom=shared_ptr<QMIGeom>(new QMIGeom()); I->geom=qmigeom; }

	const QMGeometry*      qg1 = static_cast <const QMGeometry*     >(cm1.get());
	const QMGeometry*      qg2 = static_cast <const QMGeometry*     >(cm2.get());
	const QMStateDiscrete* qs1 = dynamic_cast<const QMStateDiscrete*>(&st1);
	const QMStateDiscrete* qs2 = dynamic_cast<const QMStateDiscrete*>(&st2);

	if(!qs1 or !qs2) { throw std::runtime_error("\n\nERROR: Ig2_2xQMGeometry_QMIGeom::go has no QMStateDiscrete.\n\n");};

	qmigeom->extents1  = qg1->extents;        // type: Vector3r
	qmigeom->extents2  = qg2->extents;        // type: Vector3r
	qmigeom->relPos21  = qs2->pos - qs1->pos; // type: Vector3r
	qmigeom->size1     = qs1->size;           // type: vector<Real>
	qmigeom->size2     = qs2->size;           // type: vector<Real>
	qmigeom->gridSize1 = qs1->gridSize;       // type: vector<size_t>
	qmigeom->gridSize1 = qs2->gridSize;       // type: vector<size_t>

	return true; // interaction always happens - FIXME - I could later check for spatial overlap... or sth. like that
}

/*********************************************************************************
*
* Ip2   QMParameters   QMParameters  →  QMIPhys
*
*********************************************************************************/

CREATE_LOGGER(Ip2_2xQMParameters_QMIPhys);

void Ip2_2xQMParameters_QMIPhys::go(
	  const shared_ptr<Material>& m1, const shared_ptr<Material>& m2
	, const shared_ptr<Interaction>& I)
{
	if(timeLimitP.messageAllowed(12)) std::cerr << "####### Ip2_2xQMParameters_QMIPhys::go  START!\n";

	shared_ptr<QMIPhys> pot;
	bool isNew = !I->phys;
	if(!isNew) pot=YADE_PTR_CAST<QMIPhys>(I->phys);
	else { pot=shared_ptr<QMIPhys>(new QMIPhys()); I->phys=pot; }

	const QMParameters* qm1 = static_cast<QMParameters*>(m1.get());
	const QMParameters* qm2 = static_cast<QMParameters*>(m2.get());

	if( (qm1->hbar != qm2->hbar) or (qm1->dim != qm2->dim) )
		throw std::logic_error("\n\nERROR: Ip2_2xQMParameters_QMIPhys::go  P̲l̲a̲n̲c̲k̲'̲s̲ constant or number of d̲i̲m̲e̲n̲s̲i̲o̲n̲s̲ for body "
		+boost::lexical_cast<std::string>(I->getId1())+" and "+boost::lexical_cast<std::string>(I->getId2())+" are different.\n\n");
	else {
		pot->hbar = qm1->hbar;
		pot->dim  = qm1->dim;
	}
}

/*********************************************************************************
*
* Law2   QMIGeom + QMIPhys  :  G L O B A L   W A V E F U N C T I O N
*
*********************************************************************************/

CREATE_LOGGER(Law2_QMIGeom_QMIPhys_GlobalWavefunction);

bool Law2_QMIGeom_QMIPhys_GlobalWavefunction::go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*)
{
	// FIXME - create global (fermion wavefunction) particle   wavefunction here ?! 
	// FIXME - create global (bozon wavefunction)   interaction (potential) here ?!
	return false;
};

