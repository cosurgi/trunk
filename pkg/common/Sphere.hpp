#pragma once
#include<core/Shape.hpp>

// work around (old site, fixed bug) https://bugs.launchpad.net/yade/+bug/528509
// XXX: we need to think more about introducing yade namespace, to avoid such name conflicts, see https://gitlab.com/yade-dev/trunk/issues/57
// see comments there for explanation
namespace yade{
//FIXME - either put everything in yade namespace, or make some #define when compiling with gengeo
//        currently doxygen is lost when generating documentation, and Cylinder & ChainedCylinder
//        are not on the inheritance diagrams
//        Or maybe namespace names based on package name? (dem,lbm,qm..?)

class Sphere: public Shape{
	public:
		Sphere(Real _radius): radius(_radius){ createIndex(); }
		virtual ~Sphere () {};
	YADE_CLASS_BASE_DOC_ATTRS_CTOR(Sphere,Shape,"Geometry of spherical particle.",
		((Real,radius,NaN,,"Radius [m]")),
		createIndex(); /*ctor*/
	);
	REGISTER_CLASS_INDEX(Sphere,Shape);
};

}
// necessary
using namespace yade; 

// must be outside yade namespace
REGISTER_SERIALIZABLE(Sphere);
