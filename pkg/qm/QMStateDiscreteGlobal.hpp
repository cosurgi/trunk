// 2015 © Janek Kozicki <cosurgi@gmail.com>

#pragma once
// #if 0 +#endif
#include "QMStateDiscrete.hpp"

/*********************************************************************************
*
* Q M   G L O B A L   S T A T E   in   D I S C R E T E   E N T A N G L E D   form
*
*********************************************************************************/

/*! @brief QMStateDiscreteGlobal contains wavefunction state information about entangled particles (usually whole simulation).
 *
 * It is obtained by doing tensor product of all component wavefunctions. The LawFunctor (FIXME: QMPotBarrierLaw,QMPotHarmonicLaw,QMPotCoulombLaw) that
 * creates interactions between particles is responsible for doing this. It is the LawFunctor, because the interaction is what is responsible
 * for requirement that wavefunctions become a tensor product and become entangled.
 *
 * Member variables:
 *
 *   std::vector<Body::id_t>   members    ← List of wavefunctions that got entangled here, their respective degrees of freedom are in
 *                                          the order of being entangled
 *   NDimTable    psiGlobalTable          ← is a sum of all potentials acting on member particles
 *
 */
class QMStateDiscreteGlobal: public QMStateDiscrete
{
	public:
		virtual ~QMStateDiscreteGlobal();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			QMStateDiscreteGlobal
			, // base class
			QMStateDiscrete
			, // class description
"QMStateDiscreteGlobal contains wavefunction state information about entangled particles (whole simulation). It is obtained by doing tensor product \
of all component wavecuntions. The LawFunctor (QMPotBarrierLaw,QMPotHarmonicLaw,QMPotCoulombLaw) that creates interactions between particles is \
responsible for doing this. It is the LawFunctor, because the interaction is what is responsible for requirement that wavefunctions become a tensor \
product and become entangled."
			, // attributes, public variables
			  ((std::vector<Body::id_t>,members,std::vector<Body::id_t>({}),Attr::readonly,"List of wavefunctions that got entangled here, their respective degrees of freedom are in the order of being entangled."))
//  vector<size_t>,QMStateDiscrete::gridSize    ← gridSize for members
//  vector<Real>,size                           ← positional representation sizes for members
			, // constructor
			createIndex();
			, // python bindings
		);
		REGISTER_CLASS_INDEX(QMStateDiscreteGlobal,QMStateDiscrete);

		// FIXME - Real or Complexr  ?
		//NDimTable<Complexr> allKTable;// FIXME - Real is only good for calculating 2ⁿᵈ derivative with FFT, so it won't work with Dirac's equation
		//NDimTable<Complexr> allPotentials;
		NDimTable<Complexr> psiGlobalTable;
};
REGISTER_SERIALIZABLE(QMStateDiscreteGlobal);

