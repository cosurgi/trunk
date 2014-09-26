// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <yade/pkg/common/Dispatching.hpp>
#include <yade/core/Scene.hpp>
#include <stdexcept>

#include "QuantumMechanicalMaterial.hpp"


/*********************************************************************************
*
* W A V E   F U N C T I O N   I N T E R A C T I O N   P H Y S I C S   (constitutive parameters of the contact)
*
*********************************************************************************/

/*! @brief WaveFunctionInteractionPhysics should describe an interaction between two wave functions.
 *
 * Currently I have only free moving particle without interactions, so it is not used yet.
 *
 * Evolution of interaction is governed by Law2_WaveFunctionInteractionGeometry_WaveFunctionInteractionPhysics_WaveFunctionInteractionPhysics:
 * that includes hamiltonian elements for each interaction.
 *
 */
class WaveFunctionInteractionPhysics: public IPhys
{
	public:
		virtual ~WaveFunctionInteractionPhysics();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			WaveFunctionInteractionPhysics
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
	REGISTER_CLASS_INDEX(WaveFunctionInteractionPhysics,IPhys);
};
REGISTER_SERIALIZABLE(WaveFunctionInteractionPhysics);


/*********************************************************************************
*
* W A V E   F U N C T I O N   I N T E R A C T I O N   G E O M E T R Y   (geometrical/spatial parameters of the contact)
*
*********************************************************************************/

/*! @brief WaveFunctionInteractionGeometry should describe geometrical aspects of interaction between two wave functions.
 *
 * Currently I have only free moving particle without interactions, so it is not used yet.
 *
 */
class WaveFunctionInteractionGeometry: public IGeom
{
	public:
		virtual ~WaveFunctionInteractionGeometry();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			WaveFunctionInteractionGeometry
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
	REGISTER_CLASS_INDEX(WaveFunctionInteractionGeometry,IGeom);
};
REGISTER_SERIALIZABLE(WaveFunctionInteractionGeometry);


/*********************************************************************************
*
* I P 2   I N T E R A C T I O N   P H Y S I C S   creates constitutive parameters of the contact
*
*********************************************************************************/

/*! @brief When two WaveFunctionState meet the WavePacketParameters of those two is used to create
 * WaveFunctionInteractionPhysics with corresponding parameters.
 *
 */
class Ip2_WavePacketParameters_WavePacketParameters_WaveFunctionInteractionPhysics: public IPhysFunctor
{
	public:
		virtual void go(const shared_ptr<Material>& pp1, const shared_ptr<Material>& pp2, const shared_ptr<Interaction>& interaction);
		FUNCTOR2D(WavePacketParameters,WavePacketParameters);
		DECLARE_LOGGER;
		YADE_CLASS_BASE_DOC_ATTRS(
			  // class name
			Ip2_WavePacketParameters_WavePacketParameters_WaveFunctionInteractionPhysics
			, // base class
			IPhysFunctor
			, // class description
			"Currently does nothing"
			, // attributes, public variables
//			((long,nothing,10,,"placeholder"))
		);
};
REGISTER_SERIALIZABLE(Ip2_WavePacketParameters_WavePacketParameters_WaveFunctionInteractionPhysics);


/*! @brief When any Material meets the WavePacketParameters a WaveFunctionInteractionPhysics is made.
 *
 * This can be used for infinite potential wells, where the DEM boxes serve as walls.
 *
 */

class Ip2_Material_WavePacketParameters_WaveFunctionInteractionPhysics: public IPhysFunctor
{
	public:
		virtual void go(const shared_ptr<Material>& pp1, const shared_ptr<Material>& pp2, const shared_ptr<Interaction>& interaction);
		FUNCTOR2D(Material,WavePacketParameters);
		DECLARE_LOGGER;
		YADE_CLASS_BASE_DOC_ATTRS(
			  // class name
			Ip2_Material_WavePacketParameters_WaveFunctionInteractionPhysics
			, // base class
			IPhysFunctor
			, // class description
			"Convert :yref:`WavePacketParameters` instance and :yref:`Material` instance to \
			:yref:`WaveFunctionInteractionPhysics` with corresponding parameters."
			, // attributes, public variables
		);
};
REGISTER_SERIALIZABLE(Ip2_Material_WavePacketParameters_WaveFunctionInteractionPhysics);


/*********************************************************************************
*
* L A W 2   In DEM it was used to calculate Fn and Fs between two interacting bodies
*
*********************************************************************************/

/*! @brief In DEM it was used to calculate Fn and Fs between two interacting bodies,
 * so this function takes following input:
 *    WaveFunctionInteractionPhysics
 *    WaveFunctionInteractionGeometry
 *    Interaction
 *
 * But what will it do? Maybe Quantum Field Theory will answer that.
 *
 */

class Law2_WaveFunctionInteractionGeometry_WaveFunctionInteractionPhysics_WaveFunctionInteractionPhysics: public LawFunctor
{
	public:
		bool go(shared_ptr<IGeom>& _geom, shared_ptr<IPhys>& _phys, Interaction* I);
		FUNCTOR2D(IGeom,WaveFunctionInteractionPhysics);
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			Law2_WaveFunctionInteractionGeometry_WaveFunctionInteractionPhysics_WaveFunctionInteractionPhysics
			, // base class
			LawFunctor
			, // class description
			"Constitutive law for the :yref:`WaveFunction-model<WavePacketParameters>`."
			, // attributes, public variables
//			((bool,empty,true,,"placeholder"))
			, // constructor
			, // python bindings
	);
	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE(Law2_WaveFunctionInteractionGeometry_WaveFunctionInteractionPhysics_WaveFunctionInteractionPhysics);


/*********************************************************************************
*
* W A V E   F U N C T I O N   O P E N   G L   D I S P L A Y
*
*********************************************************************************/

#ifdef YADE_OPENGL
// This will come later, when I will have some interactions going on....
//class Gl1_WaveFunctionInteractionPhysics: public GlIPhysFunctor
//{
//	public: 
//		virtual void go(const shared_ptr<IPhys>&,const shared_ptr<Interaction>&,const shared_ptr<Body>&,const shared_ptr<Body>&,bool wireFrame);
//		virtual ~Gl1_WaveFunctionInteractionPhysics();
//		RENDERS(WaveFunctionInteractionPhysics);
//		DECLARE_LOGGER;
//		YADE_CLASS_BASE_DOC_STATICATTRS(
//			  // class name
//			Gl1_WaveFunctionInteractionPhysics
//			, // base class
//			GlIPhysFunctor
//			, // class description
//			"Render :yref:`WaveFunctionInteractionPhysics` interactions."
//			, // attributes, public variables
//			((bool,abs,true,,"Show absolute probability"))
//			((bool,real,false,,"Show only real component"))
//			((bool,imag,false,,"Show only imaginary component"))
//		);
//};
//REGISTER_SERIALIZABLE(Gl1_WaveFunctionInteractionPhysics);
#endif

