#!/usr/bin/python
# -*- coding: utf-8 -*-

size1     = 10
size2     = 10
size3     = 10
halfSize1  = Vector3(size1, 0.1 ,0.1  )
halfSize2  = Vector3(size2,size2,0.1  )
halfSize3  = Vector3(size3,size3,size3)

O.engines=[
	StateDispatcher([
		St1_QMPacketGaussianWave(),
		St1_QMStateDiscrete()
	]),
	SpatialQuickSortCollider([
		Bo1_Box_Aabb(),
	]),
	SchrodingerAnalyticPropagator()
]

analyticBody1           = QMBody()
analyticBody1.groupMask = 2
analyticBody1.shape     = QMGeometry(extents=halfSize1,color=[0.6,0.6,0.6]
                                  ,partAbsolute=['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                  ,partImaginary=['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                  ,partReal=['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                  ,step=[0.2,0.2,0.2]
                                  ,renderMaxTime=0.5
                                  ,threshold3D=0.00001)
analyticBody1.material  = QMParticle(dim=1,hbar=1,m=1)
gaussPacket1            = QMPacketGaussianWave(x0=[6,0,0],t0=0,k0=[3,0,0],a0=[1,0,0])
analyticBody1.state     = gaussPacket1
O.bodies.append(analyticBody1)

analyticBody2           = QMBody()
analyticBody2.groupMask = 4
analyticBody2.shape     = QMGeometry(extents=halfSize2,color=[0.8,0.8,0.8]
                                  ,partAbsolute=['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                  ,partImaginary=['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                  ,partReal=['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                  ,step=[0.2,0.2,0.2]
                                  ,renderMaxTime=0.5
                                  ,threshold3D=0.00001)
analyticBody2.material  = QMParticle(dim=2,hbar=1,m=1)
gaussPacket2            = QMPacketGaussianWave(x0=[6,0,0],t0=0,k0=[3,0,0],a0=[1,1,0])
#gaussPacket2            = QMPacketGaussianWave(x0=[0,0,0],t0=0,k0=[3,-2,0],a0=[1,1,0])
analyticBody2.state     = gaussPacket2
O.bodies.append(analyticBody2)

analyticBody3           = QMBody()
analyticBody3.groupMask = 8
analyticBody3.shape     = QMGeometry(extents=halfSize3,color=[1.0,1.0,1.0]
                                  ,partAbsolute=['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                  ,partImaginary=['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                  ,partReal=['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
                                  ,step=[0.2,0.2,0.2]
                                  ,renderMaxTime=0.5
                                  ,threshold3D=0.00001)
analyticBody3.material  = QMParticle(dim=3,hbar=1,m=1)
gaussPacket3            = QMPacketGaussianWave(x0=[6,0,0],t0=0,k0=[3,0,0],a0=[1,1,1])
#gaussPacket3            = QMPacketGaussianWave(dim=3,x0=[0,0,0],t0=0,k0=[3,-2,1],m=1,a0=1,hbar=1)
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


