// 2014 © Janek Kozicki <cosurgi@gmail.com>
 
#pragma once

#include <yade/pkg/common/Dispatching.hpp>
#include "QuantumMechanicalGeometryDisplay.hpp"

class Bo1_QuantumMechanicalGeometryDisplay_Aabb : public BoundFunctor
{
	public :
		void go(const shared_ptr<Shape>& cm, shared_ptr<Bound>& bv, const Se3r&, const Body*);
	FUNCTOR1D(QuantumMechanicalGeometryDisplay);
	YADE_CLASS_BASE_DOC(
		  // class name
		Bo1_QuantumMechanicalGeometryDisplay_Aabb
		, // base class name
		BoundFunctor
		, // class description
		"Functor creating :yref:`Aabb` from :yref:`QuantumMechanicalGeometryDisplay`."
	);
};

REGISTER_SERIALIZABLE(Bo1_QuantumMechanicalGeometryDisplay_Aabb);

