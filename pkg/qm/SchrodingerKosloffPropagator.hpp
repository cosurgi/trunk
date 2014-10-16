// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <yade/pkg/common/Dispatching.hpp>
#include <yade/core/GlobalEngine.hpp>
#include <yade/core/Scene.hpp>
#include <stdexcept>

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
			Schrödinger equation. The propagation is done using method from H.Tal-Ezer, R.Kosloff \
			\"An accurate and efficient scheme for propagating the time dependent Schrödinger \
			equation\", 1984."
			, // attributes, public variables
			, // constructor
			, // python bindings
	);
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE(SchrodingerKosloffPropagator);



