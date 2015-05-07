// 2014 © Janek Kozicki <cosurgi@gmail.com>
 
#pragma once

#include <pkg/common/Dispatching.hpp>
#include "QMGeometry.hpp"

class Bo1_QMGeometry_Aabb : public BoundFunctor
{
	public :
		virtual void go(const shared_ptr<Shape>& cm, shared_ptr<Bound>& bv, const Se3r&, const Body*);
	FUNCTOR1D(QMGeometry);
	YADE_CLASS_BASE_DOC(
		  // class name
		Bo1_QMGeometry_Aabb
		, // base class name
		BoundFunctor
		, // class description
		"Functor creating :yref:`Aabb` from :yref:`QMGeometry`."
	);
};

REGISTER_SERIALIZABLE(Bo1_QMGeometry_Aabb);

