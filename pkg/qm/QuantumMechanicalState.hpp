// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <yade/pkg/common/Dispatching.hpp>
#include <yade/core/GlobalEngine.hpp>
#include <yade/core/Scene.hpp>
#include <stdexcept>

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


