// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMInteraction.hpp"
#include "QMStateDiscrete.hpp"
#include "QMPotentialBarrier.hpp"
#include <core/Scene.hpp>
#include <lib/base/Math.hpp>

YADE_PLUGIN(
	(Ip2_QMParameters_QMParameters_QMPotPhysics)
	(Law2_QMPotGeometry_QMPotPhysics_QMPotPhysics)
	);

/*********************************************************************************
*
* I N T E R A C T I O N   P H Y S I C S   constitutive parameters of the contact
*
*********************************************************************************/
CREATE_LOGGER(Ip2_QMParameters_QMParameters_QMPotPhysics);
void Ip2_QMParameters_QMParameters_QMPotPhysics::go(
	const shared_ptr<Material>& pp1,
	const shared_ptr<Material>& pp2,
	const shared_ptr<Interaction>& interaction)
{	
	if(interaction->phys) return;

	// FIXME - those two lines unnecessary right now
	// btw, will they be necessary
//	const shared_ptr<QMParameters>& q1 = YADE_PTR_CAST<QMParameters>(pp1);
//	const shared_ptr<QMParameters>& q2 = YADE_PTR_CAST<QMParameters>(pp2);
	
	interaction->phys = shared_ptr<QMPotPhysics>(new QMPotPhysics());
	std::cerr <<"####### iphys created in QMInteraction\n";
}

/*********************************************************************************
*
* L A W 2   In DEM it was used to calculate Fn and Fs between two interacting bodies
*           Here it will calculate potential acting between wavepackets or potential barriers
*
*********************************************************************************/
CREATE_LOGGER(Law2_QMPotGeometry_QMPotPhysics_QMPotPhysics);

bool Law2_QMPotGeometry_QMPotPhysics_QMPotPhysics::go(
	shared_ptr<IGeom>& igeom, 
	shared_ptr<IPhys>& iphys, 
	Interaction* interaction)
{
	return true; // always a success. I had a serious bug when this was missing - interactions were deleted.
}

