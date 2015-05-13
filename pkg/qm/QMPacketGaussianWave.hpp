// 2015 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include "QMStateAnalytic.hpp"

/*********************************************************************************
*
* F R E E L Y   M O V I N G   G A U S S I A N   W A V E P A C K E T
* 
*********************************************************************************/

/*! @brief QMPacketGaussianWave is a free propagating wavepacket of Gaussian shape.
 *
 * This class is used for testing if numerical solutions of Schrödinger
 * equation of a freely moving Gaussian wavepacket are correct.
 *
 * It is an analytical representation of a freely moving QM particle. It cannot
 * do anything else than free propagation in space.
 *
 */

class QMPacketGaussianWave: public QMStateAnalytic
{
	public:
		virtual ~QMPacketGaussianWave();
		YADE_CLASS_BASE_DOC_ATTRS_INIT_CTOR_PY(
			  // class name
			QMPacketGaussianWave
			, // base class
			QMStateAnalytic
			, // class description
"This is an analytically described wave packet function $\\psi$ with a Gaussian shape defined using following forumla (in :yref: `St1_QMPacketGaussianWave`):\
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
\t$a_0$ --- initial Gaussian packet width\n\n\
\t$\\hbar$ --- Planck's constant divided by $2\\pi$\n\n\
FIXME: Above are the arguments to the function, they are not stored in this class.\
\n\n\
For higher number of dimensions the x and k are replaced with a vector, and thus reduce to a multiplication of Gaussians.\
"
			, // attributes, public variables
			((Vector3r,x0  ,Vector3r::Zero(),,"Initial wave packet center at $t=0$"))
			((Real    ,t0  ,0               ,,"Initial wave packet center at $t=0$"))
			((Vector3r,k0  ,Vector3r::Zero(),,"Initial wavenumber $k_0$"))
			((Vector3r,a0  ,Vector3r::Zero(),,"Initial Gausian packet width $a_0$, sometimes called $\\sigma$"))
			, // additional initializers (for references)
			, // constructor
			createIndex();
			, // python bindings
		);
		REGISTER_CLASS_INDEX(QMPacketGaussianWave,QMStateAnalytic);
};
REGISTER_SERIALIZABLE(QMPacketGaussianWave);

/*********************************************************************************
*
* F R E E L Y   M O V I N G   G A U S S I A N   W A V E P A C K E T   F U N C T O R
*
*********************************************************************************/

class St1_QMPacketGaussianWave: public St1_QMStateAnalytic
{
	public:
		FUNCTOR1D(QMPacketGaussianWave);
		YADE_CLASS_BASE_DOC(St1_QMPacketGaussianWave /* class name */, St1_QMStateAnalytic /* base class */
			, "Functor creating :yref:`QMPacketGaussianWave` from :yref:`QMParticle`." // class description
		);
/*FIXME, make it:	private: */
		//! return complex quantum aplitude at given positional representation coordinates
		virtual Complexr getValPos(Vector3r xyz, const QMParameters* par, const QMState* qms);
		Complexr waveFunctionValue_1D_positionRepresentation(Real x,Real x0,Real t,Real t0,Real k0,Real m, Real a, Real h);
};
REGISTER_SERIALIZABLE(St1_QMPacketGaussianWave);

