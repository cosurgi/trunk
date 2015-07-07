// 2015 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include "QMStateAnalytic.hpp"
#include "QMPacketGaussianWave.hpp"

/*********************************************************************************
*
* Q U A N T U M   H Y D R O G E N   W A V E F U N C T I O N
* 
*********************************************************************************/

/*! @brief QMPacketHydrogenEigenFunc is a wave function psi initialized as a quantum harmonic 
 * oscillator at given energy level n.
 *
 */

#include <boost/rational.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/factorials.hpp>

class QMPacketHydrogenEigenFunc: public QMStateAnalytic
{
	public:
		virtual ~QMPacketHydrogenEigenFunc();
		YADE_CLASS_BASE_DOC_ATTRS_INIT_CTOR_PY(
			  // class name
			QMPacketHydrogenEigenFunc
			, // base class
			QMStateAnalytic
			, // class description
// FIXME, FIXME, FIXME, FIXME, FIXME - bad description!!!
"This is a wave function $\\psi$ initialized as a quantum harmonic oscillator at given energy level n using folllowing forumla:\n\n\
\n\n\
.. math::\
\n\n\
\tFIXME FIXME A=\\frac{\\sqrt{2\\pi}}{\\sqrt{a\\sqrt{\\pi}}}\
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
// FIXME, FIXME, FIXME, FIXME, FIXME - bad description!!!
			, // attributes, public variables
			((Vector3i ,energyLevel,Vector3i::Zero(),,"Energy level 'n' in xyz-directions of quantum harmonic oscillator"))
			((Real     ,t0  ,0                      ,,"Initial time for generated wave packet"))
			((Vector3r ,x0  ,Vector3r::Zero()        ,,"Initial wave packet center at $t=0$"))
			, // additional initializers (for references)
			, // constructor
			createIndex();
			, // python bindings
		);
		REGISTER_CLASS_INDEX(QMPacketHydrogenEigenFunc,QMStateAnalytic);
};
REGISTER_SERIALIZABLE(QMPacketHydrogenEigenFunc);


/*********************************************************************************
*
* H Y D R O G E N   W A V E F U N C T I O N   F U N C T O R
*
*********************************************************************************/

class St1_QMPacketHydrogenEigenFunc: public St1_QMStateAnalytic
{
	public:
		FUNCTOR1D(QMPacketHydrogenEigenFunc);
		YADE_CLASS_BASE_DOC(St1_QMPacketHydrogenEigenFunc /* class name */, St1_QMStateAnalytic /* base class */
			, "Functor creating :yref:`QMPacketHydrogenEigenFunc` from :yref:`QMParameters`." // class description
		);
		//! return complex quantum aplitude at given positional representation coordinates
		virtual Complexr getValPos(Vector3r xyz, const QMParameters* par, const QMState* qms);
	private:
		Real      En_1D      (int n);
		Real      En_2D      (int n);
		Real      En_3D      (int n);
		Complexr  quantumHydrogenWavefunction_1D(int n, bool even    , Real x);         // FIXME: assume hbar=1, mass=1
		Complexr  quantumHydrogenWavefunction_2D(int n, int  l       , Real x, Real y); // FIXME: assume hbar=1, mass=1, frequency=1
		Complexr  quantumHydrogenWavefunction_3D(int n, int  l, int m, Vector3r xyz); // FIXME: assume hbar=1, mass=1, frequency=1
};
REGISTER_SERIALIZABLE(St1_QMPacketHydrogenEigenFunc);


/*********************************************************************************
*
* Q U A N T U M   H Y D R O G E N  -  F R E E   P R O P A G A T I N G
* 
*********************************************************************************/

class QMPacketHydrogenEigenGaussianWave: public QMStateAnalytic
{
	public:
		virtual ~QMPacketHydrogenEigenGaussianWave();
		YADE_CLASS_BASE_DOC_ATTRS_INIT_CTOR_PY(
			  // class name
			QMPacketHydrogenEigenGaussianWave
			, // base class
			QMStateAnalytic
			, // class description
// FIXME, FIXME, FIXME, FIXME, FIXME - bad description!!!
"This is a wave function $\\psi$ initialized as a quantum harmonic oscillator at given energy level n using folllowing forumla:\n\n\
"
// FIXME, FIXME, FIXME, FIXME, FIXME - bad description!!!
			, // attributes, public variables
			((shared_ptr<QMPacketHydrogenEigenFunc> ,hydrogenEigenFunc,,,"Hydrogen eigenfunction that is propagating freely"))
			((shared_ptr<QMPacketGaussianWave>      ,gaussianWave     ,,,"Freely propagating packet"))
			, // additional initializers (for references)
			, // constructor
			createIndex();
			, // python bindings
		);
		REGISTER_CLASS_INDEX(QMPacketHydrogenEigenGaussianWave,QMStateAnalytic);
};
REGISTER_SERIALIZABLE(QMPacketHydrogenEigenGaussianWave);


/*********************************************************************************
*
* H Y D R O G E N  -  F R E E   P R O P A G A T I N G   F U N C T O R
*
*********************************************************************************/

class St1_QMPacketHydrogenEigenGaussianWave: public St1_QMStateAnalytic
{
	public:
		virtual ~St1_QMPacketHydrogenEigenGaussianWave();
		FUNCTOR1D(QMPacketHydrogenEigenGaussianWave);
		YADE_CLASS_BASE_DOC(St1_QMPacketHydrogenEigenGaussianWave /* class name */, St1_QMStateAnalytic /* base class */
			, "Functor creating :yref:`QMPacketHydrogenEigenGaussianWave` from :yref:`QMParameters`." // class description
		);
		//! return complex quantum aplitude at given positional representation coordinates
		virtual Complexr getValPos_2particles(Vector3r xyz1, Vector3r xyz2, const QMParameters* par1, const QMParameters* par2, const QMState* qms);
	private:
//		Complexr  quantumFreePropagatingHydrogen_1D(int n, bool even    , Real x);         // FIXME: assume hbar=1, mass=1
//		Complexr  quantumFreePropagatingHydrogen_2D(int n, int  l       , Real x, Real y); // FIXME: assume hbar=1, mass=1, frequency=1
//		Complexr  quantumFreePropagatingHydrogen_3D(int n, int  l, int m, Vector3r xyz);   // FIXME: assume hbar=1, mass=1, frequency=1
};
REGISTER_SERIALIZABLE(St1_QMPacketHydrogenEigenGaussianWave);

