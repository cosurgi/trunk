from __future__ import print_function
# this script demonstrates how to benchmark using timingEnabled: how to measure how much time each module takes.
from yade import pack,timing
#O.bodies.append([	sphere((0.2,0,0),.5,fixed=True), sphere((0.2,0.0,1.01),.5), ])
O.bodies.append(pack.regularHexa(pack.inAlignedBox((0,0,0),(10,10,1)),radius=.5,gap=0,fixed=True))
O.bodies.append(pack.regularOrtho(pack.inAlignedBox((3,3,3),(7,7,4)),radius=.05,gap=0))
O.engines=[
	ForceResetter(),
	FlatGridCollider(step=.2,aabbMin=(0,0,0),aabbMax=(10,10,5),verletDist=0.005),
	# InsertionSortCollider([Bo1_Sphere_Aabb()],sweepLength=0.005),
	InteractionLoop(
		[Ig2_Sphere_Sphere_ScGeom()],
		[Ip2_FrictMat_FrictMat_FrictPhys()],
		[Law2_ScGeom_FrictPhys_CundallStrack()],
	),
	NewtonIntegrator(damping=0.4,gravity=[0,0,-10]),
]
O.dt=.6*PWaveTimeStep()
O.saveTmp()
#O.step()
#while True:
#	O.step()
#	if len(O.interactions)>0 or O.bodies[1].state.pos[2]<.97: break
print('This will take a while, drink a coffee ;)')
O.timingEnabled=True
O.run(5000,True)
timing.stats()
import sys
#sys.exit(0)
