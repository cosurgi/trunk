// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <yade/pkg/common/Dispatching.hpp>
#include <yade/core/Scene.hpp>
#include <stdexcept>

#include "QuantumMechanicalParameters.hpp"


/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   I N T E R A C T I O N   P H Y S I C S   (constitutive parameters of the contact)
*
*********************************************************************************/

/*! @brief QuantumMechanicalInteractionPhysics should describe an interaction between two wave functions.
 *
 * Currently I have only free moving particle without interactions, so it is not used yet.
 *
 * Evolution of interaction is governed by Law2_QuantumMechanicalInteractionGeometry_QuantumMechanicalInteractionPhysics_QuantumMechanicalInteractionPhysics:
 * that includes hamiltonian elements for each interaction.
 *
 */
class QuantumMechanicalInteractionPhysics: public IPhys
{
	public:
		virtual ~QuantumMechanicalInteractionPhysics();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			QuantumMechanicalInteractionPhysics
			, // base class
			IPhys
			, // class description
			"Representation of a single interaction of the WaveFunction type: storage for relevant parameters."
			, // attributes, public variables
			, // constructor
			createIndex();
			, // python bindings
		);
	DECLARE_LOGGER;
	REGISTER_CLASS_INDEX(QuantumMechanicalInteractionPhysics,IPhys);
};
REGISTER_SERIALIZABLE(QuantumMechanicalInteractionPhysics);


/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   I N T E R A C T I O N   G E O M E T R Y   (geometrical/spatial parameters of the contact)
*
*********************************************************************************/

/*! @brief QuantumMechanicalInteractionGeometry should describe geometrical aspects of interaction between two wave functions.
 *
 * Currently I have only free moving particle without interactions, so it is not used yet.
 *
 */
class QuantumMechanicalInteractionGeometry: public IGeom
{
	public:
		virtual ~QuantumMechanicalInteractionGeometry();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			QuantumMechanicalInteractionGeometry
			, // base class
			IGeom
			, // class description
			"Geometric representation of a single interaction of the WaveFunction"
			, // attributes, public variables
			, // constructor
			createIndex();
			, // python bindings
		);
	DECLARE_LOGGER;
	REGISTER_CLASS_INDEX(QuantumMechanicalInteractionGeometry,IGeom);
};
REGISTER_SERIALIZABLE(QuantumMechanicalInteractionGeometry);


/*********************************************************************************
*
* I P 2   I N T E R A C T I O N   P H Y S I C S   creates constitutive parameters of the contact
*
*********************************************************************************/

/*! @brief When two WaveFunctionState meet the QuantumMechanicalParameters of those two is used to create
 * QuantumMechanicalInteractionPhysics with corresponding parameters.
 *
 */
class Ip2_QuantumMechanicalParameters_QuantumMechanicalParameters_QuantumMechanicalInteractionPhysics: public IPhysFunctor
{
	public:
		virtual void go(const shared_ptr<Material>& pp1, const shared_ptr<Material>& pp2, const shared_ptr<Interaction>& interaction);
		FUNCTOR2D(QuantumMechanicalParameters,QuantumMechanicalParameters);
		DECLARE_LOGGER;
		YADE_CLASS_BASE_DOC_ATTRS(
			  // class name
			Ip2_QuantumMechanicalParameters_QuantumMechanicalParameters_QuantumMechanicalInteractionPhysics
			, // base class
			IPhysFunctor
			, // class description
			"Currently does nothing"
			, // attributes, public variables
//			((long,nothing,10,,"placeholder"))
		);
};
REGISTER_SERIALIZABLE(Ip2_QuantumMechanicalParameters_QuantumMechanicalParameters_QuantumMechanicalInteractionPhysics);


/*! @brief When any Material meets the QuantumMechanicalParameters a QuantumMechanicalInteractionPhysics is made.
 *
 * This can be used for infinite potential wells, where the DEM boxes serve as walls.
 *
 */

class Ip2_Material_QuantumMechanicalParameters_QuantumMechanicalInteractionPhysics: public IPhysFunctor
{
	public:
		virtual void go(const shared_ptr<Material>& pp1, const shared_ptr<Material>& pp2, const shared_ptr<Interaction>& interaction);
		FUNCTOR2D(Material,QuantumMechanicalParameters);
		DECLARE_LOGGER;
		YADE_CLASS_BASE_DOC_ATTRS(
			  // class name
			Ip2_Material_QuantumMechanicalParameters_QuantumMechanicalInteractionPhysics
			, // base class
			IPhysFunctor
			, // class description
			"Convert :yref:`QuantumMechanicalParameters` instance and :yref:`Material` instance to \
			:yref:`QuantumMechanicalInteractionPhysics` with corresponding parameters."
			, // attributes, public variables
		);
};
REGISTER_SERIALIZABLE(Ip2_Material_QuantumMechanicalParameters_QuantumMechanicalInteractionPhysics);


/*********************************************************************************
*
* L A W 2   In DEM it was used to calculate Fn and Fs between two interacting bodies
*
*********************************************************************************/

/*! @brief In DEM it was used to calculate Fn and Fs between two interacting bodies,
 * so this function takes following input:
 *    QuantumMechanicalInteractionPhysics
 *    QuantumMechanicalInteractionGeometry
 *    Interaction
 *
 * But what will it do? Maybe Quantum Field Theory will answer that.
 *
 */

class Law2_QuantumMechanicalInteractionGeometry_QuantumMechanicalInteractionPhysics_QuantumMechanicalInteractionPhysics: public LawFunctor
{
	public:
		bool go(shared_ptr<IGeom>& _geom, shared_ptr<IPhys>& _phys, Interaction* I);
		FUNCTOR2D(IGeom,QuantumMechanicalInteractionPhysics);
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			Law2_QuantumMechanicalInteractionGeometry_QuantumMechanicalInteractionPhysics_QuantumMechanicalInteractionPhysics
			, // base class
			LawFunctor
			, // class description
			"Constitutive law for the :yref:`WaveFunction-model<QuantumMechanicalParameters>`."
			, // attributes, public variables
//			((bool,empty,true,,"placeholder"))
			, // constructor
			, // python bindings
	);
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE(Law2_QuantumMechanicalInteractionGeometry_QuantumMechanicalInteractionPhysics_QuantumMechanicalInteractionPhysics);

