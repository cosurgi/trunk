// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "WaveFunctionState.hpp"
#include <yade/core/Scene.hpp>

YADE_PLUGIN(
	(QuantumMechanicalBody)
	(QuantumMechanicalState)
	(WaveFunctionState)
	(GaussianWavePacket)
	(GaussianAnalyticalPropagatingWavePacket_1D)
	(WaveFunctionGeometry)
	(WavePacketParameters)
	(WaveFunctionInteractionPhysics)
	(WaveFunctionInteractionGeometry)
	(Ip2_WavePacketParameters_WavePacketParameters_WaveFunctionInteractionPhysics)
	(Ip2_Material_WavePacketParameters_WaveFunctionInteractionPhysics)
	(Law2_WaveFunctionInteractionGeometry_WaveFunctionInteractionPhysics_WaveFunctionInteractionPhysics)
	(SchrodingerKosloffPropagator)
	#ifdef YADE_OPENGL
//	(Gl1_WaveFunctionGeometry) // moved to another file
//	(Gl1_WaveFunctionInteractionPhysics) // This is for later
	#endif	
	);

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   B O D Y
*
*********************************************************************************/
CREATE_LOGGER(QuantumMechanicalBody);
// !! at least one virtual function in the .cpp file
QuantumMechanicalBody::~QuantumMechanicalBody(){};

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   S T A T E
*
*********************************************************************************/
CREATE_LOGGER(QuantumMechanicalState);
// !! at least one virtual function in the .cpp file
QuantumMechanicalState::~QuantumMechanicalState(){};

/*********************************************************************************
*
* W A V E   F U N C T I O N   S T A T E
*
*********************************************************************************/
CREATE_LOGGER(WaveFunctionState);
// !! at least one virtual function in the .cpp file
WaveFunctionState::~WaveFunctionState(){};

CREATE_LOGGER(GaussianWavePacket);
// !! at least one virtual function in the .cpp file
GaussianWavePacket::~GaussianWavePacket(){};

CREATE_LOGGER(GaussianAnalyticalPropagatingWavePacket_1D);
// !! at least one virtual function in the .cpp file
GaussianAnalyticalPropagatingWavePacket_1D::~GaussianAnalyticalPropagatingWavePacket_1D(){};

std::complex<Real> GaussianAnalyticalPropagatingWavePacket_1D::waveFunctionValue_1D_positionRepresentation(
	Real x,    // position where wavepacket is calculated
	Real x0,   // initial position of wavepacket centar at time t=0
	Real t,    // time when wavepacket is evaluated
	Real k0,   // initial wavenumber of wavepacket
	Real m,    // particle mass
	Real a,    // wavepacket width, sometimes called sigma, of the Gaussian distribution
	Real hbar  // Planck's constant divided by 2pi
)
{
	x -= x0;
	return exp(
		-(
			(m*x*x+Mathr::I*a*a*k0*(k0*hbar*t-2.0*m*x))
			/
			(2.0*a*a*m+2.0*Mathr::I*hbar*t)
		)
	)
	/
	(
		pow(Mathr::PI,0.25)*(pow(a+Mathr::I*hbar*t/(a*m),0.5))
	);
};

/*********************************************************************************
*
* W A V E   F U N C T I O N   G E O M E T R Y
*
*********************************************************************************/
CREATE_LOGGER(WaveFunctionGeometry);
// !! at least one virtual function in the .cpp file
WaveFunctionGeometry::~WaveFunctionGeometry(){};

/*********************************************************************************
*
* W A V E   F U N C T I O N   M A T E R I A L
*
*********************************************************************************/
CREATE_LOGGER(WavePacketParameters);
// !! at least one virtual function in the .cpp file
WavePacketParameters::~WavePacketParameters(){};

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
* K O S L O F F   P R O P A G A T I O N   o f   S H R O D I N G E R   E Q.
*
*********************************************************************************/
CREATE_LOGGER(SchrodingerKosloffPropagator);
// !! at least one virtual function in the .cpp file
SchrodingerKosloffPropagator::~SchrodingerKosloffPropagator(){};

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


