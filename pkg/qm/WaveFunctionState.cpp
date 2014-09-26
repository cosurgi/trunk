// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "WaveFunctionState.hpp"
#include <yade/core/Scene.hpp>

YADE_PLUGIN(
//	(QuantumMechanicalBody)	                       // moved to another file
//	(QuantumMechanicalState)                       // moved to another file
//	(WaveFunctionState)                            // moved to another file
//	(GaussianWavePacket)                           // moved to another file
//	(GaussianAnalyticalPropagatingWavePacket_1D)   // moved to another file
//	(QuantumMechanicalGeometry)                    // moved to another file
//	(WavePacketParameters)                         // moved to another file
	(WaveFunctionInteractionPhysics)
	(WaveFunctionInteractionGeometry)
	(Ip2_WavePacketParameters_WavePacketParameters_WaveFunctionInteractionPhysics)
	(Ip2_Material_WavePacketParameters_WaveFunctionInteractionPhysics)
	(Law2_WaveFunctionInteractionGeometry_WaveFunctionInteractionPhysics_WaveFunctionInteractionPhysics)
//	(SchrodingerKosloffPropagator)                 // moved to another file
	#ifdef YADE_OPENGL
//	(Gl1_QuantumMechanicalGeometry) // moved to another file
//	(Gl1_WaveFunctionInteractionPhysics) // This is for later
	#endif	
	);

/*********************************************************************************
*
* W A V E   F U N C T I O N   I N T E R A C T I O N   P H Y S I C S     (constitutive parameters of the contact)
*
*********************************************************************************/
CREATE_LOGGER(WaveFunctionInteractionPhysics);
// !! at least one virtual function in the .cpp file
WaveFunctionInteractionPhysics::~WaveFunctionInteractionPhysics(){};


/*********************************************************************************
*
* W A V E   F U N C T I O N   I N T E R A C T I O N   G E O M E T R Y   (geometrical/spatial parameters of the contact)
*
*********************************************************************************/
CREATE_LOGGER(WaveFunctionInteractionGeometry);
// !! at least one virtual function in the .cpp file
WaveFunctionInteractionGeometry::~WaveFunctionInteractionGeometry(){};


/*********************************************************************************
*
* I N T E R A C T I O N   P H Y S I C S   constitutive parameters of the contact
*
*********************************************************************************/
CREATE_LOGGER(Ip2_WavePacketParameters_WavePacketParameters_WaveFunctionInteractionPhysics);
void Ip2_WavePacketParameters_WavePacketParameters_WaveFunctionInteractionPhysics::go(const shared_ptr<Material>& pp1, const shared_ptr<Material>& pp2, const shared_ptr<Interaction>& interaction){
}

CREATE_LOGGER(Ip2_Material_WavePacketParameters_WaveFunctionInteractionPhysics);
void Ip2_Material_WavePacketParameters_WaveFunctionInteractionPhysics::go(const shared_ptr<Material>& pp1, const shared_ptr<Material>& pp2, const shared_ptr<Interaction>& interaction){
}

/*********************************************************************************
*
* L A W 2   In DEM it was used to calculate Fn and Fs between two interacting bodies
*
*********************************************************************************/
CREATE_LOGGER(Law2_WaveFunctionInteractionGeometry_WaveFunctionInteractionPhysics_WaveFunctionInteractionPhysics);

bool Law2_WaveFunctionInteractionGeometry_WaveFunctionInteractionPhysics_WaveFunctionInteractionPhysics::go(shared_ptr<IGeom>& _geom, shared_ptr<IPhys>& _phys, Interaction* I){
}

/*********************************************************************************
*
* W A V E   F U N C T I O N   O P E N   G L   D I S P L A Y
*
*********************************************************************************/

#ifdef YADE_OPENGL

// This will come later, when I will have some interactions going on....
//	CREATE_LOGGER(Gl1_WaveFunctionInteractionPhysics);
//	bool Gl1_WaveFunctionInteractionPhysics::abs=true;
//	bool Gl1_WaveFunctionInteractionPhysics::real=false;
//	bool Gl1_WaveFunctionInteractionPhysics::imag=false;
//	Gl1_WaveFunctionInteractionPhysics::~Gl1_WaveFunctionInteractionPhysics(){};
//	void Gl1_WaveFunctionInteractionPhysics::go(const shared_ptr<IPhys>& ip, const shared_ptr<Interaction>& i, const shared_ptr<Body>& b1, const shared_ptr<Body>& b2, bool wireFrame)
//	{
//	}

#endif


