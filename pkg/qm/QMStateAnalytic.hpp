// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include "QMState.hpp"
#include <yade/pkg/common/Dispatching.hpp>
#include <yade/core/GlobalEngine.hpp>
#include <yade/core/Scene.hpp>
#include <stdexcept>

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   A N A L Y T I C A L   S T A T E
*
*********************************************************************************/

/*! @brief QMStateAnalytic contains quantum state information expressed using analytic formulas.
 *
 *  To allow propagation in time of these analytical solutions, the only universal attribute is current local time t
 *  (FIXME: currently nonrelativistic approach only).
 */
class QMStateAnalytic: public QMState
{
	public:
		virtual ~QMStateAnalytic();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			QMStateAnalytic
			, // base class
			QMState
			, // class description
"Analytic quantum mechanical state. It is expressed in terms of a mathematical function, which can be \
pretty arbitrary. It can be used to initialize the quantum mechanical state discretized into a grid or matrix, \
or to make comparisons between simulation results and analytical solutions. Derived classes will be specific \
analytic solutions to various cases."
			, // attributes, public variables
			((Real,t,0,,"Current local time in this packet $t=0$, FIXME: nonrelativistic approach."))
			, // constructor
			createIndex();
			, // python bindings
		);
		REGISTER_CLASS_INDEX(QMStateAnalytic,QMState);
};
REGISTER_SERIALIZABLE(QMStateAnalytic);

// FIXME: class QMStateAnalyticMixed: public QMStateAnalytic // ← to allow mixed states, eg: (φ+ψ)/√2

/*********************************************************************************
*
* F R E E L Y   M O V I N G   G A U S S I A N   W A V E P A C K E T
* 
*********************************************************************************/

/*! @brief FreeMovingGaussianWavePacket is a free propagating wavepacket of Gaussian shape.
 *
 * This class is used for testing if numerical solutions of Schrödinger
 * equation of a freely moving Gaussian wavepacket are correct.
 *
 * It is an analytical representation of a freely moving QM particle. It cannot
 * do anything else than free propagation in space.
 *
 */

class FreeMovingGaussianWavePacket: public QMStateAnalytic
{
	public:
		virtual ~FreeMovingGaussianWavePacket();
		virtual Complexr getValPos(Vector3r xyz);          /// return complex quantum aplitude at given positional representation coordinates
//		virtual void pyHandleCustomCtorArgs(boost::python::tuple& t, boost::python::dict& d)
//		{
//			std::cerr<<"\n GaussianWavePacket::pyHandleCustomCtorArgs\n";
//			std::cerr << k0 << "\n";
//			std::cerr << "d: \n" << d  << "\n";
//			std::cerr << "t: \n" << t  << "\n";
//		};
		virtual void postLoad(FreeMovingGaussianWavePacket& aaa)
		{ 
			std::cerr<<"\n GaussianWavePacket postLoad\n";
//			std::cerr << k0 << "\n";
//			std::cerr << aaa.k0 << "\n";
//			std::cerr<<"firstRun="<<firstRun<<"\n";
//			std::cerr<<"size="<<size<<"\n";
		}
		virtual Complexr waveFunctionValue_1D_positionRepresentation(Real x,Real x0,Real t,Real k0,Real m, Real a, Real h);
		YADE_CLASS_BASE_DOC_ATTRS_INIT_CTOR_PY(
			  // class name
			FreeMovingGaussianWavePacket
			, // base class
			QMStateAnalytic
			, // class description
"This is a wave function $\\psi$ initialized as a freely moving wave packet of Gaussian shape using following forumla:\n\n\
This is an analytically described wave packet function $\\psi$ with a Gaussian shape defined using following forumla:\
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
\t$t_0$ --- initial time when the packet was created, usually $t_0=0$\n\n\
\t$k_0$ --- initial wavenumber with which the packet was created\n\n\
\t$m$ --- mass of propagating particle\n\n\
\t$a$ --- initial Gaussian packet width\n\n\
\t$\\hbar$ --- Planck's constant divided by $2\\pi$\n\n\
FIXME: Above are the arguments to the function, they are not stored in this class.\
\n\n\
For higher number of dimensions the x and k are replaced with a vector, and thus reduce to a multiplication of Gaussians.\
"
			, // attributes, public variables
//			((Real,x,0,,"Position $x$ in the wavepacket"))                                   ← this is only used for evaluation, eg.: display
//			((Real,t,0,,"Current time $t$, assuming propagation of type exp(i(kx-ωt))"))     ← this is stored in parent class
//FIXME: where to store that?
			((Vector3r,x0  ,Vector3r::Zero(),,"Initial wave packet center at $t=0$"))
			((Real    ,t0  ,0               ,,"Initial wave packet center at $t=0$"))
			((Vector3r,k0  ,Vector3r::Zero(),,"Initial wavenumber $k_0$"))
			((Real    ,m   ,1               ,,"Particle mass"))
			((Real    ,a   ,1               ,,"Initial Gausian packet width $a$, sometimes calles $\\sigma$"))
			((Real    ,hbar,1               ,,"Planck's constant $h$ divided by $2\\pi$"))
			, // additional initializers (for references)
			, // constructor
			createIndex();
			, // python bindings
		);
		REGISTER_CLASS_INDEX(FreeMovingGaussianWavePacket,QMStateAnalytic);
};
REGISTER_SERIALIZABLE(FreeMovingGaussianWavePacket);


