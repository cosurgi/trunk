#!/usr/bin/python
# -*- coding: utf-8 -*-

# PICK NUMBER OF DIMENSIONS (1,2 or 3):
dimensions= 2
size      = 100 if dimensions==1 else 10
halfSize  = Vector3(size,size if dimensions>1 else 0.1,size if dimensions>2 else 0.1)

## This is a simple test:
## - a freely moving particle according to Schrodinger equation is calculated using Tal-Ezer Kosloff 1984 method
## - it is compared with the same movement, but calculated analytically
## The error between numerical and analytical solution is plot on the graph

O.engines=[
	InsertionSortCollider([
		Bo1_QMGeometryDisplay_Aabb(),
	]),
# No particle interactions yet, only a free propagating particle. First step will be to introduce
# potentials, then interactions between moving particles.
	#InteractionLoop(
		#[Ig2_Sphere_Sphere_ScGeom(),Ig2_Box_Sphere_ScGeom()],
		#[Ip2_FrictMat_FrictMat_FrictPhys()],
		#[Law2_ScGeom_FrictPhys_CundallStrack()]
	#),
#	SchrodingerKosloffPropagator(),
# FIXME: perhaps derive FreeMovingGaussianWavePacket from something so that this below could propagate harmonic oscillator too.
	SchrodingerAnalyticPropagator()
]


## Two particles are created - the analytical one, and the numerical one. They
## do not interact, they are two separate calculations in fact.

## The analytical one:

analyticBody = QMBody()
# make sure it will not interact with the other particle (although interaction is not possible/implemented anyway)
analyticBody.groupMask = 2
analyticBody.shape     = QMGeometryDisplay(halfSize=halfSize,color=[0.9,0.9,0.9])
# it's too simple now. Later we will have quarks (up, down, etc.), leptons and bosons as a material.
# So no material for now.
analyticBody.material  = None
gaussPacket            = FreeMovingGaussianWavePacket(dim=dimensions,x0=[0,0,0],t0=0,k0=[3,-2 if dimensions > 1 else 0,1 if dimensions > 2 else 0],m=1,a=1,hbar=1)
analyticBody.state     = gaussPacket
O.bodies.append(analyticBody)

## The numerical one:
numericalBody = QMBody()
# make sure it will not interact with the other particle (although interaction is not possible/implemented anyway)
numericalBody.groupMask = 1
numericalBody.shape     = QMGeometryDisplay(halfSize=halfSize,color=[1,1,1])
numericalBody.material  = None
# Initialize the discrete wavefunction using the analytical gaussPacket created earlier.
# The wavefunction shape can be anything - as long as it is normalized, in this case the Gauss shape is used.
# The grid size must be a power of 2 to allow FFT. Here 2**12=4096 is used.
numericalBody.state     = QMStateDiscrete(creator=gaussPacket,dim=dimensions,positionSize=halfSize*2.0,gridSize=(2**12 if dimensions==1 else 128))
O.bodies.append(numericalBody)

## Define timestep for the calculations
O.dt=.000001

## Save the scene to file, so that it can be loaded later. Supported extension are: .xml, .xml.gz, .xml.bz2.
O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

try:
	from yade import qt
        O.step()
	#qt.View()
	qt.Controller()
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.Renderer().blinkHighlight=False
except ImportError:
	pass
#O.run(20000)

