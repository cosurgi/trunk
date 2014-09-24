// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <yade/pkg/common/Dispatching.hpp>
#include <yade/core/GlobalEngine.hpp>
#include <yade/core/Scene.hpp>
#include <stdexcept>

namespace py=boost::python;

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   B O D Y
*
*********************************************************************************/

/*! @brief QuantumMechanicalBody contains single 'body' (a particle) expressed in terms of quantum mechanics.
 *
 */
class QuantumMechanicalBody: public Body
{
	public:
		virtual ~QuantumMechanicalBody();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			QuantumMechanicalBody
			, // base class
			Body
			, // class description
			"Quantum mechanical body is a single 'body' (a particle) expressed in terms of quantum mechanics."
			, // attributes, public variables
//# FIXME: (Janek) Implementing Quantum Mechanics makes some DEM assumptions
//# invalid.  I think that we should rethink what base class State contains, so
//# that in QM we would not need to use this hack to hide some variables.
//# However it is great to note that only this little 'cosmetic' hack is needed
//# to make Quantum Mechanics possible in yade
//# See also: class QuantumMechanicalState, class QuantumMechanicalBody, gui/qt4/SerializableEditor.py
//# Also I think this makes a possibility to make another Body for SPH model (see hacks inside Body.hpp YADE_SPH)
//#
//# FIXME: so in fact if we decided to fix the inheritance tree we should remove
//#        following attributes from the base class
			((string,qtHide,"chain clumpId flags qtHide",Attr::readonly,
			"Space separated list of variables to hide in qt4 interface. \
			To fix the inheritance tree we should remove those attributes from the base class."))
			, // constructor
			, // python bindings
		);
};
REGISTER_SERIALIZABLE(QuantumMechanicalBody);


/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   S T A T E
*
*********************************************************************************/

/*! @brief QuantumMechanicalState contains quantum state information
 *
 */
class QuantumMechanicalState: public State
{
	public:
		virtual ~QuantumMechanicalState();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			QuantumMechanicalState
			, // base class
			State
			, // class description
			"Quantum mechanical state."
			, // attributes, public variables
			((string,qtHide,"angMom angVel blockedDOFs densityScaling inertia isDamped mass qtHide refOri refPos vel inertia",Attr::readonly,
			"Space separated list of variables to hide in qt4 interface. \
			To fix the inheritance tree we should remove those attributes from the base class.\
			Yes, even mass must be removed, although it is back in few of derived classes."))
			, // constructor
			createIndex();
			, // python bindings
		);
		REGISTER_CLASS_INDEX(QuantumMechanicalState,State);
};
REGISTER_SERIALIZABLE(QuantumMechanicalState);

/*********************************************************************************
*
* W A V E   F U N C T I O N   S T A T E
*
*********************************************************************************/

/*! @brief WaveFunctionState contains state information about each particle.
 *
 * A spatial position representation is used, on a lattice grid of a defined size in powers of two.
 * On this grid the complex amplitude is stored, which defines the probability distribution.
 *
 */
class WaveFunctionState: public QuantumMechanicalState
{
	public:
		virtual ~WaveFunctionState();
		void postLoad(WaveFunctionState&)
		{ 
			std::cerr<<"\nWaveFunctionState postLoad\n";
//			std::cerr<<"firstRun="<<firstRun<<"\n";
//			std::cerr<<"size="<<size<<"\n";
		}
		// FIXME: the lattice grid here vector<........>
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			WaveFunctionState
			, // base class
			QuantumMechanicalState
			, // class description
			"Wave function state information about a particle."
			, // attributes, public variables
//			((bool,firstRun,4096,,"Lattice size used to describe the wave function. For FFT purposes that should be a power of 2."))
//			((int,size,4096,,"Lattice size used to describe the wave function. For FFT purposes that should be a power of 2."))
//			((int,numSpatialDimensions,1,,"Number of spatial dimensions in which wavefunction exists"))
//			((std::vector<std::complex<Real> >,table,,,,"The FFT lattice grid "))
			//This is just Serialization test, FIXME: add this to self-tests
			//((std::vector< Real >,arealTable,,,,"The FFT lattice grid "))
			//((std::vector< std::vector< Real > >,table,,,,"The FFT lattice grid "))
			//((std::complex<Real>,complexNum,,,,"test complex "))
			, // constructor
			createIndex();
			, // python bindings
		);
		REGISTER_CLASS_INDEX(WaveFunctionState,QuantumMechanicalState);
};
REGISTER_SERIALIZABLE(WaveFunctionState);

/*! @brief GaussianWavePacket is a WaveFunctionState initialized as a Gaussian wave packet.
 */

class GaussianWavePacket: public WaveFunctionState
{
	public:
		virtual ~GaussianWavePacket();
		// FIXME: the lattice grid here vector<........>
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(
			  // class name
			GaussianWavePacket
			, // base class
			WaveFunctionState
			, // class description
"This is a wave function $\\psi$ initialized as a Gaussian wave packet using following forumla:\
\n\n\
.. math::\n\n\tA=\\frac{\\sqrt{2\\pi}}{\\sqrt{a\\sqrt{\\pi}}}\
\n\n\
.. math::\n\n\tc=\\frac{A a}{\\sqrt{2 \\pi }}e^{-\\frac{a^2 (k-k_0)^2}{2}}\
\n\n\
.. math::\n\n\t\\omega=\\frac{\\hbar k^2}{2 m}\
\n\n\
.. math::\n\n\t\\psi(x,t,k_0,m,a,\\hbar)=\\frac{1}{\\sqrt{2 \\pi}}\\int_{-\\infty }^{\\infty } c(k,k_0,a) e^{i (k x-\\omega t)} dk=\
\\frac{e^{-\\frac{m x^2+i a^2 k_0 (k_0 \\hbar t -2 m x)}{2 a^2 m+2 i \\hbar t}}}\
{\\sqrt[4]{\\pi } \\sqrt{a+\\frac{i \\hbar t }{a m}}}."
//.. math::\n\n\t\\omega=\\left(\\omega/.First\\left[\\text{Solve}\\left[\\hbar\\omega=\\frac{\\hbar^2 k^2}{2 m},\\omega\\right]\\right]\\right)=\\frac{\\hbar k^2}{2 m}"
//1/(Power(E,(m*Power(x,2) + Complex(0,1)*Power(a,2)*k0*(-2*m*x + k0*t*\[HBar]))/(2*Power(a,2)*m + Complex(0,2)*t*\[HBar]))*Power(Pi,0.25)*Sqrt(a + (Complex(0,1)*t*\[HBar])/(a*m)))
			, // attributes, public variables
//			((Real,xInit,0,,"Initial position $x$ of the wavepacket"))
//			((Real,tInit,0,,"Initial time $t$, assuming propagation of type exp(i(kx-ωt))"))
//			((Real,k0Init,0,,"Initial wavenumber $k_0$"))
//			((Real,mInit,1,,"Initial mass m"))
//			((Real,aInit,1,,"Initial Gausian packet width $a$"))
//			((Real,hbarInit,1,,"Planck's constant $\\hbar$"))
			, // constructor
			createIndex();
		);
		REGISTER_CLASS_INDEX(GaussianWavePacket,WaveFunctionState);
};
REGISTER_SERIALIZABLE(GaussianWavePacket);

class GaussianAnalyticalPropagatingWavePacket_1D: public QuantumMechanicalState
{
	public:
		virtual ~GaussianAnalyticalPropagatingWavePacket_1D();
		virtual std::complex<Real> waveFunctionValue_1D_positionRepresentation(Real x,Real x0,Real t,Real k0,Real m, Real a, Real h);
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(
			  // class name
			GaussianAnalyticalPropagatingWavePacket_1D
			, // base class
			QuantumMechanicalState
			, // class description
"This is an analytically described wave packet function $\\psi$ with a Gaussian shape defined using following forumla:\
\n\n\
.. math::\
\n\n\
\tA=\\frac{\\sqrt{2\\pi}}{\\sqrt{a\\sqrt{\\pi}}}\
\n\n\
\tc=\\frac{A a}{\\sqrt{2 \\pi }}e^{-\\frac{a^2 (k-k_0)^2}{2}}\
\n\n\
\t\\omega=\\frac{\\hbar k^2}{2 m}\
\n\n\
\t\\psi(x,x_0,t,k_0,m,a,\\hbar)=\\frac{1}{\\sqrt{2 \\pi}}\\int_{-\\infty }^{\\infty } c(k,k_0,a) e^{i (k (x - x_0)-\\omega t)} dk=\
\\frac{e^{-\\frac{m (x - x_0)^2+i a^2 k_0 (k_0 \\hbar t -2 m (x - x_0))}{2 a^2 m+2 i \\hbar t}}}\
{\\sqrt[4]{\\pi } \\sqrt{a+\\frac{i \\hbar t }{a m}}}.\
\n\n\
It is not pre-computed, it is only evaluated anylytically upon request, given some input parameters:\n\n\
\t$x$ --- position where the function is evaluated\n\n\
\t$x_0$ --- initial wavepacket center $x_0=0$ at $t=0$\n\n\
\t$t$ --- time at which the evaluation takes place (usually simulation :yref:`Omega.time`)\n\n\
\t$k_0$ --- initial wavenumber with which the packet was created\n\n\
\t$m$ --- mass of propagating particle\n\n\
\t$a$ --- initial Gaussian packet width\n\n\
\t$\\hbar$ --- Planck's constant divided by $2\\pi$\n\n\
FIXME: Above are the arguments to the function, they are not stored in this class.\
"
			, // attributes, public variables
//			((Real,x,0,,"Position $x$ in the wavepacket"))
//			((Real,t,0,,"Current time $t$, assuming propagation of type exp(i(kx-ωt))"))
//
//FIXME: where to store that?
			((Real,x0,0,,"Initial wave packet center at $t=0$"))
			((Real,k0,0,,"Initial wavenumber $k_0$"))
			((Real,m,1,,"Particle mass"))
			((Real,a,1,,"Initial Gausian packet width $a$"))
			((Real,hbar,1,,"Planck's constant $h$ divided by $2\\pi$"))
			, // constructor
			createIndex();
		);
		REGISTER_CLASS_INDEX(GaussianAnalyticalPropagatingWavePacket_1D,QuantumMechanicalState);
};
REGISTER_SERIALIZABLE(GaussianAnalyticalPropagatingWavePacket_1D);

/*********************************************************************************
*
* W A V E   F U N C T I O N   G E O M E T R Y
*
*********************************************************************************/

/*! @brief WaveFunctionGeometry contains geometrical information used to display the particle on screen
 *
 * Things like color, or if it's a wireframe to display, etc.
 *
 */
class WaveFunctionGeometry: public Shape
{
	public:
		virtual ~WaveFunctionGeometry();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(
			  // class name
			WaveFunctionGeometry
			, // base class
			Shape
			, // class description
			"Wave function geomterical (prepared, maybe precomputed, for display on screen) information about a particle."
			, // attributes, public variables
			, // constructor
			createIndex();
		);
		REGISTER_CLASS_INDEX(WaveFunctionGeometry,Shape);
};
REGISTER_SERIALIZABLE(WaveFunctionGeometry);


/*********************************************************************************
*
* W A V E   F U N C T I O N   M A T E R I A L
*
*********************************************************************************/

/*! @brief WavePacketParameters contains information about material out of which each particle is made.
 *
 * Actually a wave function has no material. Although later it may come useful to categorize wavefunctions
 * as being one of fundamental indistinguishable particles in the standard model: quarks (up, down, etc.),
 * electrons (muon, tau), neutrinos, etc.
 *
 */
class WavePacketParameters: public Material
{
	public:
		virtual ~WavePacketParameters();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(
			  // class name
			WavePacketParameters
			, // base class
			Material
			, // class description
			"Actually wave function has no material. For now this class is empty only for purposes of\
			keeping compatibility with yade. Although maybe later it may come useful to distinguish \
			fundamental particles in the standard model: quarks (up, down, etc.),\
			electrons (muon, tau), neutrinos, etc.\n\
			\n\
			Important thing to note is that this Material is\
			supposed to be shared between various different instances of particles, thus\
			it will make sense to create material for a fundamental particle\n\
			\n\
			But how does this apply to the simples Schrodinger wavepacket?\n\
			\n\
			Maybe eg. fermions and bosons will derive from this class too."
			, // attributes, public variables
//			((bool,isWaveFunction,true,,"This is only a placeholder in WavePacketParameters, not used for anything."))
			((string,qtHide,"density qtHide",Attr::readonly,"Space separated list of variables to hide in qt4 interface. \
			To fix the inheritance tree we should remove those attributes from the base class."))
			, // constructor
			createIndex();
	);
	REGISTER_CLASS_INDEX(WavePacketParameters,Material);
};
REGISTER_SERIALIZABLE(WavePacketParameters);


/*********************************************************************************
*
* W A V E   F U N C T I O N   I N T E R A C T I O N   P H Y S I C S   (constitutive parameters of the contact)
*
*********************************************************************************/

/*! @brief WaveFunctionInteractionPhysics should describe an interaction between two wave functions.
 *
 * Currently I have only free moving particle without interactions, so it is not used yet.
 *
 * Evolution of interaction is governed by Law2_WaveFunctionInteractionGeometry_WaveFunctionInteractionPhysics_WaveFunctionInteractionPhysics:
 * that includes hamiltonian elements for each interaction.
 *
 */
class WaveFunctionInteractionPhysics: public IPhys
{
	public:
		virtual ~WaveFunctionInteractionPhysics();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			WaveFunctionInteractionPhysics
			, // base class
			IPhys
			, // class description
			"Representation of a single interaction of the WaveFunction type: storage for relevant parameters."
			, // attributes, public variables
			, // constructor
			createIndex();
			, // python bindings
		);
	DECLARE_LOGGER;
	REGISTER_CLASS_INDEX(WaveFunctionInteractionPhysics,IPhys);
};
REGISTER_SERIALIZABLE(WaveFunctionInteractionPhysics);


/*********************************************************************************
*
* W A V E   F U N C T I O N   I N T E R A C T I O N   G E O M E T R Y   (geometrical/spatial parameters of the contact)
*
*********************************************************************************/

/*! @brief WaveFunctionInteractionGeometry should describe geometrical aspects of interaction between two wave functions.
 *
 * Currently I have only free moving particle without interactions, so it is not used yet.
 *
 */
class WaveFunctionInteractionGeometry: public IGeom
{
	public:
		virtual ~WaveFunctionInteractionGeometry();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			WaveFunctionInteractionGeometry
			, // base class
			IGeom
			, // class description
			"Geometric representation of a single interaction of the WaveFunction"
			, // attributes, public variables
			, // constructor
			createIndex();
			, // python bindings
		);
	DECLARE_LOGGER;
	REGISTER_CLASS_INDEX(WaveFunctionInteractionGeometry,IGeom);
};
REGISTER_SERIALIZABLE(WaveFunctionInteractionGeometry);


/*********************************************************************************
*
* I P 2   I N T E R A C T I O N   P H Y S I C S   creates constitutive parameters of the contact
*
*********************************************************************************/

/*! @brief When two WaveFunctionState meet the WavePacketParameters of those two is used to create
 * WaveFunctionInteractionPhysics with corresponding parameters.
 *
 */
class Ip2_WavePacketParameters_WavePacketParameters_WaveFunctionInteractionPhysics: public IPhysFunctor
{
	public:
		virtual void go(const shared_ptr<Material>& pp1, const shared_ptr<Material>& pp2, const shared_ptr<Interaction>& interaction);
		FUNCTOR2D(WavePacketParameters,WavePacketParameters);
		DECLARE_LOGGER;
		YADE_CLASS_BASE_DOC_ATTRS(
			  // class name
			Ip2_WavePacketParameters_WavePacketParameters_WaveFunctionInteractionPhysics
			, // base class
			IPhysFunctor
			, // class description
			"Currently does nothing"
			, // attributes, public variables
//			((long,nothing,10,,"placeholder"))
		);
};
REGISTER_SERIALIZABLE(Ip2_WavePacketParameters_WavePacketParameters_WaveFunctionInteractionPhysics);


/*! @brief When any Material meets the WavePacketParameters a WaveFunctionInteractionPhysics is made.
 *
 * This can be used for infinite potential wells, where the DEM boxes serve as walls.
 *
 */

class Ip2_Material_WavePacketParameters_WaveFunctionInteractionPhysics: public IPhysFunctor
{
	public:
		virtual void go(const shared_ptr<Material>& pp1, const shared_ptr<Material>& pp2, const shared_ptr<Interaction>& interaction);
		FUNCTOR2D(Material,WavePacketParameters);
		DECLARE_LOGGER;
		YADE_CLASS_BASE_DOC_ATTRS(
			  // class name
			Ip2_Material_WavePacketParameters_WaveFunctionInteractionPhysics
			, // base class
			IPhysFunctor
			, // class description
			"Convert :yref:`WavePacketParameters` instance and :yref:`Material` instance to \
			:yref:`WaveFunctionInteractionPhysics` with corresponding parameters."
			, // attributes, public variables
		);
};
REGISTER_SERIALIZABLE(Ip2_Material_WavePacketParameters_WaveFunctionInteractionPhysics);


/*********************************************************************************
*
* L A W 2   In DEM it was used to calculate Fn and Fs between two interacting bodies
*
*********************************************************************************/

/*! @brief In DEM it was used to calculate Fn and Fs between two interacting bodies,
 * so this function takes following input:
 *    WaveFunctionInteractionPhysics
 *    WaveFunctionInteractionGeometry
 *    Interaction
 *
 * But what will it do? Maybe Quantum Field Theory will answer that.
 *
 */

class Law2_WaveFunctionInteractionGeometry_WaveFunctionInteractionPhysics_WaveFunctionInteractionPhysics: public LawFunctor
{
	public:
		bool go(shared_ptr<IGeom>& _geom, shared_ptr<IPhys>& _phys, Interaction* I);
		FUNCTOR2D(IGeom,WaveFunctionInteractionPhysics);
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			Law2_WaveFunctionInteractionGeometry_WaveFunctionInteractionPhysics_WaveFunctionInteractionPhysics
			, // base class
			LawFunctor
			, // class description
			"Constitutive law for the :yref:`WaveFunction-model<WavePacketParameters>`."
			, // attributes, public variables
//			((bool,empty,true,,"placeholder"))
			, // constructor
			, // python bindings
	);
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE(Law2_WaveFunctionInteractionGeometry_WaveFunctionInteractionPhysics_WaveFunctionInteractionPhysics);


/*********************************************************************************
*
* K O S L O F F   P R O P A G A T I O N   o f   S H R O D I N G E R   E Q.
*
*********************************************************************************/

/*! @brief The solution to Shrodinger equation   ℍ̂ψ=iℏψ̇  uses the time evolution
 * operator  Û=exp(-iℍ̂t/ℏ), and is following: ψ=Ûψ. The wavefunction ψ is
 * evolving in time and here in SchrodingerKosloffPropagator it is calculated
 * using Kosloff approach, found in 
 * H.Tal-Ezer, R.Kosloff "An accurate and efficient scheme for propagating the
 * time dependent Schrödinger equation", 1984
 *
 * But what will it do? Maybe Quantum Field Theory will answer that.
 *
 */

class SchrodingerKosloffPropagator: public GlobalEngine
{
	public:
		virtual ~SchrodingerKosloffPropagator();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			SchrodingerKosloffPropagator
			, // base class
			GlobalEngine
			, // class description
			"Engine that propagates wavefunctions according to the time evolution operator of \
			Schrödinger equation. The propagation is done using method from H.Tal-Ezer, R.Kosloff \
			\"An accurate and efficient scheme for propagating the time dependent Schrödinger \
			equation\", 1984."
			, // attributes, public variables
			, // constructor
			, // python bindings
	);
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE(SchrodingerKosloffPropagator);


/*********************************************************************************
*
* W A V E   F U N C T I O N   O P E N   G L   D I S P L A Y
*
*********************************************************************************/

#ifdef YADE_OPENGL
#include<yade/pkg/common/GLDrawFunctors.hpp>
class Gl1_GaussianAnalyticalPropagatingWavePacket_1D: public GlShapeFunctor
{
	public:
		virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&);
		virtual ~Gl1_GaussianAnalyticalPropagatingWavePacket_1D();
		RENDERS(WaveFunctionGeometry);
		DECLARE_LOGGER;
//		YADE_CLASS_BASE_DOC_STATICATTRS( //FIXME - not recognized when static attrs are not used? ????????????????????
//			  // class name
//			Gl1_GaussianAnalyticalPropagatingWavePacket_1D
//			, // base class
//			GlShapeFunctor
//			, // class description
//			"Render :yref:`GaussianAnalyticalPropagatingWavePacket_1D`."
//			, // attributes, public variables
//			((bool,abs,true,,"Show absolute probability"))
//			((bool,real,false,,"Show only real component"))
//			((bool,imag,false,,"Show only imaginary component"))
//		);
//};
//REGISTER_SERIALIZABLE(Gl1_GaussianAnalyticalPropagatingWavePacket_1D);

//class Gl1_GaussianAnalyticalPropagatingWavePacket_1D : public GlShapeFunctor
//{
//	public :
//		virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&);
//	RENDERS(GaussianAnalyticalPropagatingWavePacket_1D);
		YADE_CLASS_BASE_DOC_STATICATTRS(
			  // class name
			Gl1_GaussianAnalyticalPropagatingWavePacket_1D
			, // base class
			GlShapeFunctor
			, // class description
			"Renders :yref:`GaussianAnalyticalPropagatingWavePacket_1D` object"
			, // static public attributes
			((bool,abs,true,,"Show absolute probability"))
		);
};
REGISTER_SERIALIZABLE(Gl1_GaussianAnalyticalPropagatingWavePacket_1D);

//class Gl1_WaveFunctionGeometry: public GlShapeFunctor
//{
//	public:
//		virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&);
//		virtual ~Gl1_WaveFunctionGeometry();
//		RENDERS(WaveFunctionGeometry);
//		DECLARE_LOGGER;
//		YADE_CLASS_BASE_DOC_STATICATTRS(
//			  // class name
//			Gl1_WaveFunctionGeometry
//			, // base class
//			GlShapeFunctor
//			, // class description
//			"Render :yref:`WaveFunctionGeometry`."
//			, // attributes, public variables
//			((bool,abs,true,,"Show absolute probability"))
//			((bool,real,false,,"Show only real component"))
//			((bool,imag,false,,"Show only imaginary component"))
//		);
//};
//REGISTER_SERIALIZABLE(Gl1_WaveFunctionGeometry);


class Gl1_WaveFunctionInteractionPhysics: public GlIPhysFunctor
{
	public: 
		virtual void go(const shared_ptr<IPhys>&,const shared_ptr<Interaction>&,const shared_ptr<Body>&,const shared_ptr<Body>&,bool wireFrame);
		virtual ~Gl1_WaveFunctionInteractionPhysics();
		RENDERS(WaveFunctionInteractionPhysics);
		DECLARE_LOGGER;
		YADE_CLASS_BASE_DOC_STATICATTRS(
			  // class name
			Gl1_WaveFunctionInteractionPhysics
			, // base class
			GlIPhysFunctor
			, // class description
			"Render :yref:`WaveFunctionInteractionPhysics` interactions."
			, // attributes, public variables
			((bool,abs,true,,"Show absolute probability"))
			((bool,real,false,,"Show only real component"))
			((bool,imag,false,,"Show only imaginary component"))
		);
};
REGISTER_SERIALIZABLE(Gl1_WaveFunctionInteractionPhysics);
#endif

