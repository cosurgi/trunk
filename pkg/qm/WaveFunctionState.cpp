// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include"WaveFunctionState.hpp"
#include<yade/core/Scene.hpp>

YADE_PLUGIN(
	(WaveFunctionState)
	(GaussianWavePacket)
	(WaveFunctionGeometry)
	(WavePacketParameters)
	(WaveFunctionInteractionPhysics)
	(WaveFunctionInteractionGeometry)
	(Ip2_WavePacketParameters_WavePacketParameters_WaveFunctionInteractionPhysics)
	(Ip2_Material_WavePacketParameters_WaveFunctionInteractionPhysics)
	(Law2_ScGeom_WaveFunctionInteractionPhysics_WaveFunctionInteractionPhysics)
	(SchrodingerKosloffPropagator)
	#ifdef YADE_OPENGL
	(Gl1_WaveFunctionGeometry)
	(Gl1_WaveFunctionInteractionPhysics)
	#endif	
	);

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
* W A V E   F U N C T I O N   I N T E R A C T I O N   P H Y S I C S   (constitutive parameters of the contact)
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
CREATE_LOGGER(Law2_ScGeom_WaveFunctionInteractionPhysics_WaveFunctionInteractionPhysics);

bool Law2_ScGeom_WaveFunctionInteractionPhysics_WaveFunctionInteractionPhysics::go(shared_ptr<IGeom>& _geom, shared_ptr<IPhys>& _phys, Interaction* I){
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
	#include<yade/lib/opengl/OpenGLWrapper.hpp>
	#include<yade/lib/opengl/GLUtils.hpp>

	CREATE_LOGGER(Gl1_WaveFunctionGeometry);
	bool Gl1_WaveFunctionGeometry::abs=true;
	bool Gl1_WaveFunctionGeometry::real=false;
	bool Gl1_WaveFunctionGeometry::imag=false;
	Gl1_WaveFunctionGeometry::~Gl1_WaveFunctionGeometry(){};
	void Gl1_WaveFunctionGeometry::go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&)
	{
	}

	CREATE_LOGGER(Gl1_WaveFunctionInteractionPhysics);
	bool Gl1_WaveFunctionInteractionPhysics::abs=true;
	bool Gl1_WaveFunctionInteractionPhysics::real=false;
	bool Gl1_WaveFunctionInteractionPhysics::imag=false;
	Gl1_WaveFunctionInteractionPhysics::~Gl1_WaveFunctionInteractionPhysics(){};
	void Gl1_WaveFunctionInteractionPhysics::go(const shared_ptr<IPhys>& ip, const shared_ptr<Interaction>& i, const shared_ptr<Body>& b1, const shared_ptr<Body>& b2, bool wireFrame)
	{
	}

#endif

