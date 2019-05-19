from __future__ import print_function
from builtins import range
O.bodies.append(sphere([0,0,0],1,fixed=False))
O.engines=[
	ForceResetter(),
	PyRunner(command='O.forces.addMove(0,(1e-2,0,0))',iterPeriod=1),
	NewtonIntegrator()
]

for i in range(0,20):
	O.step()
	print(O.forces.f(0),O.bodies[0].state.pos)
quit()
