from __future__ import print_function
#
# script for testing ScGeom shear computation
# Runs the same 2-sphere setup for useShear=False and for useShear=True
#

O.bodies.append([
	sphere([0,0,0],.5000001,fixed=True,color=(1,0,0)),
	sphere([0,0,1],.5000001,fixed=False,color=(0,0,1))
])
O.engines=[
	ForceResetter(),
	InsertionSortCollider([Bo1_Sphere_Aabb(),Bo1_Facet_Aabb()]),
	IGeomDispatcher([Ig2_Sphere_Sphere_ScGeom()]),
	IPhysDispatcher([Ip2_FrictMat_FrictMat_FrictPhys()]),
	RotationEngine(rotationAxis=[1,1,0],angularVelocity=.001,ids=[1]),
	ElasticContactLaw(label='elasticLaw'),
	PyRunner(iterPeriod=10000,command='interInfo()'),
	NewtonIntegrator()
]

O.dt=1e-8
O.saveTmp('init')

def interInfo():
	i=O.interactions[0,1]
	if 1:
		print(O.time,i.phys.shearForce)
	else:
		r1,r2=O.bodies[0].shape.radius,O.bodies[1].shape.radius
		theta=[e['angularVelocity'] for e in O.engines if e.name=='RotationEngine'][0]
		f=.5*(r1+r2)*theta*O.time*i.phys['ks']
		print(O.time,i.phys.shearForce,f,i.phys.shearForce[0]/f)


print('=========== no shear ============')
O.run(100000,True)
nIter=O.iter
print('============= shear =============')
O.loadTmp('init')
#elasticLaw['useShear']=True
O.run(nIter,True)
quit()
