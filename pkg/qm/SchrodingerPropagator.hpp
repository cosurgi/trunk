// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <pkg/common/Dispatching.hpp>
#include <core/GlobalEngine.hpp>
#include <core/Scene.hpp>
#include <stdexcept>
// FIXME - Kosloff method benefits a lot if higher precision numbers are used, check
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
		Real eMin();
		Real eMax();
		Real calcKosloffR(Real dt) { return dt*(eMax() - eMin())/(2*FIXMEatomowe_hbar);}; // calculate R parameter in Kosloff method
		Real calcKosloffG(Real dt) { return dt*eMin()/(2*FIXMEatomowe_hbar);};            // calculate G parameter in Kosloff method
		// FIXME: all ak can be precalculated, only recalculate if scene->dt changes
		// FIXME: same with get_full_potentialInteractionGlobal_psiGlobalTable() - it can precalculate, and recalculate only upon dirty is set.
		Complexr calcAK(int k,Real R) { return std::pow(Mathr::I,k)*(2.0 - Real(k==0))*(boost::math::cyl_bessel_j(k,R));};
		void calc_Hnorm_psi(const NDimTable<Complexr>& in,NDimTable<Complexr>& out,/*FIXME - remove*/QMStateDiscrete* psi);
		virtual ~SchrodingerKosloffPropagator();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			SchrodingerKosloffPropagator
			, // base class
			GlobalEngine
			, // class description
"Engine that propagates wavefunctions according to the time evolution operator of \
Schrödinger equation. The solution to Shrödinger equation  ℍ̂ψ=iħψ̇ uses the time evolution \
operator  Û=exp(-iℍ̂t/ħ), and is following: ψ=Ûψ. The wavefunction ψ is evolving in time and \
here in SchrodingerKosloffPropagator it is calculated using Tal-Ezer and Kosloff approach \
found in [TalEzer1984]_"
			, // attributes, public variables
			((Real    ,FIXMEatomowe_hbar,1               ,,"Planck's constant $h$ divided by $2\\pi$"))
			((Real    ,FIXMEatomowe_MASS,1               ,,"FIXME - should use mass of the particle"))
			((int     ,steps     ,-1     ,,"Override automatic selection of number of steps in Chebyshev expansion."))
			((bool    ,virialCheck,false ,,"Check energies using virial theorem (Coulomb potential ONLY - FIXME!!!!!!!!)."))
			, // constructor
			, // python bindings
			.def("eMin"  ,&SchrodingerKosloffPropagator::eMin  ,"Get minimum energy.")
			.def("eMax"  ,&SchrodingerKosloffPropagator::eMax  ,"Get maximum energy.")
			.def("R"     ,&SchrodingerKosloffPropagator::calcKosloffR  ,"Calculate R parameter in Kosloff method.")
			.def("calcR" ,&SchrodingerKosloffPropagator::calcKosloffR  ,"Calculate R parameter in Kosloff method.")
			.def("G"     ,&SchrodingerKosloffPropagator::calcKosloffG  ,"Calculate G parameter in Kosloff method.")
			.def("calcG" ,&SchrodingerKosloffPropagator::calcKosloffG  ,"Calculate G parameter in Kosloff method.")
			.def("ak"    ,&SchrodingerKosloffPropagator::calcAK,"Calculate $a_k$ coefficient in Chebyshev polynomial expansion series.")
	);
	DECLARE_LOGGER;
	private:
		TimeLimit timeLimit;
		
		// FIXME są różne typy, to jest podejrzane. Może w QMIPhys wystarczy trzymać NDimTable, a nie całe QMStateDiscreteGlobal ?
		NDimTable<Complexr>                      get_full_potentialInteractionGlobal_psiGlobalTable();
		boost::shared_ptr<QMStateDiscreteGlobal> get_full_psiGlobal__________________psiGlobalTable();
		void virialTheorem_Grid_check();
};
REGISTER_SERIALIZABLE(SchrodingerKosloffPropagator);



