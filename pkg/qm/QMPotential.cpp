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

//FIXME: rm   /*********************************************************************************
//FIXME: rm   *
//FIXME: rm   * Ig2   Box   QMGeometry  →  QMIGeom
//FIXME: rm   *
//FIXME: rm   *********************************************************************************/
//FIXME: rm   CREATE_LOGGER(Ig2_Box_QMGeometry_QMIGeom);
//FIXME: rm   
//FIXME: rm   // FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, wywalić to:
//FIXME: rm   #include "QMStateDiscrete.hpp"
//FIXME: rm   #include "QMPotentialBarrier.hpp"
//FIXME: rm   
//FIXME: rm   bool Ig2_Box_QMGeometry_QMIGeom::go(
//FIXME: rm   	  const shared_ptr<Shape>&    qm1, const shared_ptr<Shape>&    qm2
//FIXME: rm   	, const State& state1            , const State& state2
//FIXME: rm   	, const Vector3r& shift2         // dem legacy       ← that's for periodic boundaries
//FIXME: rm   	, const bool& forceRecalculation // dem legacy FIXME ← I guess it's an override to force interaction even when it should not exist
//FIXME: rm   	, const shared_ptr<Interaction>& c)
//FIXME: rm   {
//FIXME: rm   /// 	if(c->geom) return true; // FIXME - this skips updating if geometry changes !!
//FIXME: rm   /// 
//FIXME: rm   /// 	const Se3r& se31=state1.se3; const Se3r& se32=state2.se3;
//FIXME: rm   /// 
//FIXME: rm   /// ////////////////////////////std::cerr << "Ig2_Box_QMGeometry_QMIGeom : " << state1.getClassName() << " "  << state2.getClassName() << "\n";
//FIXME: rm   /// // FIXME,FIXME - weird method of getting state1 ← needed for getting the FFT grid size
//FIXME: rm   /// // FIXME,FIXME - and stupid, I have state1 and state2 !!! why didn't I use it???
//FIXME: rm   /// 	QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[c->id2]->state.get());
//FIXME: rm   /// 	QMStateBarrier * pot=dynamic_cast<QMStateBarrier *>((*(scene->bodies))[c->id1]->state.get());
//FIXME: rm   /// 
//FIXME: rm   /// 	if(!psi or !pot) return false;
//FIXME: rm   /// 
//FIXME: rm   /// 	Vector3r pt1,pt2,normal;
//FIXME: rm   /// 	// Real depth;
//FIXME: rm   /// 
//FIXME: rm   /// 	Box* obb             = static_cast<Box*>(qm1.get());
//FIXME: rm   /// //	QMGeometry* s = static_cast<QMGeometry*>(qm2.get());
//FIXME: rm   /// 
//FIXME: rm   /// 	Vector3r extents1 = obb->extents;
//FIXME: rm   /// //	Vector3r extents2 = s  ->halfSize;
//FIXME: rm   /// 
//FIXME: rm   /// 	//
//FIXME: rm   /// 	// FIXME: do we need rotation matrix? Can't quaternion do just fine?
//FIXME: rm   /// 	// FIXME - ignore rotation for now, there was a Box ←→ Box interaction in older yade version. I could use it here
//FIXME: rm   /// 	//
//FIXME: rm   /// 	//         Matrix3r boxAxisT=se31.orientation.toRotationMatrix();
//FIXME: rm   /// 	//         Matrix3r boxAxis = boxAxisT.transpose();
//FIXME: rm   /// 
//FIXME: rm   /// 	Vector3r relPos12 = -1.0*(se32.position+shift2-se31.position); // relative position of centroids
//FIXME: rm   /// 	shared_ptr<QMIGeom> igeom;
//FIXME: rm   /// 
//FIXME: rm   /// 	//
//FIXME: rm   /// 	// FIXME: assume that interaction always exists. WRONG! FIXME, FIXME.
//FIXME: rm   /// 	// I could use depth calculation from Box ←→ Box interaction
//FIXME: rm   /// 	//
//FIXME: rm   /// 	// if (depth<0 && !c->isReal() && !force) return false;
//FIXME: rm   /// 	//
//FIXME: rm   /// 
//FIXME: rm   /// 	bool isNew=!c->geom;
//FIXME: rm   /// 	if (isNew) igeom = shared_ptr<QMIGeom>(new QMIGeom());
//FIXME: rm   /// 	else igeom = YADE_PTR_CAST<QMIGeom>(c->geom);
//FIXME: rm   /// 
//FIXME: rm   /// 	//igeom->relativePosition21 = relPos12;
//FIXME: rm   /// 	//// igeom->relativeOrientation21 = ; FIXME
//FIXME: rm   /// 	//igeom->halfSize1 = extents1;
//FIXME: rm   /// 	//igeom->halfSize2 = extents2;
//FIXME: rm   /// 
//FIXME: rm   /// 	NDimTable<Complexr>& val(igeom->potentialValues);
//FIXME: rm   /// 	val.resize(psi->tableValuesPosition); // use size of grid for ψ₀ to write potential data into igeom→potentialValues
//FIXME: rm   /// 
//FIXME: rm   /// 
//FIXME: rm   /// 	if(psi->gridSize.size()==1) {
//FIXME: rm   /// 		// FIXME - 1D only
//FIXME: rm   /// 		size_t startI=psi->xToI(relPos12[0]-extents1[0],0);
//FIXME: rm   /// 		size_t endI  =psi->xToI(relPos12[0]+extents1[0],0);
//FIXME: rm   /// 
//FIXME: rm   /// 	// FIXME - remove cerr
//FIXME: rm   /// //		std::cerr << startI << " " << endI << " " << relPos12[0] << " "
//FIXME: rm   /// //			  << extents1[0] << " " << psi->tableValuesPosition.size0(0) << " " << val.size0(0) << " " << pot->potentialValue << "\n";
//FIXME: rm   /// 
//FIXME: rm   /// 		for(size_t i=startI ; i<=endI ; i++) {
//FIXME: rm   /// 			if(i>=0 and i<val.size0(0))
//FIXME: rm   /// 
//FIXME: rm   /// 	// FIXME - 1D only
//FIXME: rm   /// 			{
//FIXME: rm   /// 				switch(pot->potentialType) {
//FIXME: rm   /// 					case 0 : val.at(i)=pot->potentialValue; break;
//FIXME: rm   /// 					case 1 : val.at(i)=std::pow(psi->iToX(i,0),2)*0.5; break;
//FIXME: rm   /// 					default : std::cerr << "Unknown potential type\n";break;
//FIXME: rm   /// 				}
//FIXME: rm   /// 			}
//FIXME: rm   /// 		}
//FIXME: rm   /// 	}
//FIXME: rm   /// 	if(psi->gridSize.size()==2) {
//FIXME: rm   /// 		// FIXME - 2D only, NOT general :(((
//FIXME: rm   /// 		size_t startI=psi->xToI(relPos12[0]-extents1[0],0);
//FIXME: rm   /// 		size_t endI  =psi->xToI(relPos12[0]+extents1[0],0);
//FIXME: rm   /// 		size_t startJ=psi->xToI(relPos12[1]-extents1[1],1);
//FIXME: rm   /// 		size_t endJ  =psi->xToI(relPos12[1]+extents1[1],1);
//FIXME: rm   /// 
//FIXME: rm   /// 		for(size_t i=startI ; i<=endI ; i++)
//FIXME: rm   /// 		for(size_t j=startJ ; j<=endJ ; j++)
//FIXME: rm   /// 		{
//FIXME: rm   /// 			if(i>=0 and i<val.size0(0))
//FIXME: rm   /// 			if(j>=0 and j<val.size0(1))
//FIXME: rm   /// 
//FIXME: rm   /// 	// FIXME - 1D only
//FIXME: rm   /// 			{
//FIXME: rm   /// 				switch(pot->potentialType) { ////////// !!!!!!!!!!!! FIXME!!!!!!!!!!!!!
//FIXME: rm   /// 					// niech woła właściwą funkcję z odpowiedniego potencjału, zamiast tuaj to liczyć.
//FIXME: rm   /// 					case 0 : val.at(i,j)=pot->potentialValue; break;
//FIXME: rm   /// 					case 1 : val.at(i,j)=  std::pow(psi->iToX(i,0),2)*0.5
//FIXME: rm   /// 					                      +std::pow(psi->iToX(j,1),2)*0.5; break;
//FIXME: rm   /// 					default : std::cerr << "Unknown potential type\n";break;
//FIXME: rm   /// 				}
//FIXME: rm   /// 			}
//FIXME: rm   /// 		}
//FIXME: rm   /// 	}
//FIXME: rm   /// 
//FIXME: rm   /// 
//FIXME: rm   /// 	c->geom = igeom;
//FIXME: rm   /// 	std::cerr <<"####### igeom created in QMInteraction\n";
//FIXME: rm   	return true;
//FIXME: rm   }
//FIXME: rm   
//FIXME: rm   bool Ig2_Box_QMGeometry_QMIGeom::goReverse(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2,
//FIXME: rm     const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c)
//FIXME: rm   {
//FIXME: rm   	c->swapOrder();
//FIXME: rm   	return go(cm2,cm1,state2,state1,-shift2,force,c);
//FIXME: rm   }

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
	return false;
};

