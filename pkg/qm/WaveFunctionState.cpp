// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include"WaveFunctionState.hpp"
#include<yade/core/Scene.hpp>

YADE_PLUGIN(
	(QuantumMechanicalBody)
	(QuantumMechanicalState)
	(WaveFunctionState)
	(GaussianWavePacket)
	(GaussianAnalyticalPropagatingWavePacket_1D)
	(WaveFunctionGeometry)
	(WavePacketParameters)
	(WaveFunctionInteractionPhysics)
	(WaveFunctionInteractionGeometry)
	(Ip2_WavePacketParameters_WavePacketParameters_WaveFunctionInteractionPhysics)
	(Ip2_Material_WavePacketParameters_WaveFunctionInteractionPhysics)
	(Law2_WaveFunctionInteractionGeometry_WaveFunctionInteractionPhysics_WaveFunctionInteractionPhysics)
	(SchrodingerKosloffPropagator)
	#ifdef YADE_OPENGL
	(Gl1_WaveFunctionGeometry)
//	(Gl1_WaveFunctionInteractionPhysics) // This is for later
	#endif	
	);

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   B O D Y
*
*********************************************************************************/
CREATE_LOGGER(QuantumMechanicalBody);
// !! at least one virtual function in the .cpp file
QuantumMechanicalBody::~QuantumMechanicalBody(){};

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   S T A T E
*
*********************************************************************************/
CREATE_LOGGER(QuantumMechanicalState);
// !! at least one virtual function in the .cpp file
QuantumMechanicalState::~QuantumMechanicalState(){};

/*********************************************************************************
*
* W A V E   F U N C T I O N   S T A T E
*
*********************************************************************************/
CREATE_LOGGER(WaveFunctionState);
// !! at least one virtual function in the .cpp file
WaveFunctionState::~WaveFunctionState(){};

CREATE_LOGGER(GaussianWavePacket);
// !! at least one virtual function in the .cpp file
GaussianWavePacket::~GaussianWavePacket(){};

CREATE_LOGGER(GaussianAnalyticalPropagatingWavePacket_1D);
// !! at least one virtual function in the .cpp file
GaussianAnalyticalPropagatingWavePacket_1D::~GaussianAnalyticalPropagatingWavePacket_1D(){};

std::complex<Real> GaussianAnalyticalPropagatingWavePacket_1D::waveFunctionValue_1D_positionRepresentation
	(Real x,Real x0,Real t,Real k0,Real m, Real a, Real h)
{
	x -= x0;
	return exp(
		-(
			(m*x*x+Mathr::I*a*a*k0*(k0*h*t-2.0*m*x))
			/
			(2.0*a*a*m+2.0*Mathr::I*h*t)
		)
	)
	/
	(
		pow(Mathr::PI,0.25)*(pow(a+Mathr::I*h*t/(a*m),0.5))
	);
};

/*********************************************************************************
*
* W A V E   F U N C T I O N   G E O M E T R Y
*
*********************************************************************************/
CREATE_LOGGER(WaveFunctionGeometry);
// !! at least one virtual function in the .cpp file
WaveFunctionGeometry::~WaveFunctionGeometry(){};

/*********************************************************************************
*
* W A V E   F U N C T I O N   M A T E R I A L
*
*********************************************************************************/
CREATE_LOGGER(WavePacketParameters);
// !! at least one virtual function in the .cpp file
WavePacketParameters::~WavePacketParameters(){};

/*********************************************************************************
*
* W A V E   F U N C T I O N   I N T E R A C T I O N   P H Y S I C S     (constitutive parameters of the contact)
*
*********************************************************************************/
CREATE_LOGGER(WaveFunctionInteractionPhysics);
// !! at least one virtual function in the .cpp file
WaveFunctionInteractionPhysics::~WaveFunctionInteractionPhysics(){};


/*********************************************************************************
*
* W A V E   F U N C T I O N   I N T E R A C T I O N   G E O M E T R Y   (geometrical/spatial parameters of the contact)
*
*********************************************************************************/
CREATE_LOGGER(WaveFunctionInteractionGeometry);
// !! at least one virtual function in the .cpp file
WaveFunctionInteractionGeometry::~WaveFunctionInteractionGeometry(){};


/*********************************************************************************
*
* I N T E R A C T I O N   P H Y S I C S   constitutive parameters of the contact
*
*********************************************************************************/
CREATE_LOGGER(Ip2_WavePacketParameters_WavePacketParameters_WaveFunctionInteractionPhysics);
void Ip2_WavePacketParameters_WavePacketParameters_WaveFunctionInteractionPhysics::go(const shared_ptr<Material>& pp1, const shared_ptr<Material>& pp2, const shared_ptr<Interaction>& interaction){
}

CREATE_LOGGER(Ip2_Material_WavePacketParameters_WaveFunctionInteractionPhysics);
void Ip2_Material_WavePacketParameters_WaveFunctionInteractionPhysics::go(const shared_ptr<Material>& pp1, const shared_ptr<Material>& pp2, const shared_ptr<Interaction>& interaction){
}

/*********************************************************************************
*
* L A W 2   In DEM it was used to calculate Fn and Fs between two interacting bodies
*
*********************************************************************************/
CREATE_LOGGER(Law2_WaveFunctionInteractionGeometry_WaveFunctionInteractionPhysics_WaveFunctionInteractionPhysics);

bool Law2_WaveFunctionInteractionGeometry_WaveFunctionInteractionPhysics_WaveFunctionInteractionPhysics::go(shared_ptr<IGeom>& _geom, shared_ptr<IPhys>& _phys, Interaction* I){
}


/*********************************************************************************
*
* K O S L O F F   P R O P A G A T I O N   o f   S H R O D I N G E R   E Q.
*
*********************************************************************************/
CREATE_LOGGER(SchrodingerKosloffPropagator);
// !! at least one virtual function in the .cpp file
SchrodingerKosloffPropagator::~SchrodingerKosloffPropagator(){};

/*********************************************************************************
*
* W A V E   F U N C T I O N   O P E N   G L   D I S P L A Y
*
*********************************************************************************/
#ifdef YADE_OPENGL
#include<yade/pkg/common/GLDrawFunctors.hpp>
#include<yade/lib/opengl/OpenGLWrapper.hpp>
#include<yade/lib/opengl/GLUtils.hpp>

CREATE_LOGGER(Gl1_WaveFunctionGeometry);
bool Gl1_WaveFunctionGeometry::absolute=true;
bool Gl1_WaveFunctionGeometry::partReal=true;
bool Gl1_WaveFunctionGeometry::partImaginary=true;
bool Gl1_WaveFunctionGeometry::probability=true;
bool Gl1_WaveFunctionGeometry::renderSmoothing=true;
int  Gl1_WaveFunctionGeometry::renderSpecular=10;
int  Gl1_WaveFunctionGeometry::renderAmbient=15;
int  Gl1_WaveFunctionGeometry::renderDiffuse=100;
int  Gl1_WaveFunctionGeometry::renderShininess=50;
Gl1_WaveFunctionGeometry::~Gl1_WaveFunctionGeometry(){};

void Gl1_WaveFunctionGeometry::go(const shared_ptr<Shape>& shape, const shared_ptr<State>& state,bool wire,const GLViewInfo&)
{
	if(not(absolute or partReal or partImaginary or probability)) return; // nothing to draw
	GaussianAnalyticalPropagatingWavePacket_1D* packet=dynamic_cast<GaussianAnalyticalPropagatingWavePacket_1D*>(state.get());
	Vector3r col = shape->color;
/*
	if(partReal) {
		glBegin(GL_LINE_STRIP);
		glColor3v(   Vector3r(col.cwiseProduct(Vector3r(0.4,0.4,1.0))) ); // display partReal part in bluish color
		for(Real x=-4.0 ; x<4.0 ; x+=0.01 )
		{
			std::complex<Real> wfval = packet->waveFunctionValue_1D_positionRepresentation(x,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
			glVertex3d(x,std::real(wfval),0);
		}
		glEnd();
	}

	if(imag) {
		glBegin(GL_LINE_STRIP);
		glColor3v(   Vector3r(col.cwiseProduct(Vector3r(1.0,0.4,0.4))) ); // display imag part in reddish color
		for(Real x=-4.0 ; x<4.0 ; x+=0.01 )
		{
			std::complex<Real> wfval = packet->waveFunctionValue_1D_positionRepresentation(x,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
			glVertex3d(x,std::imag(wfval),0);
		}
		glEnd();
	}

	if(abs) {
		glBegin(GL_LINE_STRIP);
		glColor3v(shape->color); // display abs in intended shape color
		for(Real x=-4.0 ; x<4.0 ; x+=0.01 )
		{
			std::complex<Real> wfval = packet->waveFunctionValue_1D_positionRepresentation(x,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
			glVertex3d(x,std::abs(wfval),0);
		}
		glEnd();
	}
*/

	Real step=0.1;
	Real startX= -4.0;
	Real endX  =  4.0;
	Real startY= -4.0;
	Real endY  =  4.0;
	std::vector<std::vector<Real> > waveVals;
	waveVals.resize(int((endX-startX)/step)+1);
	FOREACH(std::vector<Real>& yy, waveVals) {yy.resize(int((endX-startX)/step)+1,0);};

	if(partReal) { // FIXME - merge those four into one loop
		int i=0;
		for(Real x=startX ; x<=endX ; x+=step,i++ )
		{
			int j=0;
			for(Real y=startY ; y<=endY ; y+=step,j++ )
			{
				std::complex<Real> wfvalx0 = packet->waveFunctionValue_1D_positionRepresentation(x     ,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				std::complex<Real> wfvaly0 = packet->waveFunctionValue_1D_positionRepresentation(y     ,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				waveVals[i][j]=std::real(wfvalx0*wfvaly0);
			}
		}
		drawSurface(waveVals,shape,Vector3r(col.cwiseProduct(Vector3r(0.4,0.4,1.0)))); // display partReal part in bluish color
	}
	if(partImaginary) {
		int i=0;
		for(Real x=startX ; x<=endX ; x+=step,i++ )
		{
			int j=0;
			for(Real y=startY ; y<=endY ; y+=step,j++ )
			{
				std::complex<Real> wfvalx0 = packet->waveFunctionValue_1D_positionRepresentation(x     ,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				std::complex<Real> wfvaly0 = packet->waveFunctionValue_1D_positionRepresentation(y     ,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				waveVals[i][j]=std::imag(wfvalx0*wfvaly0);
			}
		}
		drawSurface(waveVals,shape,Vector3r(col.cwiseProduct(Vector3r(1.0,0.4,0.4)))); // display imag part in reddish color
	}
	if(absolute) {
		int i=0;
		for(Real x=startX ; x<=endX ; x+=step,i++ )
		{
			int j=0;
			for(Real y=startY ; y<=endY ; y+=step,j++ )
			{
				std::complex<Real> wfvalx0 = packet->waveFunctionValue_1D_positionRepresentation(x     ,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				std::complex<Real> wfvaly0 = packet->waveFunctionValue_1D_positionRepresentation(y     ,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				waveVals[i][j]=std::abs(wfvalx0*wfvaly0);
			}
		}
		drawSurface(waveVals,shape,Vector3r(col.cwiseProduct(Vector3r(0.4,1.0,0.4)))); // display abs value in geenish color
	}
	if(probability) {
		int i=0;
		for(Real x=startX ; x<=endX ; x+=step,i++ )
		{
			int j=0;
			for(Real y=startY ; y<=endY ; y+=step,j++ )
			{
				std::complex<Real> wfvalx0 = packet->waveFunctionValue_1D_positionRepresentation(x     ,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				std::complex<Real> wfvaly0 = packet->waveFunctionValue_1D_positionRepresentation(y     ,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				waveVals[i][j]=std::real( (wfvalx0*wfvaly0)*std::conj(wfvalx0*wfvaly0) );
			}
		}
		drawSurface(waveVals,shape,col);
	}

/* FIXME: add parameters, auto or specified by hand startX,endX (Y,Z), dX,dY,dZ
	if(partReal) {
		glColor3v(shape->color); // display abs in intended shape color
		for(Real x=-4.0 ; x<4.0 ; x+=0.1 )
		{
			glBegin(GL_LINE_STRIP);
			for(Real y=-4.0 ; y<4.0 ; y+=0.01 )
			{
				std::complex<Real> wfvalx0 = packet->waveFunctionValue_1D_positionRepresentation(x,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				std::complex<Real> wfvaly0 = packet->waveFunctionValue_1D_positionRepresentation(y,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				glVertex3d(x    ,std::real(wfvalx0*wfvaly0),y);
			}
			glEnd();
		}
		for(Real y=-4.0 ; y<4.0 ; y+=0.1 )
		{
			glBegin(GL_LINE_STRIP);
			for(Real x=-4.0 ; x<4.0 ; x+=0.01 )
			{
				std::complex<Real> wfvalx0 = packet->waveFunctionValue_1D_positionRepresentation(x,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				std::complex<Real> wfvaly0 = packet->waveFunctionValue_1D_positionRepresentation(y,packet->x0,0,packet->k0,packet->m,packet->a,packet->hbar);
				glVertex3d(x    ,std::real(wfvalx0*wfvaly0),y);
			}
			glEnd();
		}
	}
*/
};

void Gl1_WaveFunctionGeometry::drawSurface(const std::vector<std::vector<Real> >& waveVals,const shared_ptr<Shape>& shape,Vector3r col)
{
	//FIXME - get randes from AABB or if not present - let user set them, and use some default.
	Real step=0.1;
	Real startX= -4.0;
	Real endX  =  4.0;
	Real startY= -4.0;
	Real endY  =  4.0;
	// Now I have waveVals[i][j] filled with values to plot. So calculate all normals. First normals for triangles
	//      *        1                                                     
	//    / |1\    / | \     wavNormV[i][j] stores normal for vertex[i][j] averaged from all 4 neigbour normals (except for edges)
	//   *--x--*  4--0--2
	//    \ | /    \ | /
	//      *        3    
	std::vector<std::vector<Vector3r> > wavNormV;
	wavNormV.resize(int((endX-startX)/step)+1);
	FOREACH(std::vector<Vector3r>& yy, wavNormV) {yy.resize(int((endX-startX)/step)+1,Vector3r(0,0,0));};
	int lenX=wavNormV.size();
	int lenY=wavNormV[0].size();
	Vector3r p0(0,0,0),p1(0,0,0),p2(0,0,0),p3(0,0,0),p4(0,0,0);
	Vector3r           n1(0,0,0),n2(0,0,0),n3(0,0,0),n4(0,0,0);
	int i=0;
	for(Real x=startX ; x<=endX ; x+=step,i++ )
	{
		int j=0;
		for(Real y=startY ; y<=endY ; y+=step,j++ )
		{
			                p0=Vector3r(x     ,y     ,waveVals[i  ][j  ]);
			if((j+1)<lenY){ p1=Vector3r(x     ,y+step,waveVals[i  ][j+1]);} else{ p1=p0;};
			if((i+1)<lenX){ p2=Vector3r(x+step,y     ,waveVals[i+1][j  ]);} else{ p2=p0;};
			if((j-1)>=0  ){ p3=Vector3r(x     ,y-step,waveVals[i  ][j-1]);} else{ p3=p0;};
			if((i-1)>=0  ){ p4=Vector3r(x-step,y     ,waveVals[i-1][j  ]);} else{ p4=p0;};
			n1 = (p2-p0).cross(p1-p0);
			n2 = (p3-p0).cross(p2-p0);
			n3 = (p4-p0).cross(p3-p0);
			n4 = (p1-p0).cross(p4-p0);
			int count=0;
			if(n1.squaredNorm()>0) { n1.normalize(); count++;};
			if(n2.squaredNorm()>0) { n2.normalize(); count++;};
			if(n3.squaredNorm()>0) { n3.normalize(); count++;};
			if(n4.squaredNorm()>0) { n4.normalize(); count++;};
			wavNormV[i][j] = Vector3r(n1+n2+n3+n4)/(1.0*count);
		}
	}

	// now draw surface
	GLfloat mat[4];
	mat[0] = renderSpecular/100.0;
	mat[1] = renderSpecular/100.0;
	mat[2] = renderSpecular/100.0;
	mat[3] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat);
	mat[0] = renderAmbient/100.0;
	mat[1] = renderAmbient/100.0;
	mat[2] = renderAmbient/100.0;
	mat[3] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat);
	mat[0] = renderDiffuse/100.0;
	mat[1] = renderDiffuse/100.0;
	mat[2] = renderDiffuse/100.0;
	mat[3] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat);
	GLfloat sh=renderShininess;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS,&sh);

	glEnable(GL_CULL_FACE);
	if(renderSmoothing){ glShadeModel(GL_SMOOTH);};
	//draw front
	glCullFace(GL_BACK);
	glColor3v(col);
	for(int i=0 ; i<lenX-1 ; i++ )
	{
		Real x=startX+i*step;
		glBegin(GL_TRIANGLE_STRIP);
		for(int j=0 ; j<lenY ; j++ )
		{
			Real y=startY+j*step;
			glNormal3v(         wavNormV[i  ][j]);
			glVertex3f(x     ,y,waveVals[i  ][j]);
			glNormal3v(         wavNormV[i+1][j]);
			glVertex3f(x+step,y,waveVals[i+1][j]);
		}
		glEnd();
	}
	//draw back
	glCullFace(GL_BACK);
	glColor3v(Vector3r(col.cwiseProduct(Vector3r(0.5,0.5,0.5))));
	for(int i=0 ; i<lenX-1 ; i++ )
	{
		Real x=startX+i*step;
		glBegin(GL_TRIANGLE_STRIP);
		for(int j=lenY-1 ; j>=0 ; j-- )
		{
			Real y=startY+j*step;
			glNormal3v(         wavNormV[i  ][j]);
			glVertex3f(x     ,y,waveVals[i  ][j]);
			glNormal3v(         wavNormV[i+1][j]);
			glVertex3f(x+step,y,waveVals[i+1][j]);
		}
		glEnd();
	}
//	glEnable(GL_CULL_FACE);
	glShadeModel(GL_FLAT);
}

// This will come later, when I will have some interactions going on....
//	CREATE_LOGGER(Gl1_WaveFunctionInteractionPhysics);
//	bool Gl1_WaveFunctionInteractionPhysics::abs=true;
//	bool Gl1_WaveFunctionInteractionPhysics::real=false;
//	bool Gl1_WaveFunctionInteractionPhysics::imag=false;
//	Gl1_WaveFunctionInteractionPhysics::~Gl1_WaveFunctionInteractionPhysics(){};
//	void Gl1_WaveFunctionInteractionPhysics::go(const shared_ptr<IPhys>& ip, const shared_ptr<Interaction>& i, const shared_ptr<Body>& b1, const shared_ptr<Body>& b2, bool wireFrame)
//	{
//	}

#endif

