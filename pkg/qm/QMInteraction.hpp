// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <pkg/common/Dispatching.hpp>
#include <core/Scene.hpp>
#include <stdexcept>

#include "QMParameters.hpp"


/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   I N T E R A C T I O N   P H Y S I C S   (constitutive parameters of the contact)
*
*********************************************************************************/

/*! @brief QMInteractionPhysics should describe an interaction between two wave functions.
 *
 * Currently I have only free moving particle without interactions, so it is not used yet.
 *
 * Evolution of interaction is governed by Law2_QMInteractionGeometry_QMInteractionPhysics_QMInteractionPhysics:
 * that includes hamiltonian elements for each interaction.
 *
 */
class QMInteractionPhysics: public IPhys
{
	public:
		virtual ~QMInteractionPhysics();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			QMInteractionPhysics
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
	REGISTER_CLASS_INDEX(QMInteractionPhysics,IPhys);
};
REGISTER_SERIALIZABLE(QMInteractionPhysics);


/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   I N T E R A C T I O N   G E O M E T R Y   (geometrical/spatial parameters of the contact)
*
*********************************************************************************/

/*! @brief QMInteractionGeometry should describe geometrical aspects of interaction between two wave functions.
 *
 * Currently I have only free moving particle without interactions, so it is not used yet.
 *
 */
class QMInteractionGeometry: public IGeom
{
	public:
		virtual ~QMInteractionGeometry();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			QMInteractionGeometry
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
	REGISTER_CLASS_INDEX(QMInteractionGeometry,IGeom);
};
REGISTER_SERIALIZABLE(QMInteractionGeometry);


/*********************************************************************************
*
* I P 2   I N T E R A C T I O N   P H Y S I C S   creates constitutive parameters of the contact
*
*********************************************************************************/

/*! @brief When two QMState meet the QMParameters of those two is used to create
 * QMInteractionPhysics with corresponding parameters.
 *
 */
class Ip2_QMParameters_QMParameters_QMInteractionPhysics: public IPhysFunctor
{
	public:
		virtual void go(const shared_ptr<Material>& pp1, const shared_ptr<Material>& pp2, const shared_ptr<Interaction>& interaction);
		FUNCTOR2D(QMParameters,QMParameters);
		DECLARE_LOGGER;
		YADE_CLASS_BASE_DOC_ATTRS(
			  // class name
			Ip2_QMParameters_QMParameters_QMInteractionPhysics
			, // base class
			IPhysFunctor
			, // class description
			"Currently does nothing"
			, // attributes, public variables
//			((long,nothing,10,,"placeholder"))
		);
};
REGISTER_SERIALIZABLE(Ip2_QMParameters_QMParameters_QMInteractionPhysics);


/*! @brief When any Material meets the QMParameters a QMInteractionPhysics is made.
 *
 * This can be used for infinite potential wells, where the DEM boxes serve as walls.
 *
 */

class Ip2_Material_QMParameters_QMInteractionPhysics: public IPhysFunctor
{
	public:
		virtual void go(const shared_ptr<Material>& pp1, const shared_ptr<Material>& pp2, const shared_ptr<Interaction>& interaction);
		FUNCTOR2D(Material,QMParameters);
		DECLARE_LOGGER;
		YADE_CLASS_BASE_DOC_ATTRS(
			  // class name
			Ip2_Material_QMParameters_QMInteractionPhysics
			, // base class
			IPhysFunctor
			, // class description
			"Convert :yref:`QMParameters` instance and :yref:`Material` instance to \
			:yref:`QMInteractionPhysics` with corresponding parameters."
			, // attributes, public variables
		);
};
REGISTER_SERIALIZABLE(Ip2_Material_QMParameters_QMInteractionPhysics);


/*********************************************************************************
*
* L A W 2   In DEM it was used to calculate Fn and Fs between two interacting bodies
*
*********************************************************************************/

/*! @brief In DEM it was used to calculate Fn and Fs between two interacting bodies,
 * so this function takes following input:
 *    QMInteractionPhysics
 *    QMInteractionGeometry
 *    Interaction
 *
 * But what will it do? Maybe Quantum Field Theory will answer that.
 *
 */

class Law2_QMInteractionGeometry_QMInteractionPhysics_QMInteractionPhysics: public LawFunctor
{
	public:
		bool go(shared_ptr<IGeom>& _geom, shared_ptr<IPhys>& _phys, Interaction* I);
		FUNCTOR2D(IGeom,QMInteractionPhysics);
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			Law2_QMInteractionGeometry_QMInteractionPhysics_QMInteractionPhysics
			, // base class
			LawFunctor
			, // class description
			"Constitutive law for the :yref:`WaveFunction-model<QMParameters>`."
			, // attributes, public variables
//			((bool,empty,true,,"placeholder"))
			, // constructor
			, // python bindings
	);
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE(Law2_QMInteractionGeometry_QMInteractionPhysics_QMInteractionPhysics);

