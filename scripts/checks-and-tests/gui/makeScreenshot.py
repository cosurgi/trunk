#!/usr/bin/python
# -*- coding: utf-8 -*-
##########################################################################
#  2019        Janek Kozicki                                             #
#                                                                        #
#  This program is free software; it is licensed under the terms of the  #
#  GNU General Public License v2 or later. See file LICENSE for details. #
##########################################################################

import subprocess,time
from yade import qt

class MakeScreenshot:
	"""
	This simple class makes screenshots.
	"""
	def __init__(self,name=None):
		self.scrNum=0;
		if(name != None):
			self.name=name;
		else:
			self.name="";

	def makeNextScreenshot():
		time.sleep(1)
		subprocess.run(["scrot", "-z" , "scr"+str(self.scrNum).zfill(2)+name+".png"])
		time.sleep(1)

	def screenshotEngine():
		self.scrNum += 1
		intro = "="*20+" "+"stage:"+str(self.scrNum)+" iter:"+str(O.iter)
		if(self.scrNum == 0):
			makeNextScreenshot();
			print(intro)
			return
		if(self.scrNum == 1):
			makeNextScreenshot();
			print(intro+" opening yade.qt.View()")
			yade.qt.View();
			vv=yade.qt.views()[0]
			vv.axes=True
			vv.eyePosition=(2.8717367257539266,-3.2498802823394684,11.986065750108025)
			vv.upVector  =(-0.786154130840315,-0.2651054185084529,0.558283798475441)
			vv.center()
			return
		if(self.scrNum == 2):
			makeNextScreenshot();
			print(intro+" opening yade.qt.Inspector()")
			yade.qt.Inspector()
			yade.qt.controller.inspector.setGeometry(1050,20,500,1100)
			yade.qt.controller.inspector.show()
			return
		if(self.scrNum == 3):
			makeNextScreenshot();
			print(intro+" changing tab to bodies")
			yade.qt.controller.inspector.close()
			yade.qt.controller.inspector.setGeometry(1050,20,500,1100)
			yade.qt.controller.inspector.tabWidget.setCurrentIndex(1)
			yade.qt.controller.inspector.show()
			return
		if(self.scrNum == 4):
			makeNextScreenshot();
			print(intro+" changing tab to interactions")
			yade.qt.controller.inspector.close()
			yade.qt.controller.inspector.setGeometry(1050,20,500,1100)
			yade.qt.controller.inspector.tabWidget.setCurrentIndex(2)
			yade.qt.controller.inspector.show()
			return
		if(self.scrNum == 5):
			makeNextScreenshot();
			print(intro+" changing tab to cell")
			yade.qt.controller.inspector.close()
			yade.qt.controller.inspector.setGeometry(1050,20,500,1100)
			yade.qt.controller.inspector.tabWidget.setCurrentIndex(3)
			yade.qt.controller.inspector.show()
			return
		if(self.scrNum == 6):
			makeNextScreenshot();
			print(intro+" changing tab to bodies")
			yade.qt.controller.inspector.close()
			yade.qt.controller.inspector.setGeometry(1050,20,500,1100)
			yade.qt.controller.inspector.tabWidget.setCurrentIndex(1)
			yade.qt.controller.inspector.show()
			print(intro+" moving yade.qt.Controller()")
			yade.qt.Controller()
			yade.qt.controller.setGeometry(550,20,500,1100)
			return
		if(self.scrNum == 7):
			makeNextScreenshot();
			print(intro+" changing tab to display")
			yade.qt.controller.setTabActive('display')
			return
		if(self.scrNum == 8):
			makeNextScreenshot();
			print(intro+" changing tab to generator")
			yade.qt.controller.setTabActive('generator')
			return
		if(self.scrNum == 9):
			makeNextScreenshot();
			print(intro+" changing tab to python")
			yade.qt.controller.setTabActive('python')
			return
		if(self.scrNum == 10):
			makeNextScreenshot();
			print(intro+" changing tab to simulation")
			yade.qt.controller.setTabActive('simulation')
			return
		if(self.scrNum == 11):
			makeNextScreenshot();
			# FIXME: I couldn't get matplotlib to draw the plot, while screenshotting is going on.
			# makeNextScreenshot();
			# O.pause()
			# print(intro+" opening yade.plot.plot()")
			# plot.liveInterval=0
			# plot.plot(subPlots=False)
			# fig=yade.plot.plot();
			# time.sleep(5)
			# matplotlib.pyplot.draw()
			# time.sleep(5)
			# makeNextScreenshot();
			# matplotlib.pyplot.close(fig)
			return
		if(self.scrNum == 12):
			# FIXME: this number '12' appears in three places:
			# * here
			# * in function makeNextScreenshot
			# * bash script scripts/checks-and-tests/testGui.sh
			# if more stages are added to this test, it has to be updated in three places.
			makeNextScreenshot();
			print(intro+" exiting")
			O.pause()
			vv=yade.qt.views()[0]
			vv.close()
			yade.qt.controller.inspector.close()
			yade.qt.controller.close()
			yade.runtime.opts.exitAfter=True
#			sys.exit(0)
			os._exit(0)
#			ip = get_ipython()
#			ip.ask_exit()
#			ip.pt_cli.exit()
#			quit()
			return

