// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <yade/pkg/common/Dispatching.hpp>
#include <yade/core/GlobalEngine.hpp>
#include <yade/core/Scene.hpp>
#include <stdexcept>

/*********************************************************************************
*
* A N A L Y T I C   P R O P A G A T I O N   of   S H R Ö D I N G E R  E Q.
*
*********************************************************************************/

/*! @brief Propagate analytical solution of Schrödinger equation in time.
 *
 * This engine doesn't solve anything, since analytical problems have only
 * analytical solutions, which must be found outside computer or using symbolic
 * manipulation. The purpose of this engine is to propagate in time the
 * solution found elsewhere.
 *
 * It is done only by incrementing the `t` variable in the analytical solution.
 *
 */

// FIXME: perhaps derive FreeMovingGaussianWavePacket from something so that this below could propagate harmonic oscillator too.
class SchrodingerAnalyticPropagator: public GlobalEngine
{
	public:
		virtual void action();
		virtual ~SchrodingerAnalyticPropagator();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			SchrodingerAnalyticPropagator
			, // base class
			GlobalEngine
			, // class description
"Propagate analytical solution of Schrödinger equation in time. This engine doesn't solve anything, \
since analytical problems have only analytical solutions, which must be found outside computer or using \
symbolic manipulation. The purpose of this engine is to propagate in time the solution found elsewhere. \
It is done only by incrementing the :yref:`time<QMStateAnalytic::t>` variable in the analytical solution."
			, // attributes, public variables
			, // constructor
			, // python bindings
	);
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE(SchrodingerAnalyticPropagator);

/*********************************************************************************
*
* K O S L O F F   P R O P A G A T I O N   o f   S H R Ö D I N G E R   E Q.
*
*********************************************************************************/

/*! @brief The solution to Shrödinger equation   ℍ̂ψ=iℏψ̇  uses the time evolution
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
Schrödinger equation. The solution to Shrödinger equation  ℍ̂ψ=iℏψ̇ uses the time evolution \
operator  Û=exp(-iℍ̂t/ℏ), and is following: ψ=Ûψ. The wavefunction ψ is evolving in time and \
here in SchrodingerKosloffPropagator it is calculated using Tal-Ezer and Kosloff approach \
found in [TalEzer1984]_"
			, // attributes, public variables
			, // constructor
			, // python bindings
	);
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE(SchrodingerKosloffPropagator);



