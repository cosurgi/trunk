// 2015 © Janek Kozicki <cosurgi@gmail.com>

#include "QMPotential.hpp"
#include "QMGeometry.hpp"

YADE_PLUGIN(
	(QMPotGeometry)
	(QMPotPhysics)
	(Ig2_Box_QMGeometry_QMPotGeometry)
	(Ig2_QMGeometry_QMGeometry_QMPotGeometry)
	);

/*********************************************************************************
*
* Q M   interaction   P O T E N T I A L   G E O M E T R Y            QMPotGeometry
*
*********************************************************************************/
CREATE_LOGGER(QMPotGeometry);
// !! at least one virtual function in the .cpp file
QMPotGeometry::~QMPotGeometry(){};

/*********************************************************************************
*
* Q M   interaction   P O T E N T I A L   P H Y S I C S               QMPotPhysics
*
*********************************************************************************/
CREATE_LOGGER(QMPotPhysics);
// !! at least one virtual function in the .cpp file
QMPotPhysics::~QMPotPhysics(){};

/*********************************************************************************
*
* Ig2   Box   QMGeometry  →  QMPotGeometry
*
*********************************************************************************/
CREATE_LOGGER(Ig2_Box_QMGeometry_QMPotGeometry);

// FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, wywalić to:
#include "QMStateDiscrete.hpp"
#include "QMPotentialBarrier.hpp"

bool Ig2_Box_QMGeometry_QMPotGeometry::go(
	  const shared_ptr<Shape>&    qm1, const shared_ptr<Shape>&    qm2
	, const State& state1            , const State& state2
	, const Vector3r& shift2         // dem legacy       ← that's for periodic boundaries
	, const bool& forceRecalculation // dem legacy FIXME ← I guess it's an override to force interaction even when it should not exist
	, const shared_ptr<Interaction>& c)
{
	if(c->geom) return true; // FIXME - this skips updating if geometry changes !!

	const Se3r& se31=state1.se3; const Se3r& se32=state2.se3;

////////////////////////////std::cerr << "Ig2_Box_QMGeometry_QMPotGeometry : " << state1.getClassName() << " "  << state2.getClassName() << "\n";
// FIXME,FIXME - weird method of getting state1 ← needed for getting the FFT grid size
// FIXME,FIXME - and stupid, I have state1 and state2 !!! why didn't I use it???
	QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[c->id2]->state.get());
	QMStateBarrier * pot=dynamic_cast<QMStateBarrier *>((*(scene->bodies))[c->id1]->state.get());

	if(!psi or !pot) return false;

	Vector3r pt1,pt2,normal;
	// Real depth;

	Box* obb             = static_cast<Box*>(qm1.get());
//	QMGeometry* s = static_cast<QMGeometry*>(qm2.get());

	Vector3r extents1 = obb->extents;
//	Vector3r extents2 = s  ->halfSize;

	//
	// FIXME: do we need rotation matrix? Can't quaternion do just fine?
	// FIXME - ignore rotation for now, there was a Box ←→ Box interaction in older yade version. I could use it here
	//
	//         Matrix3r boxAxisT=se31.orientation.toRotationMatrix();
	//         Matrix3r boxAxis = boxAxisT.transpose();

	Vector3r relPos12 = -1.0*(se32.position+shift2-se31.position); // relative position of centroids
	shared_ptr<QMPotGeometry> igeom;

	//
	// FIXME: assume that interaction always exists. WRONG! FIXME, FIXME.
	// I could use depth calculation from Box ←→ Box interaction
	//
	// if (depth<0 && !c->isReal() && !force) return false;
	//

	bool isNew=!c->geom;
	if (isNew) igeom = shared_ptr<QMPotGeometry>(new QMPotGeometry());
	else igeom = YADE_PTR_CAST<QMPotGeometry>(c->geom);

	//igeom->relativePosition21 = relPos12;
	//// igeom->relativeOrientation21 = ; FIXME
	//igeom->halfSize1 = extents1;
	//igeom->halfSize2 = extents2;

	NDimTable<Complexr>& val(igeom->potentialValues);
	val.resize(psi->tableValuesPosition); // use size of grid for ψ₀ to write potential data into igeom→potentialValues


	if(psi->gridSize.size()==1) {
		// FIXME - 1D only
		size_t startI=psi->xToI(relPos12[0]-extents1[0],0);
		size_t endI  =psi->xToI(relPos12[0]+extents1[0],0);

	// FIXME - remove cerr
//		std::cerr << startI << " " << endI << " " << relPos12[0] << " "
//			  << extents1[0] << " " << psi->tableValuesPosition.size0(0) << " " << val.size0(0) << " " << pot->potentialValue << "\n";

		for(size_t i=startI ; i<=endI ; i++) {
			if(i>=0 and i<val.size0(0))

	// FIXME - 1D only
			{
				switch(pot->potentialType) {
					case 0 : val.at(i)=pot->potentialValue; break;
					case 1 : val.at(i)=std::pow(psi->iToX(i,0),2)*0.5; break;
					default : std::cerr << "Unknown potential type\n";break;
				}
			}
		}
	}
	if(psi->gridSize.size()==2) {
		// FIXME - 2D only, NOT general :(((
		size_t startI=psi->xToI(relPos12[0]-extents1[0],0);
		size_t endI  =psi->xToI(relPos12[0]+extents1[0],0);
		size_t startJ=psi->xToI(relPos12[1]-extents1[1],1);
		size_t endJ  =psi->xToI(relPos12[1]+extents1[1],1);

		for(size_t i=startI ; i<=endI ; i++)
		for(size_t j=startJ ; j<=endJ ; j++)
		{
			if(i>=0 and i<val.size0(0))
			if(j>=0 and j<val.size0(1))

	// FIXME - 1D only
			{
				switch(pot->potentialType) { ////////// !!!!!!!!!!!! FIXME!!!!!!!!!!!!!
					// niech woła właściwą funkcję z odpowiedniego potencjału, zamiast tuaj to liczyć.
					case 0 : val.at(i,j)=pot->potentialValue; break;
					case 1 : val.at(i,j)=  std::pow(psi->iToX(i,0),2)*0.5
					                      +std::pow(psi->iToX(j,1),2)*0.5; break;
					default : std::cerr << "Unknown potential type\n";break;
				}
			}
		}
	}


	c->geom = igeom;
	std::cerr <<"####### igeom created in QMInteraction\n";
	return true;
}

bool Ig2_Box_QMGeometry_QMPotGeometry::goReverse(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2,
  const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c)
{
	c->swapOrder();
	return go(cm2,cm1,state2,state1,-shift2,force,c);
}

/*********************************************************************************
*
* Ig2   QMGeometry   QMGeometry  →  QMPotGeometry
*
*********************************************************************************/

CREATE_LOGGER(Ig2_QMGeometry_QMGeometry_QMPotGeometry);

bool Ig2_QMGeometry_QMGeometry_QMPotGeometry::go(
	  const shared_ptr<Shape>&    cm1, const shared_ptr<Shape>&    cm2
	, const State&             state1, const State&             state2
	, const Vector3r& /*dem legacy*/ , const bool& forceRecalculation
	, const shared_ptr<Interaction>& c)
{
	std::cerr << "Quantum entanglement not implemented yet.\n";
// będę tutaj przygotowywał dwie siatki, możliwe, że o różnych gridSize. Dobrze by było przelecieć wszystkie węzły siatki i dać ostrzeżenie
// jeżli któreś wezły są od siebie w odległości mniejszej niż 1/10 mniejszego step()
// to po to aby uniknąć dzielenia przez zero
	return false;
}

/*********************************************************************************
*
* Ip2   QMParameters   QMParameters  →  QMPotPhysics
*
*********************************************************************************/

CREATE_LOGGER(Ip2_2xQMParameters_QMPotPhysics);

void Ip2_2xQMParameters_QMPotPhysics::go(
	  const shared_ptr<Material>& m1, const shared_ptr<Material>& m2
	, const shared_ptr<Interaction>& I)
{
	const QMParameters* qm1 = static_cast<QMParameters*>(m1.get());
	const QMParameters* qm2 = static_cast<QMParameters*>(m2.get());

	shared_ptr<QMPotPhysics> pot;
	bool isNew = !I->phys;
	if(!isNew) pot=YADE_PTR_CAST<QMPotPhysics>(I->phys);
	else { pot=shared_ptr<QMPotPhysics>(new QMPotPhysics()); I->phys=pot; }

	if( (qm1->hbar != qm2->hbar) or (qm1->dim != qm2->dim) )
		throw std::logic_error("ERROR: P̲l̲a̲n̲c̲k̲'̲s̲ constant or number of d̲i̲m̲e̲n̲s̲i̲o̲n̲s̲ for body "+boost::lexical_cast<std::string>(I->getId1())+" and "+boost::lexical_cast<std::string>(I->getId2())+" are different\n");
	else {
		pot->hbar = qm1->hbar;
		pot->dim  = qm1->dim;
	}
}

