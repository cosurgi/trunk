// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMInteraction.hpp"
#include "QMStateDiscrete.hpp"
#include "QMStateBarrier.hpp"
#include <core/Scene.hpp>
#include <lib/base/Math.hpp>

YADE_PLUGIN(
	(QMInteractionPhysics)
	(QMInteractionGeometry)
	(Ig2_Box_QMGeometryDisplay_QMInteractionGeometry)
	(Ip2_QMParameters_QMParameters_QMInteractionPhysics)
	(Ip2_Material_QMParameters_QMInteractionPhysics)
	(Law2_QMInteractionGeometry_QMInteractionPhysics_QMInteractionPhysics)
	);

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   I N T E R A C T I O N   P H Y S I C S     (constitutive/physical parameters of the contact)
*
*********************************************************************************/
CREATE_LOGGER(QMInteractionPhysics);
// !! at least one virtual function in the .cpp file
QMInteractionPhysics::~QMInteractionPhysics(){};


/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   I N T E R A C T I O N   G E O M E T R Y   (geometrical/spatial parameters of the contact)
*
*********************************************************************************/
CREATE_LOGGER(QMInteractionGeometry);
// !! at least one virtual function in the .cpp file
QMInteractionGeometry::~QMInteractionGeometry(){};


/*********************************************************************************
*
* I G 2   B O X   W A V E F U N C T I O N   I N T E R A C T I O N                 (creates geometry of two overlapping entities)
*
*********************************************************************************/

bool Ig2_Box_QMGeometryDisplay_QMInteractionGeometry::go(
	const shared_ptr<Shape>& qm1, 
	const shared_ptr<Shape>& qm2, 
	const State& state1, 
	const State& state2, 
	const Vector3r& shift2 ,    // dem legacy       ← that's for periodic boundaries
	const bool& /* force */ ,   // dem legacy FIXME ← I guess it's an override to force interaction even when it should not exist
	const shared_ptr<Interaction>& c)
{
	if(c->geom) return true; // FIXME - this skips updating if geometry changes !!
	
	const Se3r& se31=state1.se3; const Se3r& se32=state2.se3;

	// FIXME,FIXME - weird method of getting state1 ← needed for getting the FFT grid size
	QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[c->id2]->state.get());
	QMStateBarrier * pot=dynamic_cast<QMStateBarrier *>((*(scene->bodies))[c->id1]->state.get());

	if(!psi or !pot) return false;

	Vector3r pt1,pt2,normal;
	// Real depth;

	Box* obb             = static_cast<Box*>(qm1.get());
//	QMGeometryDisplay* s = static_cast<QMGeometryDisplay*>(qm2.get());

	Vector3r extents1 = obb->extents;
//	Vector3r extents2 = s  ->halfSize;

	// 
	// FIXME: do we need rotation matrix? Can't quaternion do just fine?
	// FIXME - ignore rotation for now, there was a Box ←→ Box interaction in older yade version. I could use it here
	//
	//         Matrix3r boxAxisT=se31.orientation.toRotationMatrix();
	//         Matrix3r boxAxis = boxAxisT.transpose();

	Vector3r relPos12 = -1.0*(se32.position+shift2-se31.position); // relative position of centroids
	shared_ptr<QMInteractionGeometry> igeom;

	//
	// FIXME: assume that interaction always exists. WRONG! FIXME, FIXME.
	// I could use depth calculation from Box ←→ Box interaction
	//
	// if (depth<0 && !c->isReal() && !force) return false;
	//

	bool isNew=!c->geom;
	if (isNew) igeom = shared_ptr<QMInteractionGeometry>(new QMInteractionGeometry());
	else igeom = YADE_PTR_CAST<QMInteractionGeometry>(c->geom);
	
	//igeom->relativePosition21 = relPos12;
	//// igeom->relativeOrientation21 = ; FIXME
	//igeom->halfSize1 = extents1;
	//igeom->halfSize2 = extents2;

	NDimTable<Complexr>& val(igeom->potentialValues);
	val.resize(psi->tableValuesPosition); // use size of grid for ψ₀ to write potential data into igeom→potentialValues


	if(psi->dim==1) {
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
	if(psi->dim==2) {
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



bool Ig2_Box_QMGeometryDisplay_QMInteractionGeometry::goReverse(const shared_ptr<Shape>& cm1, const shared_ptr<Shape>& cm2,
  const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c)
{
	c->swapOrder();
	return go(cm2,cm1,state2,state1,-shift2,force,c);
}

/*********************************************************************************
*
* I N T E R A C T I O N   P H Y S I C S   constitutive parameters of the contact
*
*********************************************************************************/
CREATE_LOGGER(Ip2_QMParameters_QMParameters_QMInteractionPhysics);
void Ip2_QMParameters_QMParameters_QMInteractionPhysics::go(
	const shared_ptr<Material>& pp1,
	const shared_ptr<Material>& pp2,
	const shared_ptr<Interaction>& interaction)
{	
	if(interaction->phys) return;

	// FIXME - those two lines unnecessary right now
	// btw, will they be necessary
//	const shared_ptr<QMParameters>& q1 = YADE_PTR_CAST<QMParameters>(pp1);
//	const shared_ptr<QMParameters>& q2 = YADE_PTR_CAST<QMParameters>(pp2);
	
	interaction->phys = shared_ptr<QMInteractionPhysics>(new QMInteractionPhysics());
	std::cerr <<"####### iphys created in QMInteraction\n";
}

CREATE_LOGGER(Ip2_Material_QMParameters_QMInteractionPhysics);
void Ip2_Material_QMParameters_QMInteractionPhysics::go(const shared_ptr<Material>& pp1, const shared_ptr<Material>& pp2, const shared_ptr<Interaction>& interaction){
}

/*********************************************************************************
*
* L A W 2   In DEM it was used to calculate Fn and Fs between two interacting bodies
*           Here it will calculate potential acting between wavepackets or potential barriers
*
*********************************************************************************/
CREATE_LOGGER(Law2_QMInteractionGeometry_QMInteractionPhysics_QMInteractionPhysics);

bool Law2_QMInteractionGeometry_QMInteractionPhysics_QMInteractionPhysics::go(
	shared_ptr<IGeom>& igeom, 
	shared_ptr<IPhys>& iphys, 
	Interaction* interaction)
{
	return true; // always a success. I had a serious bug when this was missing - interactions were deleted.
}

