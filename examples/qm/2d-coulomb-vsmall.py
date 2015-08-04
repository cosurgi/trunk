#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 2
size1d   = 150
GRIDSIZE  = [128,80] # 2.5
halfSize  = [size1d,size1d*(1.0*GRIDSIZE[1]/GRIDSIZE[0]),0.1]# must be three components, because yade is inherently 3D and uses Vector3r. Remaining components will be used for AABB

# wavepacket parameters
k0                  = [-0.2,0.0,0.0]
gaussWidth          = [ 10  ,10 ,0  ]
t0                  = 300
x0                  = [0,-30,0]

# potential parameters
potentialCoefficient= [-1.0,0,0]
Pot_x = 10
Pot_y = 6
SH0= -size1d+(2.0*size1d/GRIDSIZE[0])*(1.0*GRIDSIZE[0]/2)+(1.0*size1d/GRIDSIZE[0])+Pot_x ## wiesza się gdy dam -20    } InteractionLoop.cpp:120
SH1= -size1d+(2.0*size1d/GRIDSIZE[1])*(1.0*GRIDSIZE[1]/2)+(1.0*size1d/GRIDSIZE[1])+Pot_y ##                  +15 ???  } bo wcale nie jest symetryczne!!
potentialCenter      = [ SH0,SH1 ,0  ]
potentialHalfSize    = halfSize # size ??
potentialMaximum     = -100000000; # negative puts ZERO at center, positive - puts this value.

O.engines=[
	StateDispatcher([
		St1_QMPacketGaussianWave(),
		St1_QMStPotentialCoulomb(),
	]),
	SpatialQuickSortCollider([
		Bo1_Box_Aabb(),
	]),
	InteractionLoop(
		[Ig2_2xQMGeometry_QMIGeom()],
		[Ip2_QMParameters_QMParametersCoulomb_QMIPhysCoulomb()],
		[Law2_QMIGeom_QMIPhysCoulomb()]
	),
	SchrodingerKosloffPropagator(steps=-1,threadNum=4),
]

partsScale = 2000

displayOptions1   = {'partsScale':partsScale, 'renderWireLight':True,'partAbsolute':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partReal':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'stepRender':["default hidden","hidden","frame","stripes","mesh"]
                    }
displayOptions2        = { 'renderWireLight':False,'partsScale':partsScale
                          ,'partAbsolute':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partImaginary':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partReal':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
#		    ,'partsSquared':1
                          ,'renderMaxTime':0.5
                          }
displayOptionsPot= { 'partAbsolute':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'partReal':['default wire', 'hidden', 'nodes', 'points', 'wire', 'surface']
                    ,'stepRender':["default hidden","hidden","frame","stripes","mesh"]
                    }

## 1: Analytical packet
analyticBody = QMBody()
analyticBody.shape     = QMGeometry(extents=halfSize,color=[0.6,0.6,0.6])
analyticBody.material  = QMParticle(dim=dimensions,hbar=1,m=1)
gaussPacketArg         = {'x0':x0,'t0':t0,'k0':k0,'a0':gaussWidth,'gridSize':GRIDSIZE}
analyticBody.state     = QMPacketGaussianWave(**gaussPacketArg)
#nid=O.bodies.append(analyticBody)        # do not append, it is used only to create the numerical one
#O.bodies[nid].state.setAnalytic()        # is propagated as analytical solution - no calculations involved

## 2: The numerical one:
numericalBody = QMBody()
numericalBody.shape     = QMGeometry(extents=halfSize,color=[1,1,1],displayOptions=[QMDisplayOptions(**displayOptions2)])
numericalBody.material  = analyticBody.material
numericalBody.state     = QMPacketGaussianWave(**gaussPacketArg) #,se3=[[0.5,0.5,0.5],Quaternion((1,0,0),0)])
nid=O.bodies.append(numericalBody)
O.bodies[nid].state.setNumeric()          # is being propagated by SchrodingerKosloffPropagator

## 3: The box with potential
potentialBody = QMBody()
potentialBody.shape     = QMGeometry(extents=potentialHalfSize,color=[0.1,0.4,0.1],wire=True,displayOptions=[QMDisplayOptions(partsScale=1,**displayOptionsPot)])
potentialBody.material  = QMParametersCoulomb(dim=dimensions,hbar=1,coefficient=potentialCoefficient)
potentialBody.state     = QMStPotentialCoulomb(se3=[potentialCenter,Quaternion((1,0,0),0)])
O.bodies.append(potentialBody)

## Define timestep for the calculations
#O.dt=.000001
O.dt=10

## Save the scene to file, so that it can be loaded later. Supported extension are: .xml, .xml.gz, .xml.bz2.
O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

try:
	from yade import qt
	qt.Controller()
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.controller.setWindowTitle("Gaussian packet in 2D Coulomb potential")
	qt.Renderer().blinkHighlight=False
	qt.View()
        qt.Renderer().light2Pos=[Pot_x,Pot_y,30]
	qt.views()[0].center(False,100) # median=False, suggestedRadius = 5

except ImportError:
	pass
#O.run(20000)

