/*************************************************************************
*  Copyright (C) 2004 by Olivier Galizzi                                 *
*  olivier.galizzi@imag.fr                                               *
*  Copyright (C) 2004 by Janek Kozicki                                   *
*  cosurgi@berlios.de                                                    *
*  Copyright (C) 2018 by Bruno Chareyre                                  *
*  bruno.chareyre@grenoble-inp.fr                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/
#pragma once

#include "Shape.hpp"
#include "Bound.hpp"
#include "State.hpp"
#include "Material.hpp"

#include <lib/base/Math.hpp>
#include <lib/serialization/Serializable.hpp>
#include <lib/multimethods/Indexable.hpp>

namespace yade { // Cannot have #include directive inside.

class Scene;
class Interaction;

class Body: public Serializable{
	public:
		// numerical types for storing ids
		using id_t = int ;
		// internal structure to hold some interaction of a body; used by InteractionContainer;
		using MapId2IntrT = std::map<Body::id_t, shared_ptr<Interaction> >;
		// groupMask type

		// bits for Body::flags
		enum { FLAG_BOUNDED=1, FLAG_ASPHERICAL=2, FLAG_SUBDOMAIN=4, FLAG_FLUIDDOMAINBBOX = 8}; /* add powers of 2 as needed */
		//! symbolic constant for body that doesn't exist.
		static const Body::id_t ID_NONE;
		//! get Body pointer given its id. 
		static const shared_ptr<Body>& byId(Body::id_t _id,Scene* rb=NULL);
		static const shared_ptr<Body>& byId(Body::id_t _id,shared_ptr<Scene> rb);
		
		//! Whether this Body is a Clump.
		//! @note The following is always true: \code (Body::isClump() XOR Body::isClumpMember() XOR Body::isStandalone()) \endcode
		bool isClump() const {return clumpId!=ID_NONE && id==clumpId;}
		//! Whether this Body is member of a Clump.
		bool isClumpMember() const {return clumpId!=ID_NONE && id!=clumpId;}
		//! Whether this body is standalone (neither Clump, nor member of a Clump)
		bool isStandalone() const {return clumpId==ID_NONE;}

		//! Whether this body has all DOFs blocked
		// inline accessors
		// logic: http://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit-in-c
		bool isDynamic() const { assert(state); return state->blockedDOFs!=State::DOF_ALL; }
		void setDynamic(bool d){ assert(state); if(d){ state->blockedDOFs=State::DOF_NONE; } else { state->blockedDOFs=State::DOF_ALL; state->vel=state->angVel=Vector3r::Zero(); } }
		bool isBounded() const {return flags & FLAG_BOUNDED; }
		void setBounded(bool d){ if(d) flags|=FLAG_BOUNDED; else flags&=~(FLAG_BOUNDED); }
		bool getIsSubdomain() const {return flags & FLAG_SUBDOMAIN; }
		void setIsSubdomain(bool d){ if(d) flags|=FLAG_SUBDOMAIN; else flags&=~(FLAG_SUBDOMAIN); }
		bool isAspherical() const {return flags & FLAG_ASPHERICAL; }
		void setAspherical(bool d){ if(d) flags|=FLAG_ASPHERICAL; else flags&=~(FLAG_ASPHERICAL); }
		bool getIsFluidDomainBbox() const {return flags & FLAG_FLUIDDOMAINBBOX;}
		void setIsFluidDomainBbox(bool d) {if(d) flags|=FLAG_FLUIDDOMAINBBOX; else flags&=~(FLAG_FLUIDDOMAINBBOX);}
		
		/*! Hook for clump to update position of members when user-forced reposition and redraw (through GUI) occurs.
		 * This is useful only in cases when engines that do that in every iteration are not active - i.e. when the simulation is paused.
		 * (otherwise, GLViewer would depend on Clump and therefore Clump would have to go to core...) */
		virtual void userForcedDisplacementRedrawHook(){return;}

		boost::python::list py_intrs();

		Body::id_t getId() const {return id;};
		unsigned int coordNumber() const;  // Number of neighboring particles

		mask_t getGroupMask() const {return groupMask; };
		bool maskOk(int mask) const;
		bool maskCompatible(int mask) const;
#ifdef YADE_MASK_ARBITRARY
		bool maskOk(const mask_t& mask) const;
		bool maskCompatible(const mask_t& mask) const;
#endif

		// only BodyContainer can set the id of a body
		friend class BodyContainer;

	// clang-format off
	YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(Body,Serializable,"A particle, basic element of simulation; interacts with other bodies.",
		((Body::id_t,id,Body::ID_NONE,Attr::readonly,"Unique id of this body."))

		((mask_t,groupMask,1,,"Bitmask for determining interactions."))
		((int,flags,FLAG_BOUNDED,Attr::readonly,"Bits of various body-related flags. *Do not access directly*. In c++, use isDynamic/setDynamic, isBounded/setBounded, isAspherical/setAspherical. In python, use :yref:`Body.dynamic`, :yref:`Body.bounded`, :yref:`Body.aspherical`."))
#ifdef YADE_MPI
		((int,subdomain,0,,"the subdomain this body belongs to."))
#endif
		((shared_ptr<Material>,material,,,":yref:`Material` instance associated with this body."))
		((shared_ptr<State>,state,new State,,"Physical :yref:`state<State>`."))
		((shared_ptr<Shape>,shape,,,"Geometrical :yref:`Shape`."))
		((shared_ptr<Bound>,bound,,,":yref:`Bound`, approximating volume for the purposes of collision detection."))
		((MapId2IntrT,intrs,,Attr::hidden,"Map from otherId to Interaction with otherId, managed by InteractionContainer."))
		((int,clumpId,Body::ID_NONE,Attr::readonly,"Id of clump this body makes part of; invalid number if not part of clump; see :yref:`Body::isStandalone`, :yref:`Body::isClump`, :yref:`Body::isClumpMember` properties. \n\nNot meant to be modified directly from Python, use :yref:`O.bodies.appendClumped<BodyContainer.appendClumped>` instead."))
		((long,chain,-1,,"Id of chain to which the body belongs."))
		((long,iterBorn,-1,Attr::readonly,"Step number at which the body was added to simulation."))
		((Real,timeBorn,-1,Attr::readonly,"Time at which the body was added to simulation."))
		,
		/* ctor */,
		/* py */
		//
		.def_readwrite("mat",&Body::material,"Shorthand for :yref:`Body::material`")
		.add_property("dynamic",&Body::isDynamic,&Body::setDynamic,"Whether this body will be moved by forces. (In c++, use ``Body::isDynamic``/``Body::setDynamic``) :ydefault:`true`")
		.add_property("bounded",&Body::isBounded,&Body::setBounded,"Whether this body should have :yref:`Body.bound` created. Note that bodies without a :yref:`bound <Body.bound>` do not participate in collision detection. (In c++, use ``Body::isBounded``/``Body::setBounded``) :ydefault:`true`")
		.add_property("aspherical",&Body::isAspherical,&Body::setAspherical,"Whether this body has different inertia along principal axes; :yref:`NewtonIntegrator` makes use of this flag to call rotation integration routine for aspherical bodies, which is more expensive. :ydefault:`false`")
#ifdef YADE_MPI
		.add_property("isSubdomain",&Body::getIsSubdomain,&Body::setIsSubdomain,"Whether this body is a subdomain should have :yref:`Body.bound` created. Subdomains` do not participate to collision detection with their own bodies, they may interact with external bodies and other subdomains through virtual interactions. (In c++, use ``Body::getIsSubdomain``/``Body::setIsSubdomain``) :ydefault:`false`")		
		.add_property("isFluidDomainBox",&Body::setIsFluidDomainBbox,&Body::getIsFluidDomainBbox,"Whether this body is a Fluid grid bounding box should have :yref:`Body.bound` created. FluidDomainBboxes` do not participate to collision detection with their own bodies, they may interact with external bodies and other subdomains through virtual interactions. (In c++, use ``Body::getIsFluidDomainBbox``/``Body::setIsFluidDomainBbox``) :ydefault:`false`")
#endif
		.add_property("mask",boost::python::make_getter(&Body::groupMask,boost::python::return_value_policy<boost::python::return_by_value>()),boost::python::make_setter(&Body::groupMask,boost::python::return_value_policy<boost::python::return_by_value>()),"Shorthand for :yref:`Body::groupMask`")
		.add_property("isStandalone",&Body::isStandalone,"True if this body is neither clump, nor clump member; false otherwise.")
		.add_property("isClumpMember",&Body::isClumpMember,"True if this body is clump member, false otherwise.")
		.add_property("isClump",&Body::isClump,"True if this body is clump itself, false otherwise.")
		.def_readwrite("chain",&Body::chain,"Returns Id of chain to which the body belongs.")
		.def("intrs",&Body::py_intrs,"Return list of all real interactions in which this body participates.")
	);
	// clang-format on
};
REGISTER_SERIALIZABLE(Body);

} // namespace yade

