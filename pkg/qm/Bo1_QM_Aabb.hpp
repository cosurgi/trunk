// 2014 © Janek Kozicki <cosurgi@gmail.com>
 
#pragma once

#include <pkg/common/Dispatching.hpp>
#include "QMGeometryDisplay.hpp"

class Bo1_QMGeometryDisplay_Aabb : public BoundFunctor
{
	public :
		void go(const shared_ptr<Shape>& cm, shared_ptr<Bound>& bv, const Se3r&, const Body*);
	FUNCTOR1D(QMGeometryDisplay);
	YADE_CLASS_BASE_DOC(
		  // class name
		Bo1_QMGeometryDisplay_Aabb
		, // base class name
		BoundFunctor
		, // class description
		"Functor creating :yref:`Aabb` from :yref:`QMGeometryDisplay`."
	);
};

REGISTER_SERIALIZABLE(Bo1_QMGeometryDisplay_Aabb);

