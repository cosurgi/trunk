#!/usr/bin/python
# -*- coding: utf-8 -*-

dimensions= 2
#size1d   = 20
#size1d   = 20 #80
size1d   = 70
halfSize1 = [size1d,size1d,0.1]
halfSize2 = halfSize1
#GRIDSIZE  = 16
#GRIDSIZE  = [32,32] #3*64 #2**7
GRIDSIZE  = [70,70] #3*64 #2**7

# hydrogen parameters
#SH0= -size1d+(2.0*size1d/GRIDSIZE[0])*(1.0*GRIDSIZE[0]/2)+(1.0*size1d/GRIDSIZE[0])
#SH1= -size1d+(2.0*size1d/GRIDSIZE[1])*(1.0*GRIDSIZE[1]/2)+(1.0*size1d/GRIDSIZE[1])
#potentialCenter      = [ SH0,SH1 ,0  ]
potentialCenter      = [ 0 ,0  ,0  ]
potentialHalfSize    = Vector3(size1d,size1d,3)
potentialMaximum     = 10000;
hydrogenEigenFunc_n  = 2
hydrogenEigenFunc_l  = 1

# wavepacket parameters
k0_x         = 0
k0_y         = 0
gaussWidth_x = 1.0
gaussWidth_y = 1.0
potentialCoefficient1= [-1.0,0,0]
potentialCoefficient2= [ 1.0,0,0]


O.engines=[
	StateDispatcher([
		St1_QMPacketGaussianWave(),
		St1_QMPacketHydrogenEigenFunc(),
	]),
	SpatialQuickSortCollider([
		Bo1_Box_Aabb(),
	]),
	InteractionLoop(
		[Ig2_2xQMGeometry_QMIGeom()],
		[Ip2_QMParticleCoulomb_QMParametersCoulomb_QMIPhysCoulombParticles()],
		[Law2_QMIGeom_QMIPhysCoulombParticlesFree()]
	),
#	SchrodingerKosloffPropagator(FIXMEatomowe_MASS=2),
	SchrodingerAnalyticPropagator(),
]

scaleAll=300
separate_r_R=True
drawFFT=False
fftPOS = size1d if drawFFT else 0.0

def append_dict(d,arg):
    import copy
    ret = copy.deepcopy(d)
    ret.update(arg)
    return ret

displayOptions         = { 'partsScale':scaleAll,'partsSquared':0
                          ,'partAbsolute':['default surface', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partImaginary':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'partReal':['default hidden', 'hidden', 'nodes', 'points', 'wire', 'surface']
                          ,'renderMaxTime':0.5
                          , 'renderRotated45':separate_r_R
                          }

displayOptions1_num    = append_dict(displayOptions,{'renderSe3':(Vector3(size1d*2.4,0,0), Quaternion((1,0,0),0))})
displayOptions2_num    =             displayOptions
displayOptions1_an     = append_dict(displayOptions,{'renderSe3':(Vector3(size1d*2.4,size1d*2.4,0), Quaternion((1,0,0),0))})
displayOptions2_an     = append_dict(displayOptions,{'renderSe3':(Vector3(0,size1d*2.4,0), Quaternion((1,0,0),0))})

#body0.material  = QMParametersCoulomb(dim=dimensions,hbar=1#,m=1
#                                      ,coefficient=potentialCoefficient1,potentialMaximum=potentialMaximum)
# FFTW is best at handling sizes of the form 2ᵃ 3ᵇ 5ᶜ 7ᵈ 11ᵉ 13ᶠ , where e+f is either 0 or 1  ## http://www.nanophys.kth.se/nanophys/fftw-info/fftw_3.html

## FIXME - przetestować w mathematica z różnie przesuniętymi względem siebie siatkami i różnym rozstawem siatek.
##         głównie chodzi o to, żeby węzły siatek się nie nakrywały.

########## Analytic solutions
body0           = QMBody()
body0.groupMask = 2
body0.shape     = QMGeometry(extents=halfSize1,color=[0.5,0.5,0.5],displayOptions=[QMDisplayOptions(**displayOptions1_an)])
body0.material  = QMParticleCoulomb(dim=dimensions,hbar=1,m=4,coefficient=potentialCoefficient1) # m=2 FIXMEatomowe
body0.state     = QMPacketGaussianWave(x0=potentialCenter,t0=0,k0=[k0_x,k0_y,0],a0=[gaussWidth_x,gaussWidth_y,0],gridSize=GRIDSIZE) #,se3=[[0.5,0.5,0.5],Quaternion((1,0,0),0)])
nid=O.bodies.append(body0)
O.bodies[nid].state.setAnalytic()

body1           = QMBody()
body1.groupMask = 2
body1.shape     = QMGeometry(extents=halfSize2,color=[0.6,0.6,0.6],displayOptions=[QMDisplayOptions(**displayOptions2_an)])
body1.material  = QMParametersCoulomb(dim=dimensions,hbar=1#,m=2 # FIXMEatomowe
                                      ,coefficient=potentialCoefficient2,potentialMaximum=potentialMaximum)
coulombPacketArg      = {'energyLevel':[hydrogenEigenFunc_n,hydrogenEigenFunc_l,0],'x0':potentialCenter,'gridSize':GRIDSIZE}
body1.state     = QMPacketHydrogenEigenFunc(**coulombPacketArg)
nid=O.bodies.append(body1)
O.bodies[nid].state.setAnalytic()


########## Numeric solutions
# body2           = QMBody()
# body2.groupMask = 1
# body2.shape     = QMGeometry(extents=halfSize1,color=[0.9,0.9,0.9],displayOptions=[QMDisplayOptions(**displayOptions1_num)])
# body2.material  = QMParticleCoulomb(dim=dimensions,hbar=1,m=4,coefficient=potentialCoefficient1) # m=2 FIXMEatomowe
# body2.state     = QMPacketGaussianWave(x0=potentialCenter,t0=0,k0=[k0_x,k0_y,0],a0=[gaussWidth_x,gaussWidth_y,0],gridSize=GRIDSIZE) #,se3=[[0.5,0.5,0.5],Quaternion((1,0,0),0)])
# nid=O.bodies.append(body2)
# O.bodies[nid].state.setNumeric()
# 
# body3           = QMBody()
# body3.groupMask = 1
# body3.shape     = QMGeometry(extents=halfSize2,color=[1,1,1],displayOptions=[QMDisplayOptions(**displayOptions2_num)])
# body3.material  = QMParametersCoulomb(dim=dimensions,hbar=1#,m=2 # m=2 FIXMEatomowe
#                                       ,coefficient=potentialCoefficient2,potentialMaximum=potentialMaximum)
# coulombPacketArg      = {'energyLevel':[hydrogenEigenFunc_n,hydrogenEigenFunc_l,0],'x0':potentialCenter,'gridSize':GRIDSIZE}
# body3.state     = QMPacketHydrogenEigenFunc(**coulombPacketArg)
# nid=O.bodies.append(body3)
# O.bodies[nid].state.setNumeric()

O.dt=.25

O.save('/tmp/a.xml.bz2');
#o.run(100000); o.wait(); print o.iter/o.realtime,'iterations/sec'

try:
	from yade import qt
	qt.Controller()
	qt.controller.setWindowTitle("Electron-positron pair in 2D")
	qt.Renderer().blinkHighlight=False
	qt.View()
	qt.controller.setViewAxes(dir=(0,1,0),up=(0,0,1))
	qt.views()[0].center(False,60) # median=False, suggestedRadius = 5

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
