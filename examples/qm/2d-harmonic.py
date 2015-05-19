#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 2
size_1d   = 10
halfSize  = [size_1d*2,size_1d*1.2,0.1]
size      = [x * 2 for x in halfSize]

# wavepacket parameters
k0_x         = 3
k0_y         = 2
gaussWidth_x = 1.0
gaussWidth_y = 2.0
potentialCoefficient= [0.2,0.5,0.5]

# potential parameters
potentialCenter   = [ 0, 0 ,0  ]
potentialHalfSize = halfSize # size ??
potentialValue    = 0.0

O.engines=[
	StateDispatcher([
		St1_QMPacketGaussianWave(),
		St1_QMStPotentialHarmonic(),
	]),
	SpatialQuickSortCollider([
		Bo1_Box_Aabb(),
	]),
	InteractionLoop(
		[Ig2_2xQMGeometry_QMIGeom()],
		[Ip2_QMParameters_QMParametersHarmonic_QMIPhysHarmonic()],
		[Law2_QMIGeom_QMIPhysHarmonic()]
	),
	SchrodingerKosloffPropagator(),
]

displayOptionsPot= { 'partAbsolute':['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partImaginary':['default draw: ∬ψ(x₁,y₁,x₂,y₂)dx₂dy₂ end_x₂'    ## FIXME - make it work,
		                                                                      ## but not in partImaginary, make such 4 choices along each
                                                                                      ## QMStateDiscrete::gridSize dimension
                    ### FIXME 2 - inicjalizacja - tylko tą wybraną wartością, nie ruszać reszty całego Menu, niech ono się samo jakoś
                    ###           przywraca jeśli zostało 'skasowane'
                    ###                  partReal="default hidden" ##  i koniec, bez wymieniania reszty
                    ###
		                     ,'draw: ∫ψ(x₁,y₁,x₂,y₂)dy₂' , 'draw: ∬ψ(x₁,y₁,x₂,y₂)dx₂dy₂ start_x₂'
				     ,'draw: ∬ψ(x₁,y₁,x₂,y₂)dx₂dy₂ end_x₂', 'draw: ∬ψ(x₁,y₁,x₂,y₂)dx₂dy₂ start_x₂,end_x₂']
                    ,'partReal':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
		    ,'stepRender':["default hidden","hidden","frame","stripes","mesh"]
		    ,'wire':True}

## 1: Analytical packet
analyticBody = QMBody()
analyticBody.shape     = QMGeometry(extents=halfSize,color=[0.6,0.6,0.6],partsScale=10)
analyticBody.material  = QMParticle(dim=dimensions,hbar=1,m=1)
gaussPacketArg         = {'x0':[0,2,0],'t0':0,'k0':[k0_x,k0_y,0],'a0':[gaussWidth_x,gaussWidth_y,0],'size':size,'gridSize':[2**7,2**6]}
analyticBody.state     = QMPacketGaussianWave(**gaussPacketArg)
#nid=O.bodies.append(analyticBody)        # do not append, it is used only to create the numerical one
#O.bodies[nid].state.blockedDOFs='xyzXYZ' # is propagated as analytical solution - no calculations involved

## 2: The numerical one:
numericalBody = QMBody()
numericalBody.shape     = QMGeometry(extents=halfSize,color=[1,1,1],partsScale=10)
numericalBody.material  = analyticBody.material
numericalBody.state     = QMPacketGaussianWave(**gaussPacketArg) #,se3=[[0.5,0.5,0.5],Quaternion((1,0,0),0)])
nid=O.bodies.append(numericalBody)
O.bodies[nid].state.blockedDOFs=''      # is being propagated by SchrodingerKosloffPropagator

## 3: The box with potential
potentialBody = QMBody()
potentialBody.shape     = QMGeometry(extents=potentialHalfSize,color=[0.1,0.4,0.1],partsScale=-10,**displayOptionsPot)
potentialBody.material  = QMParametersHarmonic(dim=dimensions,hbar=1,coefficient=potentialCoefficient)
potentialBody.state     = QMStPotentialHarmonic(se3=[potentialCenter,Quaternion((1,0,0),0)])
O.bodies.append(potentialBody)

## Define timestep for the calculations
#O.dt=.000001
O.dt=.1

## Save the scene to file, so that it can be loaded later. Supported extension are: .xml, .xml.gz, .xml.bz2.
O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

try:
	from yade import qt
	qt.Controller()
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.controller.setWindowTitle("Gaussian packet in 2D harmonic potential")
	qt.Renderer().blinkHighlight=False
	qt.View()
	qt.views()[0].center(False,5) # median=False, suggestedRadius = 5

except ImportError:
	pass
#O.run(20000)

