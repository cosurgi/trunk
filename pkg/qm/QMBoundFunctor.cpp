// 2014 © Janek Kozicki <cosurgi@gmail.com>
 
#include "Bo1_QM_Aabb.hpp"

YADE_PLUGIN((Bo1_QMGeometry_Aabb));

void Bo1_QMGeometry_Aabb::go(const shared_ptr<Shape>& shape, shared_ptr<Bound>& bound, const Se3r& se3, const Body* b)
{
	QMGeometry* qmGeom = static_cast<QMGeometry*>(shape.get());
	if(!bound){ bound=shared_ptr<Bound>(new Aabb); }
	Aabb* aabb=static_cast<Aabb*>(bound.get());

	Matrix3r r=se3.orientation.toRotationMatrix();
	Vector3r halfSize(Vector3r::Zero());
	for( int i=0; i<3; ++i )
		for( int j=0; j<3; ++j )
			halfSize[i] += std::abs( r(i,j) * qmGeom->halfSize[j] );
	
	aabb->min = se3.position-halfSize;
	aabb->max = se3.position+halfSize;
}

