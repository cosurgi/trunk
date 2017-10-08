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
		std::vector<mask_t>			 allChannelMasks;
		void findAllEligibleGroupMasks();
		std::vector<bool>                        potentialCanChangeNOW_NOTstatic__ANYMORE;
		std::vector<NDimTable<Complexr> >        Vpsi_NOTstatic__ANYMORE;
		std::vector<NDimTable<Real> >		 kTable_NOTstatic__ANYMORE;
		std::vector<bool>                        haskTable_NOTstatic__ANYMORE;
		std::vector<NDimTable<Real> >		 dTable_NOTstatic__ANYMORE;
		std::vector<bool>			 hasdTable_NOTstatic__ANYMORE;

		NDimTable<Complexr>                      global_dTable;    // FIXME - ten tutaj to już jest prawdziwa bezsensowna proteza :(
		Real					 last_R;
		Real					 last_G;
		Real					 last_dtR;
		Real					 last_dtG;
		boost::shared_ptr<QMStateDiscreteGlobal> global_psiGlobal; //         Na pewno w wielu przypadkach nie działa
		virtual void action();
		Real eMin();
		Real eKinSelectedChannel(size_t mask_id);
		Real eMax();
		Real eMinSelectedChannel(size_t mask_id);
		Real eMaxSelectedChannel(size_t mask_id);
		Real calcKosloffR(Real dt);// { return dt*(eMax() - eMin())/(2*FIXMEatomowe_hbar);}; // calculate R parameter in Kosloff method
		Real calcKosloffG(Real dt);// { return dt*eMin()/(2*FIXMEatomowe_hbar);};            // calculate G parameter in Kosloff method
		// FIXME: all ak can be precalculated, only recalculate if scene->dt changes
		// FIXME: same with get_full_potentialInteractionGlobal_psiGlobalTable() - it can precalculate, and recalculate only upon dirty is set.
		Complexr calcAK(int k,Real R) { return std::pow(Mathr::I,k)*(2.0 - Real(k==0))*(boost::math::cyl_bessel_j(k,R));};
		void calc_Hnorm_psi(const NDimTable<Complexr>& in,NDimTable<Complexr>& out,/*FIXME - remove*/QMStateDiscrete* psi, size_t mask_id);
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
			((int     ,steps      ,-1    ,,"Override automatic selection of number of steps in Chebyshev expansion."))
			((int     ,threadNum  ,1     ,,"Number of FFT threads."))
			((bool    ,virialCheck,false ,,"Check energies using virial theorem (Coulomb potential ONLY - FIXME!!!!!!!!)."))
			((int     ,printIter  ,0     ,,"Print Kosloff interation."))
			((bool    ,doCopyTable,true  ,,"Whether to copy the NDimTable for calculation (memory waste, nice drawing)."))
			 // FIXME - osobne wielkości dampMarginX przy większej liczbie wymiarów.
			//((Real    ,dampMarginX   ,-1  ,,"Distance in atomic units from the edge, where damping is implemented."))
			((Real    ,dampMarginBandMin,-1  ,,"Distance in atomic units from the edge, where damping is implemented."))
			((Real    ,dampMarginBandMax,-1  ,,"Distance in atomic units from the edge, where damping is implemented."))
			((bool    ,dampFormulaSmooth,true,,"true - uses exp() with smoothed edge, false - uses 'traditional' exp() with discontinuity"))
			((Real    ,dampExponent     ,1   ,,"The coefficient 'c' in exp(-c*gamma(r))"))
			((bool    ,dampDebugPrint   ,true,,"When true the damping NDimTable is written to file 'dampDebugPrint', once."))
			((bool    ,hasDampTableCheck,false ,Attr::readonly,"Notify if dampTable was generated"))
			((bool    ,hasDampTableRegen,false ,              ,"Force regenerating damping NDimTable if options were changed"))

			// Próbuję jak najmniejszym wysiłkiem dopisać możliwość liczenia na kilku kanałach niezależnie.
			// groupMask , kilka kanałów
			((bool    ,useGroupMasks,false ,              ,"This SchrodingerKosloffPropagator will act only on wavefunctions & potential with given groupMask"))
			((mask_t  ,useGroupTheseMasks,    1 ,              ,"This SchrodingerKosloffPropagator will act only on wavefunctions & potential with given groupMask"))
//			((bool    ,useGroupMaskBool,false ,              ,"This SchrodingerKosloffPropagator will act only on wavefunctions & potential with given groupMask"))
//			((mask_t  ,useGroupThisMask,    1 ,              ,"This SchrodingerKosloffPropagator will act only on wavefunctions & potential with given groupMask"))
			((bool    ,kosloffR_needs_Update, true ,(Attr::hidden|Attr::noSave),"helper bool, to note that Kosloff R, G must be updated for some reason"))
			((bool    ,kosloffG_needs_Update, true ,(Attr::hidden|Attr::noSave),"helper bool, to note that Kosloff R, G must be updated for some reason"))
			((int     ,maxIter_NOTstatic__ANYMORE, 0 ,(Attr::hidden|Attr::noSave),"helper for printing maxIter"))

//			((bool    ,useGroupMaskBoolEnergyMinMax,false ,              ,"Use Emin,Emax from all SchrodingerKosloffPropagators that match this mask"))
//			((mask_t  ,useGroupMaskEnergyMinMax    ,    0 ,              ,"Use Emin,Emax from all SchrodingerKosloffPropagators that match this mask"))

			, // constructor
			, // python bindings
			.def("eMin"  ,&SchrodingerKosloffPropagator::eMin  ,"Get minimum energy.")
			.def("eKinSelectedChannel"  ,&SchrodingerKosloffPropagator::eKinSelectedChannel  ,"Get maximum allowed by the grid kinetic energy.")
			.def("eMax"  ,&SchrodingerKosloffPropagator::eMax  ,"Get maximum energy.")
			.def("eMinSelectedChannel"  ,&SchrodingerKosloffPropagator::eMinSelectedChannel,"Get minimum energy, ignoring other channels (ignoring useGroupMaskEnergyMinMax).")
			.def("eMaxSelectedChannel"  ,&SchrodingerKosloffPropagator::eMaxSelectedChannel,"Get maximum energy, ignoring other channels (ignoring useGroupMaskEnergyMinMax).")
			.def("R"     ,&SchrodingerKosloffPropagator::calcKosloffR  ,"Calculate R parameter in Kosloff method.")
			.def("calcR" ,&SchrodingerKosloffPropagator::calcKosloffR  ,"Calculate R parameter in Kosloff method.")
			.def("G"     ,&SchrodingerKosloffPropagator::calcKosloffG  ,"Calculate G parameter in Kosloff method.")
			.def("calcG" ,&SchrodingerKosloffPropagator::calcKosloffG  ,"Calculate G parameter in Kosloff method.")
			.def("ak"    ,&SchrodingerKosloffPropagator::calcAK,"Calculate $a_k$ coefficient in Chebyshev polynomial expansion series.")
	);
	DECLARE_LOGGER;
	private:
		TimeLimit timeLimit;

// this one was used to do some profiling to find the slowest parts of the algorithm
//		TimeLimit delay;
		
		// FIXME są różne typy, to jest podejrzane. Może w QMIPhys wystarczy trzymać NDimTable, a nie całe QMStateDiscreteGlobal ?
		const NDimTable<Complexr>&               get_full_potentialInteractionGlobal_psiGlobalTable(size_t mask_id);
		boost::shared_ptr<QMStateDiscreteGlobal> get_full_psiGlobal__________________psiGlobalTable(size_t mask_id);
		void virialTheorem_Grid_check();
};
REGISTER_SERIALIZABLE(SchrodingerKosloffPropagator);



#include<pkg/common/OpenGLRenderer.hpp>
#include <lib/opengl/OpenGLWrapper.hpp>

class GlExtra_QMEngine : public GlExtraDrawer {
	public:
	DECLARE_LOGGER;
	virtual void render();
	YADE_CLASS_BASE_DOC_ATTRS( // Class
		GlExtra_QMEngine
		,// Base
		GlExtraDrawer
		, // Doc
		"Find an instance of :yref:`SchrodingerKosloffPropagator` and show its damping 'dTable' or momentum 'kTable'."
		, // Attrs
		((bool,drawDTable,true,,"Draw dTable"))
		((bool,dampForceWire,false,,"Draw dTable and force wireframe"))
		((Vector3r,dampColor,Vector3r(1,1,1),,"dTable color"))
		((boost::shared_ptr<Serializable>,dampDisplayOptions,,,"..?"))
		((boost::shared_ptr<SchrodingerKosloffPropagator>,qmEngine,,Attr::hidden,"Drawing stuff from :yref:`SchrodingerKosloffPropagator`"))
	);
};

REGISTER_SERIALIZABLE(GlExtra_QMEngine);

