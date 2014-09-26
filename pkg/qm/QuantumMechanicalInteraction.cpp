// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QuantumMechanicalInteraction.hpp"
#include <yade/core/Scene.hpp>

YADE_PLUGIN(
//	(QuantumMechanicalBody)	                       // moved to another file
//	(QuantumMechanicalState)                       // moved to another file
//	(WaveFunctionState)                            // moved to another file
//	(GaussianWavePacket)                           // moved to another file
//	(GaussianAnalyticalPropagatingWavePacket_1D)   // moved to another file
//	(QuantumMechanicalGeometryDisplay)                    // moved to another file
//	(QuantumMechanicalParameters)                  // moved to another file
	(QuantumMechanicalInteractionPhysics)
	(QuantumMechanicalInteractionGeometry)
	(Ip2_QuantumMechanicalParameters_QuantumMechanicalParameters_QuantumMechanicalInteractionPhysics)
	(Ip2_Material_QuantumMechanicalParameters_QuantumMechanicalInteractionPhysics)
	(Law2_QuantumMechanicalInteractionGeometry_QuantumMechanicalInteractionPhysics_QuantumMechanicalInteractionPhysics)
//	(SchrodingerKosloffPropagator)                 // moved to another file
	#ifdef YADE_OPENGL
//	(Gl1_QuantumMechanicalGeometryDisplay) // moved to another file
//	(Gl1_QuantumMechanicalInteractionPhysics) // This is for later
	#endif
	);

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   I N T E R A C T I O N   P H Y S I C S     (constitutive parameters of the contact)
*
*********************************************************************************/
CREATE_LOGGER(QuantumMechanicalInteractionPhysics);
// !! at least one virtual function in the .cpp file
QuantumMechanicalInteractionPhysics::~QuantumMechanicalInteractionPhysics(){};


/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   I N T E R A C T I O N   G E O M E T R Y   (geometrical/spatial parameters of the contact)
*
*********************************************************************************/
CREATE_LOGGER(QuantumMechanicalInteractionGeometry);
// !! at least one virtual function in the .cpp file
QuantumMechanicalInteractionGeometry::~QuantumMechanicalInteractionGeometry(){};


/*********************************************************************************
*
* I N T E R A C T I O N   P H Y S I C S   constitutive parameters of the contact
*
*********************************************************************************/
CREATE_LOGGER(Ip2_QuantumMechanicalParameters_QuantumMechanicalParameters_QuantumMechanicalInteractionPhysics);
void Ip2_QuantumMechanicalParameters_QuantumMechanicalParameters_QuantumMechanicalInteractionPhysics::go(const shared_ptr<Material>& pp1, const shared_ptr<Material>& pp2, const shared_ptr<Interaction>& interaction){
}

CREATE_LOGGER(Ip2_Material_QuantumMechanicalParameters_QuantumMechanicalInteractionPhysics);
void Ip2_Material_QuantumMechanicalParameters_QuantumMechanicalInteractionPhysics::go(const shared_ptr<Material>& pp1, const shared_ptr<Material>& pp2, const shared_ptr<Interaction>& interaction){
}

/*********************************************************************************
*
* L A W 2   In DEM it was used to calculate Fn and Fs between two interacting bodies
*
*********************************************************************************/
CREATE_LOGGER(Law2_QuantumMechanicalInteractionGeometry_QuantumMechanicalInteractionPhysics_QuantumMechanicalInteractionPhysics);

bool Law2_QuantumMechanicalInteractionGeometry_QuantumMechanicalInteractionPhysics_QuantumMechanicalInteractionPhysics::go(shared_ptr<IGeom>& _geom, shared_ptr<IPhys>& _phys, Interaction* I){
}

