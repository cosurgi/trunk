#!/usr/bin/python
# -*- coding: utf-8 -*-

size1     = 10
size2     = 10
size3     = 10
halfSize1  = Vector3(size1, 0.1 ,0.1  )
halfSize2  = Vector3(size2,size2,0.1  )
halfSize3  = Vector3(size3,size3,size3)

O.engines=[
	InsertionSortCollider([
		Bo1_QMGeometryDisplay_Aabb(),
	]),
	SchrodingerAnalyticPropagator()
]

analyticBody1           = QMBody()
analyticBody1.groupMask = 2
analyticBody1.shape     = QMGeometryDisplay(halfSize=halfSize1,color=[0.6,0.6,0.6])
analyticBody1.material  = None
gaussPacket1            = FreeMovingGaussianWavePacket(dim=1,x0=[0,0,0],t0=0,k0=[3,0,0],m=1,a=1,hbar=1)
analyticBody1.state     = gaussPacket1
O.bodies.append(analyticBody1)

analyticBody2           = QMBody()
analyticBody2.groupMask = 4
analyticBody2.shape     = QMGeometryDisplay(halfSize=halfSize2,color=[0.8,0.8,0.8])
analyticBody2.material  = None
gaussPacket2            = FreeMovingGaussianWavePacket(dim=2,x0=[0,0,0],t0=0,k0=[3,0,0],m=1,a=1,hbar=1)
#gaussPacket2            = FreeMovingGaussianWavePacket(dim=2,x0=[0,0,0],t0=0,k0=[3,-2,0],m=1,a=1,hbar=1)
analyticBody2.state     = gaussPacket2
O.bodies.append(analyticBody2)

analyticBody3           = QMBody()
analyticBody3.groupMask = 8
analyticBody3.shape     = QMGeometryDisplay(halfSize=halfSize3,color=[1.0,1.0,1.0])
analyticBody3.material  = None
gaussPacket3            = FreeMovingGaussianWavePacket(dim=3,x0=[0,0,0],t0=0,k0=[3,0,0],m=1,a=1,hbar=1)
#gaussPacket3            = FreeMovingGaussianWavePacket(dim=3,x0=[0,0,0],t0=0,k0=[3,-2,1],m=1,a=1,hbar=1)
analyticBody3.state     = gaussPacket3
O.bodies.append(analyticBody3)

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


