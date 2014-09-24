// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include"WaveFunctionState.hpp"
#include<yade/core/Scene.hpp>

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
	(Gl1_GaussianAnalyticalPropagatingWavePacket_1D)
//	(Gl1_WaveFunctionGeometry)
	(Gl1_WaveFunctionInteractionPhysics)
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

std::complex<Real> GaussianAnalyticalPropagatingWavePacket_1D::waveFunctionValue_1D_positionRepresentation
	(Real x,Real x0,Real t,Real k0,Real m, Real a, Real h)
{
	x -= x0;
	return exp(-(m*x*x+Mathr::I*a*a*k0*(k0*h*t-2.0*m*x)/(2.0*a*a*m+2.0*Mathr::I*h*t)))/(pow(Mathr::PI,0.25)*(pow(a+Mathr::I*h*t/(a*m),0.25)));
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
	#include<yade/pkg/common/GLDrawFunctors.hpp>
	#include<yade/lib/opengl/OpenGLWrapper.hpp>
	#include<yade/lib/opengl/GLUtils.hpp>

	// FIXME: probably should be merged with Gl1_WaveFunctionGeometry, but let's do something that works first.
	CREATE_LOGGER(Gl1_GaussianAnalyticalPropagatingWavePacket_1D);
	bool Gl1_GaussianAnalyticalPropagatingWavePacket_1D::abs=true;
//	bool Gl1_GaussianAnalyticalPropagatingWavePacket_1D::real=false;
//	bool Gl1_GaussianAnalyticalPropagatingWavePacket_1D::imag=false;
	Gl1_GaussianAnalyticalPropagatingWavePacket_1D::~Gl1_GaussianAnalyticalPropagatingWavePacket_1D(){};
//	void Gl1_GaussianAnalyticalPropagatingWavePacket_1D::go
//	(const shared_ptr<Shape>& shape, const shared_ptr<State>& state,bool wire,const GLViewInfo& glv)
//	{
//		std::cerr<<"Gl1_GaussianAnalyticalPropagatingWavePacket_1D\n";
//		glColor3v(shape->color);
////		GaussianAnalyticalPropagatingWavePacket_1D* packet=(dynamic_cast<GaussianAnalyticalPropagatingWavePacket_1D>)(state.get());
//		glutSolidCube(0.2);
//	};

void Gl1_GaussianAnalyticalPropagatingWavePacket_1D::go(const shared_ptr<Shape>& shape, const shared_ptr<State>& state,bool wire,const GLViewInfo&)
{
	//Vector3r &extents = (static_cast<Box*>(cg.get()))->extents;
	Vector3r extents(0.1,0.2,0.3);
	GaussianAnalyticalPropagatingWavePacket_1D* packet=dynamic_cast<GaussianAnalyticalPropagatingWavePacket_1D*>(state.get());
//	std::cerr<<"\nGl1_GaussianAnalyticalPropagatingWavePacket_1D\n";
	glScalef(2*extents[0],2*extents[1],2*extents[2]);
//	if (wire)
//	{
//		glutWireCube(1);
//	} else {
//		glutSolidCube(1);
//	}
	glBegin(GL_LINE_STRIP);
	glColor3v(shape->color);
	for(Real x=-32.0 ; x<32.0 ; x+=0.1 )
	{
//	(Real x,Real x0,Real t,Real k0,Real m, Real a, Real h)
		std::complex<Real> y = packet->waveFunctionValue_1D_positionRepresentation(x,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
		glVertex3d(x,std::real(y),0);
	}
	glColor3v(shape->color);
	for(Real x=-32.0 ; x<32.0 ; x+=0.1 )
	{
//	(Real x,Real x0,Real t,Real k0,Real m, Real a, Real h)
		std::complex<Real> y = packet->waveFunctionValue_1D_positionRepresentation(x,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
		glVertex3d(x,std::imag(y),0);
	}
	glColor3v(shape->color);
	for(Real x=-32.0 ; x<32.0 ; x+=0.1 )
	{
//	(Real x,Real x0,Real t,Real k0,Real m, Real a, Real h)
		std::complex<Real> y = packet->waveFunctionValue_1D_positionRepresentation(x,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
		glVertex3d(x,std::abs(y),0);
	}
	glEnd();
};


//	CREATE_LOGGER(Gl1_WaveFunctionGeometry);
//	bool Gl1_WaveFunctionGeometry::abs=true;
//	bool Gl1_WaveFunctionGeometry::real=false;
//	bool Gl1_WaveFunctionGeometry::imag=false;
//	Gl1_WaveFunctionGeometry::~Gl1_WaveFunctionGeometry(){};
//	void Gl1_WaveFunctionGeometry::go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&)
//	{
//	}

	CREATE_LOGGER(Gl1_WaveFunctionInteractionPhysics);
	bool Gl1_WaveFunctionInteractionPhysics::abs=true;
	bool Gl1_WaveFunctionInteractionPhysics::real=false;
	bool Gl1_WaveFunctionInteractionPhysics::imag=false;
	Gl1_WaveFunctionInteractionPhysics::~Gl1_WaveFunctionInteractionPhysics(){};
	void Gl1_WaveFunctionInteractionPhysics::go(const shared_ptr<IPhys>& ip, const shared_ptr<Interaction>& i, const shared_ptr<Body>& b1, const shared_ptr<Body>& b2, bool wireFrame)
	{
	}

#endif

