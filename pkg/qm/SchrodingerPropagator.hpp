// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <pkg/common/Dispatching.hpp>
#include <core/GlobalEngine.hpp>
#include <core/Scene.hpp>
#include <stdexcept>
// FIXME - Kosloff method benefits from higher precision, check 
//   #include <boost/multiprecision/cpp_dec_float.hpp>
//   and http://www.boost.org/doc/libs/1_54_0/libs/math/example/bessel_zeros_example_1.cpp
#include <boost/math/special_functions/bessel.hpp>

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
 */

class SchrodingerKosloffPropagator: public GlobalEngine
{
	public:
		virtual void action();
		Real eMin() {return 0;}; // FIXME - it will have to scan the scene and find minimum potential
		Real eMax(); // this function scans the scene and currently finds (only) maximum momentum
		Real calcKosloffR() { return scene->dt*(eMax() - eMin())/(2*hbar);}; // calculate R parameter in Kosloff method
		Real calcKosloffG() { return scene->dt*eMin()/(2*hbar);};            // calculate G parameter in Kosloff method
		Complexr calcAKseriesCoefficient(int k,Real R) { return std::pow(Mathr::I,k)*(2.0 - Real(k==0))*(boost::math::cyl_bessel_j(k,R));};
		void doFFT_1D (const std::vector<Complexr>& in,std::vector<Complexr>& out);
		void doIFFT_1D(const std::vector<Complexr>& in,std::vector<Complexr>& out);
		void fftTest(); // debug + testing
		void calcPsiPlus_1(const std::vector<Complexr>& in,std::vector<Complexr>& out,/*FIXME - remove*/QMStateDiscrete* psi);
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
			// FIXME - it should get moved to QMParameters (maybe?)
			((Real    ,hbar,1               ,,"Planck's constant $h$ divided by $2\\pi$"))
			((Real    ,potential,0          ,,"Some potential barrier")) // FIXME
			((Real    ,potentialStart,25    ,,"Some potential barrier")) // FIXME
			((Real    ,potentialEnd  ,30    ,,"Some potential barrier")) // FIXME
			, // constructor
			, // python bindings
			.def("eMin"  ,&SchrodingerKosloffPropagator::eMin  ,"Get minimum energy.")
			.def("eMax"  ,&SchrodingerKosloffPropagator::eMax  ,"Get maximum energy.")
			.def("R"     ,&SchrodingerKosloffPropagator::calcKosloffR  ,"Calculate R parameter in Kosloff method.")
			.def("calcR" ,&SchrodingerKosloffPropagator::calcKosloffR  ,"Calculate R parameter in Kosloff method.")
			.def("G"     ,&SchrodingerKosloffPropagator::calcKosloffG  ,"Calculate G parameter in Kosloff method.")
			.def("calcG" ,&SchrodingerKosloffPropagator::calcKosloffG  ,"Calculate G parameter in Kosloff method.")
			.def("ak"    ,&SchrodingerKosloffPropagator::calcAKseriesCoefficient,"Calculate $a_k$ coefficient in Chebyshev polynomial expansion.")
			.def("fftTest",&SchrodingerKosloffPropagator::fftTest,"Do FFT test.")
	);
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE(SchrodingerKosloffPropagator);



