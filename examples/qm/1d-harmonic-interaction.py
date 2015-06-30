#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 1
size1d   = 60
halfSize1 = [size1d,0.2,0.1]
halfSize2 = halfSize1
GRIDSIZE  = 128

# wavepacket parameters
k0_x         = 0
k0_y         = 0
gaussWidth_x = 1.0
gaussWidth_y = 0.0
potentialCoefficient= [2.0,0.5,0.5]


O.engines=[
	StateDispatcher([
		St1_QMPacketGaussianWave(),
	]),
	SpatialQuickSortCollider([
		Bo1_Box_Aabb(),
	]),
	InteractionLoop(
		[Ig2_2xQMGeometry_QMIGeom()],
		[Ip2_QMParticleHarmonic_QMParticleHarmonic_QMIPhysHarmonicParticles()],
		[Law2_QMIGeom_QMIPhysHarmonicParticles()]
	),
	SchrodingerKosloffPropagator(),
]

displayEnt= { 'partAbsolute':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
             ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
             ,'partReal':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
             ,'stepRender':["default hidden","hidden","frame","stripes","mesh"]
            }

body0           = QMBody()
body0.shape     = QMGeometry(extents=halfSize1,color=[0.8,0.8,0.8],displayOptions=[
     QMDisplayOptions(partsScale=10,partsSquared=False,renderWireLight=False  ,renderSe3=(Vector3(-size1d*1.2,       0,0), Quaternion((1,0,0),0)) )
    ,QMDisplayOptions(partsScale=10,partsSquared=False,renderWireLight=False  ,renderSe3=(Vector3(-size1d*1.2, size1d,0), Quaternion((1,0,0),0)) )
    ,QMDisplayOptions(partsScale=10,partsSquared=False,renderWireLight=False  ,renderSe3=(Vector3(-size1d*1.2,-size1d,0), Quaternion((1,0,0),0)) )
    ,QMDisplayOptions(partsScale=10,partsSquared=True,renderWireLight=True   ,renderSe3=(Vector3(-size1d*1.2,       0,0), Quaternion((1,0,0),0))
        ,doMarginalDistribution=[(0,0,0),(0,0,0)],**displayEnt)

    ,QMDisplayOptions(partsScale=10,partsSquared=False,renderWireLight=False,renderFFT=True  ,renderSe3=(Vector3(size1d*1.2,       0,0), Quaternion((1,0,0),0)) )
    ,QMDisplayOptions(partsScale=10,partsSquared=False,renderWireLight=False,renderFFT=True  ,renderSe3=(Vector3(size1d*1.2, size1d,0), Quaternion((1,0,0),0)) )
    ,QMDisplayOptions(partsScale=10,partsSquared=False,renderWireLight=False,renderFFT=True  ,renderSe3=(Vector3(size1d*1.2,-size1d,0), Quaternion((1,0,0),0)) )
    ,QMDisplayOptions(partsScale=10,partsSquared=True,renderWireLight=True ,renderFFT=True  ,renderSe3=(Vector3(size1d*1.2,       0,0), Quaternion((1,0,0),0))
        , doMarginalDistribution=[(0,0,0),(0,0,0)],**displayEnt)
    #                  ,renderSe3=(Vector3(0,0,-4), Quaternion((1,0,0),0))
    #                  ,renderFFTScale=(4,1,1)
])
body0.material  = QMParticleHarmonic(dim=dimensions,hbar=1,m=1,coefficient=potentialCoefficient)
# FFTW is best at handling sizes of the form 2ᵃ 3ᵇ 5ᶜ 7ᵈ 11ᵉ 13ᶠ , where e+f is either 0 or 1  ## http://www.nanophys.kth.se/nanophys/fftw-info/fftw_3.html
body0.state     = QMPacketGaussianWave(x0=[-1,0,0],t0=0,k0=[k0_x,k0_y,0],a0=[gaussWidth_x,gaussWidth_y,0],gridSize=[GRIDSIZE]) #,se3=[[0.5,0.5,0.5],Quaternion((1,0,0),0)])
nid=O.bodies.append(body0)
O.bodies[nid].state.setNumeric()

body1           = QMBody()
body1.shape     = QMGeometry(extents=halfSize2,color=[1,1,1],displayOptions=[
     QMDisplayOptions(partsScale=10,partsSquared=False,renderWireLight=False  ,renderSe3=(Vector3(-size1d*1.2        ,0,0), Quaternion((0,0,1),0   )) )
    ,QMDisplayOptions(partsScale=10,partsSquared=False,renderWireLight=False  ,renderSe3=(Vector3(-size1d*1.2+size1d,0,0), Quaternion((0,0,1),pi/2)) )
    ,QMDisplayOptions(partsScale=10,partsSquared=False,renderWireLight=False  ,renderSe3=(Vector3(-size1d*1.2-size1d,0,0), Quaternion((0,0,1),pi/2)) )

    ,QMDisplayOptions(partsScale=10,partsSquared=False,renderWireLight=False,renderFFT=True    ,renderSe3=(Vector3(size1d*1.2        ,0,0), Quaternion((1,0,0),0   )) )
    ,QMDisplayOptions(partsScale=10,partsSquared=False,renderWireLight=False,renderFFT=True    ,renderSe3=(Vector3(size1d*1.2+size1d,0,0), Quaternion((0,0,1),pi/2)) )
    ,QMDisplayOptions(partsScale=10,partsSquared=False,renderWireLight=False,renderFFT=True    ,renderSe3=(Vector3(size1d*1.2-size1d,0,0), Quaternion((0,0,1),pi/2)) )
])
body1.material  = QMParticleHarmonic(dim=dimensions,hbar=1,m=1,coefficient=potentialCoefficient)

## FIXME - przetestować w mathematica z różnie przesuniętymi względem siebie siatkami i różnym rozstawem siatek.
##         głównie chodzi o to, żeby węzły siatek się nie nakrywały.


body1.state     = QMPacketGaussianWave(x0=[1,0,0],t0=0,k0=[-k0_x,-k0_y,0],a0=[gaussWidth_x,gaussWidth_y,0],gridSize=[GRIDSIZE]) #,se3=[[0.5,0.5,0.5],Quaternion((1,0,0),0)])
nid=O.bodies.append(body1)
O.bodies[nid].state.setNumeric()

O.dt=.05

O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

try:
	from yade import qt
	qt.Controller()
	qt.controller.setWindowTitle("Two Gaussian packets connected via harmonic potential in 2D")
	qt.Renderer().blinkHighlight=False
	qt.View()
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.views()[0].center(False,10) # median=False, suggestedRadius = 5

except ImportError:
	pass
#O.run(20000)


#### save result for comparison with mathematica
#
# nn=O.bodies[0].state
# ff=open("1d-harmonic-interaction-yade.txt",'w')
# for i in range(nn.gridSize[0]):
#     ff.write(str(nn.iToX(i,0))+" "+str(nn.iToX(96,1))+" "+str((nn.atPsiGlobal([i,96])).real)+" "+str((nn.atPsiGlobal([i,96])).imag)+"\n")
# ff.close()
#Yade [3]: def zapisz(nazwa,x2):
#     ...:     nn=O.bodies[0].state
#     ...:     ff=open(nazwa,'w')
#     ...:     for i in range(nn.gridSize[0]):
#     ...:         ff.write(str(nn.iToX(i,0))+" "+str(nn.iToX(x2,1))+" "+str((nn.atPsiGlobal([i,x2])).real)+" "+str((nn.atPsiGlobal([i,x2])).imag)+"\n")
#     ...:     ff.close()
#     ...:     


#zapisz("1d-harmonic-interaction-yade_0.05_506.txt",506)
#zapisz("1d-harmonic-interaction-yade_0.05_507.txt",507)
#zapisz("1d-harmonic-interaction-yade_0.05_508.txt",508)
#zapisz("1d-harmonic-interaction-yade_0.05_509.txt",509)
#zapisz("1d-harmonic-interaction-yade_0.05_510.txt",510)
#zapisz("1d-harmonic-interaction-yade_0.05_511.txt",511)
#zapisz("1d-harmonic-interaction-yade_0.05_512.txt",512)
#zapisz("1d-harmonic-interaction-yade_0.05_513.txt",513)
#zapisz("1d-harmonic-interaction-yade_0.05_514.txt",514)
#zapisz("1d-harmonic-interaction-yade_0.05_515.txt",515)
#zapisz("1d-harmonic-interaction-yade_0.05_516.txt",516)
#zapisz("1d-harmonic-interaction-yade_0.05_517.txt",517)
#zapisz("1d-harmonic-interaction-yade_0.05_518.txt",518)

#zapisz("1d-harmonic-interaction-yade_0.1_506.txt",506)
#zapisz("1d-harmonic-interaction-yade_0.1_507.txt",507)
#zapisz("1d-harmonic-interaction-yade_0.1_508.txt",508)
#zapisz("1d-harmonic-interaction-yade_0.1_509.txt",509)
#zapisz("1d-harmonic-interaction-yade_0.1_510.txt",510)
#zapisz("1d-harmonic-interaction-yade_0.1_511.txt",511)
#zapisz("1d-harmonic-interaction-yade_0.1_512.txt",512)
#zapisz("1d-harmonic-interaction-yade_0.1_513.txt",513)
#zapisz("1d-harmonic-interaction-yade_0.1_514.txt",514)
#zapisz("1d-harmonic-interaction-yade_0.1_515.txt",515)
#zapisz("1d-harmonic-interaction-yade_0.1_516.txt",516)
#zapisz("1d-harmonic-interaction-yade_0.1_517.txt",517)
#zapisz("1d-harmonic-interaction-yade_0.1_518.txt",518)

# gnuplot 
#N="./1d-harmonic-interaction-yade_0."
#M="./1d-harmonic-interaction-mathematica_0."
#set grid
#set xtics 1
#t=512
#plot N."05_".sprintf("%.3i",t).".txt" u 1:3 w l lt 1,N."05_".sprintf("%.3i",t).".txt" u 1:4 w l lt 2,M."05_".sprintf("%.3i",t).".txt" u 1:3 w l lt 3,M."05_".sprintf("%.3i",t).".txt" u 1:4 w l lt 4 title sprintf("%.3i",t)
#plot N."1_".sprintf("%.3i",t).".txt" u 1:3 w l lt 1,N."1_".sprintf("%.3i",t).".txt" u 1:4 w l lt 2,M."1_".sprintf("%.3i",t).".txt" u 1:3 w l lt 3,M."1_".sprintf("%.3i",t).".txt" u 1:4 w l lt 4 title sprintf("%.3i",t)
#


