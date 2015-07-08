// 2014 © Janek Kozicki <cosurgi@gmail.com>

/*********************************************************************************
*
* W A V E   F U N C T I O N   O P E N   G L   D I S P L A Y
*
*********************************************************************************/

#ifdef YADE_OPENGL
#include "Gl1_QMGeometry.hpp"
#include <core/Scene.hpp>

YADE_PLUGIN(
	(Gl1_QMGeometry)
	(Gl1_NDimTable)
	);

#include "QMBody.hpp"
#include "QMState.hpp"
#include "QMStateAnalytic.hpp"
#include "QMStateDiscrete.hpp"
#include "QMStateDiscreteGlobal.hpp"
#include <pkg/common/GLDrawFunctors.hpp>
#include <lib/opengl/OpenGLWrapper.hpp>
#include <lib/opengl/GLUtils.hpp>
#include <lib/smoothing/Spline6Interpolate.hpp>
#include <boost/lexical_cast.hpp>

/*********************************************************************************
*
* Q M   G E O M E T R Y    O P E N   G L   D I S P L A Y
*
*********************************************************************************/

Vector3r Gl1_QMGeometry::lastDiscreteStep            = Vector3r(-1,-1,-1);
Real     Gl1_QMGeometry::lastDiscreteScale           = -1;
bool     Gl1_QMGeometry::analyticUsesStepOfDiscrete  = true; // FIXME - probably should be moved into QMDisplayOptions, but when I will have more particles
bool     Gl1_QMGeometry::analyticUsesScaleOfDiscrete = true; //         I will need to pick which discrete step to use! Because each discrete particle
                                                             //         can have different step!

CREATE_LOGGER(Gl1_QMGeometry);
Gl1_QMGeometry::~Gl1_QMGeometry(){};

void firstRunOnly(Scene* scene) {
	static bool first(true);
	if(first) {
		first=false;
		shared_ptr<StateDispatcher> st;
		FOREACH(shared_ptr<Engine>& e, scene->engines){ st=YADE_PTR_DYN_CAST<StateDispatcher>(e); if(st) break; }
		if(!st) {
			std::cerr << "Gl1_* : StateDispatcher is missing. It is needed before first draw, to setSpatialSize from Box::extents.\n";
			exit(1);
		};
		st->action();
	}
};

void Gl1_QMGeometry::go(
	const shared_ptr<Shape>& shape, 
	const shared_ptr<State>& state,
	bool forceWire,
	const GLViewInfo& glinfo
)
{
	firstRunOnly(scene); // we want to draw before simulation starts. So QMState must learn about its spatial size Box::extents here.
	timeLimit.readWallClock();
	QMGeometry*        qmg = static_cast<QMGeometry*>(shape.get());
	QMStateDiscrete*   qms  = static_cast<QMStateDiscrete*>(state.get());

/* 3 */// static int printed=false;
/* 3 */// if(true /*not printed*/) {
/* 3 */// 	std::cout << "\n-3------------------------------\n";
/* 3 */// 	qms->getPsiGlobalExisting()->psiGlobalTable.print();
/* 3 */// 	std::cout << "\n-3------------------------------\n";
/* 3 */// 	printed=true;
/* 3 */// }

	// ↓ patrz też uwagi w QMState.hpp
	size_t dimSpatial = qms->gridSize.size(); // originally generated particle can be max 3D
	if(dimSpatial == 0 or dimSpatial>3 ) { std::cerr << "\n\nERROR: Gl1_QMGeometry::go cannot draw when gridSize.size() == 0 or gridSize.size() > 3\n\n";return; };

	for(auto& current_opt : qmg->displayOptions) {
		shared_ptr<QMDisplayOptions> curOpt=YADE_PTR_DYN_CAST<QMDisplayOptions>(current_opt);
		if(not curOpt){ std::cerr << "\n\nERROR, no QMDisplayOptions found, but"<< curOpt->getClassName() <<"\n\n"; continue; };
		if(     menuSelection(curOpt->partAbsolute )=="hidden" and menuSelection(curOpt->partReal     )=="hidden" and menuSelection(curOpt->partImaginary)=="hidden" and menuSelection(curOpt->stepRender   )=="hidden")
			continue; // nothing to draw

		// 1. MARGINAL DISTRIBUTIONS - make sure they are defined, because they determine which steps (along which dimensions) we use the `2. STEP`
		std::size_t rank = qms->getPsiGlobalExisting()->dim();
		//                                       ↓ allow bigger defines: they are prepared by the user in advance
		bool wasTooSmall=false;
		if(curOpt->doMarginalDistribution.size() < rank) { // Did the user define marginal distributions?
			std::cerr << "WARNING: displayOptions::doMarginalDistribution too small!\n";
			curOpt->doMarginalDistribution.resize(rank,Vector3i(-1,-1,-1));
			wasTooSmall=true;
		};
		bool marginalDistributionDefined=true;
		for(size_t i=0 ; i < rank ; i++ ) { // verify that marginal distribution is properly defined
			Vector3i& d = curOpt->doMarginalDistribution[i];
			if(   d[0]<0 or d[0] > 2 or d[1] > d[2]
			   or d[1]<0 or d[1] > (int)qms->getPsiGlobalExisting()->psiGlobalTable.dim()[i]
			   or d[2]<0 or d[2] > (int)qms->getPsiGlobalExisting()->psiGlobalTable.dim()[i] )
				marginalDistributionDefined=false;
		}
		if(not marginalDistributionDefined) {
			//if(timeLimit.messageAllowed(5))
				std::cerr << "Resetting QMDisplayOptions::doMarginalDistribution to valid state\n";
			for(size_t i=0 ; i < rank ; i++ ) {
				Vector3i& d = curOpt->doMarginalDistribution[i];
				if(d[0]<0 or wasTooSmall)   d[0]=2;   // by default integrate everything
				if(d[1]<0 or wasTooSmall)   d[1]=0;   // from zero
				int max=(int)(qms->getPsiGlobalExisting()->psiGlobalTable.dim()[i]);
				if(d[2]>max or wasTooSmall) d[2]=max; // to last node
			}
			for(std::size_t i = qms->partOfpsiGlobalZero() ; i < (qms->partOfpsiGlobalZero()+qms->dim()) ;  i++) {
				Vector3i& d = curOpt->doMarginalDistribution[i];
				d[0]=0;                                                    // but don't integrate what belongs to this particle
			}
		}
		// 1.1. when d[0]==2, make sure that EVERYTHING is integrated, or if it's not integrated d[0]==0 also set it to reasonable default
		for(size_t i=0 ; i < rank ; i++ ) {
			Vector3i& d = curOpt->doMarginalDistribution[i];
			if(d[0]==2 or d[0]==0) {
				d[1]=0;                                                    // from zero
				d[2]=qms->getPsiGlobalExisting()->psiGlobalTable.dim()[i]; // to last node ← this one needs updating, if user changed gridSize
			}
		}
		// 1.2. now build string representation for current marginal distribution
		vector<std::string> utfNum={"₀","₁","₂","₃","₄","₅","₆","₇","₈","₉"};
		vector<std::string> intSym={" ","∫","∬","∭"};//,"⨌"};
		std::string intLetters="xyz";
		std::string representation="";
		std::string intVolume="";
		std::string intBounds="",normSq1="|",normSq2="|²";
		size_t intNum(0); // how much to integrate
		for(auto& d : curOpt->doMarginalDistribution) if(d[0]!=0) intNum++;
		if(intNum != rank and curOpt->marginalDensityOnly == false) { normSq1=""; normSq2=""; };
		size_t drawSpatialDim = rank - intNum;
		if(drawSpatialDim>3) {  std::cerr << "ERROR: cannot plot more than 3D"; continue;  };
		representation += intSym[intNum % (intSym.size()-1)];
		for(int s=0 ; s<((int)(intNum)/(int)(intSym.size()-1)) ; s++) representation += intSym[intSym.size()-1];
		representation += normSq1+std::string("ψ(");
		Vector3r mySize(0,0,0); size_t mySizePos=0;
		curOpt->renderAxis_i=Vector3i(-1,-1,-1);
		for(size_t i=0 ; i < rank ; i++ ) {
			representation+=intLetters[i % dimSpatial]+utfNum[int(i / dimSpatial)+1];
			if(i != (rank-1)) representation+=",";
			Vector3i& d = curOpt->doMarginalDistribution[i];
			if(d[0]!=0) {
				intVolume+=std::string("d")      +intLetters[i % dimSpatial]+utfNum[int(i / dimSpatial)+1];
				intBounds+=std::string(",  ")+intLetters[i % dimSpatial]+utfNum[int(i / dimSpatial)+1]+std::string("=")+boost::lexical_cast<std::string>(d[1]);
				intBounds+=std::string("…")  +                                                                          boost::lexical_cast<std::string>(d[2]);
			} else { // What's not integrated: here we prepare for finding correct STEP and rotation
		// 1.3. and prepare renderAxis_i and renderSize along the way
				curOpt->renderGlobal_i[mySizePos  ] = (int)(i);
				curOpt->renderAxis_i  [mySizePos  ] = (i % dimSpatial); // we will have to draw along this axis: 0,1,2 = x,y,z
				curOpt->renderSize    [mySizePos++] = qms->getPsiGlobalExisting()->getSpatialSizeGlobal()[i]; // it will have this size
		// 1.3end.
			}
		}
//std::cerr << " mySizePos      = " << mySizePos      << "\n";
//std::cerr << " drawSpatialDim = " << drawSpatialDim << "\n";
		assert(mySizePos == drawSpatialDim);
		representation+=std::string(",t)")+normSq2+intVolume;
		curOpt->marginalDistribEquation = representation+intBounds;
		// 1.2end. - we just created a nice string like this one: ∫ψ(x₁,y₁)dy₁ start_x₂=0, end_x₂=10
		// 1end. now we have properly defined marginal distributions

		// 2. STEP and SCALE -  take care of step for drawing stuff
		if(qms->isAnalytic()) { // is analytic, or it is a potential, need special care during drawing, because step can be changed
			if(analyticUsesStepOfDiscrete)                             qmg   ->qtReadonly="step";       else qmg   ->qtReadonly="";
			if(analyticUsesScaleOfDiscrete and curOpt->partsScale > 0) curOpt->qtReadonly="partsScale"; else curOpt->qtReadonly="";
			// Set SCALE                        don't rescale if it's negative (division)   ↓
			if(analyticUsesScaleOfDiscrete and lastDiscreteScale > 0 and curOpt->partsScale > 0) {
				curOpt->partsScale = lastDiscreteScale;
			}
			// Set STEP
			if(qmg->step[0]<0 or qmg->step[1]<0 or qmg->step[2]<0)
			{ // we can have undefined step, if user hasn't set it to anything. Fortunately there's gridSize to help
				qmg->step=Vector3r(0,0,0);    // (1)                         ↓ find step from gridSize
				for(size_t i=0 ; i < dimSpatial     ; i++) qmg->step[i]=qms->getSpatialSize()[i]/((Real)(qms->gridSize[i]));
			}
			if(analyticUsesStepOfDiscrete and lastDiscreteStep[0] > 0) {
				qmg->step = lastDiscreteStep; // (2)
			}
			if(qmg->step != qmg->lastStep or qms->wasGenerated==false) { // here we need to regenerate analytic psiGlobalTable since the step has changed.
				// lastStep detects changes on 3 occassions:
				// (1) and (2) above. And (3) when step was changed from user interface. In each case we need to regenerate psiGlobalTable
				qmg->lastStep = qmg->step; //                                    ↓ find gridSize from step
				for(size_t i=0 ; i < dimSpatial ; i++) qms->gridSize[i]=size_t(qms->getSpatialSize()[i]/qmg->step[i]);
				// need to recalculate step, because of rounding errors for gridSize.
				for(size_t i=0 ; i < dimSpatial ; i++) qmg->step[i]=qms->getSpatialSize()[i]/((Real)(qms->gridSize[i]));

				// But there's only one psiGlobalTable for each particle, so steps in ALL displayOptions must be the same.
// FIXME: przeliczał jeśli zmienił się `step`:
//std::cerr << "  → Updejtuję TERAZ: " << qmg->getClassName() << "  " << qms->getClassName() << "\n";
//std::cerr << " qmg->step = " << qmg->step << "  qmg->lastStep = " << qmg->lastStep << "  qms->wasGenerated = " << qms->wasGenerated << "\n";
	/* Fx3 */			qms->update(); //update Yourself Pretty Please I Know You Shouldn't Do That Because You Are Just Some Data();
// FIXME: 
	/* Fx3 */			curOpt->lastMarginalDistributionCalculatedIter = -1;
			}
			curOpt->step=qmg->step;
			for(size_t i=0 ; i < dimSpatial ; i++) {
				curOpt->start[i] = qms->start(i);             // FIXME? or not? problem is that N-nodes have (N-1) lines between: |---|---|---|---|---
				curOpt->end[i]   = qms->end(i)- qmg->step[i]; // maybe change the start() and end() values in QMStateDiscrete??   ¹ 1 ² 2 ³ 3 ⁴ 4 ⁵ 5
			}
		}
/*FIXME: tutaj było `else`, ale dla marginalDistribution póki co je wywalam, żeby step poniżej się jakoś wyznaczał */ 
	/* Fx3 */	// else
		{
		// finding step for discrete is different: we must deal with possible marginalDistribution,
		// currently I don't support marginalDistribution of analytical solutions
			qmg->qtReadonly="step";qmg->step=Vector3r(0,0,0);
			for(size_t i=0 ; i < dimSpatial ; i++) qmg->lastStep[i] = (qmg->step[i] = qms->stepInPositionalRepresentation(i));
			lastDiscreteStep  = qmg->step;
			lastDiscreteScale = curOpt->partsScale;

			curOpt->step=Vector3r(0,0,0);
			for(size_t i=0 ; i < drawSpatialDim ; i++) {
				curOpt->step [i]=qms->getPsiGlobalExisting()->getSpatialSizeGlobal()[curOpt->renderGlobal_i[i]]/((Real)(qms->getPsiGlobalExisting()->gridSize[curOpt->renderGlobal_i[i]]));
				curOpt->start[i]=qms->getPsiGlobalExisting()->start                 (curOpt->renderGlobal_i[i]);                  // FIXME? or not? problem is that N-nodes have (N-1) lines between: |---|---|---|---|---
				curOpt->end  [i]=qms->getPsiGlobalExisting()->end                   (curOpt->renderGlobal_i[i])- curOpt->step[i]; // maybe change the start() and end() values in QMStateDiscrete??   ¹ 1 ² 2 ³ 3 ⁴ 4 ⁵ 5
			}
		}
//if(timeLimit.messageAllowed(5)) {
//std::cerr << "step         found: " << curOpt->step << "\n";
//std::cerr << "renderSize   found: " << curOpt->renderSize << "\n";
//std::cerr << "renderAxis_i found: " << curOpt->renderAxis_i << "\n";
//}
		// 2end. STEP is found

		// 3. MARGINAL DISTRIBUTIONS - now we can calculate marginal distribution along some dimension
		if(   curOpt->lastMarginalDistributionCalculatedIter != scene->iter
		   or curOpt->lastMarginalNormalize      != curOpt->marginalNormalize
		   or curOpt->lastMarginalDensityOnly    != curOpt->marginalDensityOnly
		   or curOpt->lastDoMarginalDistribution != curOpt->doMarginalDistribution
		   or curOpt->lastRenderFFT != curOpt->renderFFT
		   or intNum == rank
		)
		{
			curOpt->lastMarginalDistributionCalculatedIter = scene->iter;
			curOpt->lastMarginalNormalize      = curOpt->marginalNormalize;
			curOpt->lastMarginalDensityOnly    = curOpt->marginalDensityOnly;
			curOpt->lastDoMarginalDistribution = curOpt->doMarginalDistribution;
			curOpt->lastRenderFFT              = curOpt->renderFFT;
			if(intNum==0) { // nothing to integrate
				curOpt->marginalDistribution = maybeTransform(qms,dimSpatial,curOpt,qms->getPsiGlobalExisting()->psiGlobalTable);
				if(curOpt->renderFFT) curOpt->marginalDistribution.niceFFT();
			} else if(intNum == rank) { // Integrate everything!
				if(timeLimit.messageAllowed(5)) {
					std::cout << representation << "="
						<< boost::lexical_cast<std::string>
						   (
						        qms->getPsiGlobalExisting()->psiGlobalTable.integrateAllNormSquared
							(
							     qms->getPsiGlobalExisting()->getSpatialSizeGlobal()
							)
						   )
						<< "  (whole wavefunction integrated, nothing to draw)\n";
				}
			} else {
				std::vector<short int> remainDims(rank, 0);
				for(std::size_t i = 0 ; i < rank ;  i++) if(curOpt->doMarginalDistribution[i][0]==0) remainDims[i]=1;
				if(curOpt->renderFFT) {
					NDimTable<Complexr>      tmp = maybeTransform(qms,dimSpatial,curOpt,qms->getPsiGlobalExisting()->psiGlobalTable);
					tmp.niceFFT();
					curOpt->marginalDistribution = tmp                                                               .calcMarginalDistribution     (remainDims,qms->getPsiGlobalExisting()->getSpatialSizeGlobal()/* FIXME - spatialSize is incorrect in momentum (inverse) space, and maybe wrong after maybeTransform (which now uses the same size for this reason) */,curOpt->marginalNormalize,curOpt->marginalDensityOnly);
				} else {
/* 1 */// static bool printed=false;
/* 1 */// if(true /*not printed*/) {
/* 1 */// 	std::cout << "\n-1=============================-\n";
/* 1 */// 	qms->getPsiGlobalExisting()->psiGlobalTable.print();
/* 1 */// 	std::cout << "\n-1=============================-\n";
/* 1 */// 	printed=true;
/* 1 */// }
/* 1 */// std::cerr << "\n\n CRASH ???? \n\n";
/* 1 */// std::cerr << " intNum     = " << intNum     << "\n";
/* 1 */// std::cerr << " dimSpatial = " << dimSpatial << "\n";
/* 1 */// std::cerr << " rank()     = " << qms->getPsiGlobalExisting()->psiGlobalTable.rank() << "\n";

					curOpt->marginalDistribution = maybeTransform(qms,dimSpatial,curOpt,qms->getPsiGlobalExisting()->psiGlobalTable).calcMarginalDistribution     (remainDims,qms->getPsiGlobalExisting()->getSpatialSizeGlobal(),curOpt->marginalNormalize,curOpt->marginalDensityOnly);
				}
			}
		}
		if(intNum == rank) continue; // nothing to draw!
		// 3end. marginal distributions are calculated

		Gl1_NDimTable worker;
		glPushMatrix();
		glTranslatev (curOpt->renderSe3.position);
		AngleAxisr aa(curOpt->renderSe3.orientation);
		if(curOpt->renderFFT) glScalev(curOpt->renderFFTScale);
		glRotatef(aa.angle()*Mathr::RAD_TO_DEG,aa.axis()[0],aa.axis()[1],aa.axis()[2]);
		worker.drawNDimTable(curOpt->marginalDistribution, curOpt,qmg->color,forceWire,glinfo);
		glPopMatrix();
	}
};

NDimTable<Complexr>& Gl1_QMGeometry::maybeTransform(QMStateDiscrete* qms,size_t dimSpatial, shared_ptr<QMDisplayOptions>& opt, NDimTable<Complexr>& arg)
{
	if(not opt->renderRotated45)
	{
		return arg;
	}
	else {
		transformed = arg;
		transformed.calcTransformedFromOther(
			  arg
			, dimSpatial
			, {// vector of functions
			 // r coordinate:   r = r1-r2;
			 [&](std::vector<Vector3r>& xyzTab, unsigned int coordIdx)->Real
			 {
				assert(xyzTab.size() ==2);
				Vector3r r1 = xyzTab[0];
				Vector3r r2 = xyzTab[1];

				Vector3r r  = (r1-r2)*0.5; // FIXME - raczej bez tego 0.5
				return r   [coordIdx]; // is calculated separately for each coordinate x,y,z
			 }
			,// R coordinate:   R = (m1*r1+m2*r2)/(m1+m2)
			 [&](std::vector<Vector3r>& xyzTab, unsigned int coordIdx)->Real
			 {
				assert(xyzTab.size() ==2);
				Vector3r r1 = xyzTab[0];
				Vector3r r2 = xyzTab[1];

				Real m1 = 1; // FIXME - wyprowadzić parametr `masa` na zewnątrz tej funkcji, do skryptu pythonowego
				Real m2 = 1; // FIXME - wyprowadzić parametr `masa` na zewnątrz tej funkcji, do skryptu pythonowego
				Vector3r R  = (m1*r1+m2*r2)/(m1+m2);
				return R   [coordIdx]; // is calculated separately for each coordinate x,y,z
			 }
			}
			, [&](Real i, int d)->Real    { return /* this == transformed */          qms->getPsiGlobalExisting()->iToX     (i,d);} // FIXME - zakładam że mają takie same rozmiary i rozdzielczość
			, [&](Real x, int d)->Real    { return /* other== (arg) original == qms */qms->getPsiGlobalExisting()->xToI_Real(x,d);}
		);
		return transformed;
	}
}

/*********************************************************************************
*
* Q M   M E C H A N I C A L   O P E N   G L   D I S P L A Y
*
*********************************************************************************/












///////////// dziś zacznę tutaj ---------------------
//  1. Gl1_QMIPhys, Gl1_QMGPhys, Gl1_QMGeometry wołają tę samą funkcję, z dwoma tylko argumentami: displayOptions oraz odpowiednie psiGlobal
//  2. w środku tylko jedna krótka parolinijkowa pętla po std::vector<shared_ptr<Serializable>> displayOptions
//  3. stamtąd wołane całe to ustrojstwo które tyle czasu pisałem
//
//  4. następnie mogę wyrwać pętlę generującą potencjały do klasy bazowej
//
//  5. i wtedy mogę wprowadzić pot. Coulomba
//
//  6. plotować w yade na żywo, dokładność obliczeń                                                                       ↓ wyprowadzić do pythona
       //                                                       " ∫|ψ|²dx = " << boost::lexical_cast<std::string>(qms->  psiMarginalDistribution.integrateAllNormSquared(qms->size)) << "\n";};
       //"integrate marginal "; CERR(qms->           gridSize); " ∫|ψ|²dx = " << boost::lexical_cast<std::string>(qms->  psiMarginalDistribution.integrateAllNormSquared(qms->size           )) << "\n";};
       //"integrate whole    "; CERR(qms->psiGlobal->gridSize); " ∫|ψ|²dx = " << boost::lexical_cast<std::string>(qms->psiGlobal->psiGlobalTable.integrateAllNormSquared(qms->psiGlobal->size)) << "\n";};











// This will come later, when I will have some interactions going on.... FIXME - draw potentials !!
//	CREATE_LOGGER(Gl1_QMIPhys);
//	bool Gl1_QMIPhys::abs=true;
//	bool Gl1_QMIPhys::real=false;
//	bool Gl1_QMIPhys::imag=false;
//	Gl1_QMIPhys::~Gl1_QMIPhys(){};
//	void Gl1_QMIPhys::go(const shared_ptr<IPhys>& ip, const shared_ptr<Interaction>& i, const shared_ptr<Body>& b1, const shared_ptr<Body>& b2, bool wireFrame)
//	{
//	}



/*********************************************************************************
*
* Q M   O P E N   G L   D R A W I N G       draws NDimTable marginal distributions
*
*********************************************************************************/

CREATE_LOGGER(Gl1_NDimTable);
Gl1_NDimTable::~Gl1_NDimTable(){};

Gl1_NDimTable::Gl1_NDimTable()
{
	partsToDraw.resize(0);
	partsToDraw.push_back( [this](){ return menuSelection(opt->partReal     )!="hidden" and not opt->partsSquared; } );
	partsToDraw.push_back( [this](){ return menuSelection(opt->partImaginary)!="hidden" and not opt->partsSquared; } );
	partsToDraw.push_back( [this](){ return menuSelection(opt->partAbsolute )!="hidden" and not opt->partsSquared; } );
	partsToDraw.push_back( [this](){ return menuSelection(opt->partReal     )!="hidden" and     opt->partsSquared; } );
	partsToDraw.push_back( [this](){ return menuSelection(opt->partImaginary)!="hidden" and     opt->partsSquared; } );
	partsToDraw.push_back( [this](){ return menuSelection(opt->partAbsolute )!="hidden" and     opt->partsSquared; } );

	drawStyle.resize(0);
	drawStyle.push_back  ( [this](){ return menuSelection(opt->partReal     );                                   } );
	drawStyle.push_back  ( [this](){ return menuSelection(opt->partImaginary);                                   } );
	drawStyle.push_back  ( [this](){ return menuSelection(opt->partAbsolute );                                   } );
	drawStyle.push_back  ( [this](){ return menuSelection(opt->partReal     );                                   } );
	drawStyle.push_back  ( [this](){ return menuSelection(opt->partImaginary);                                   } );
	drawStyle.push_back  ( [this](){ return menuSelection(opt->partAbsolute );                                   } );

	valueToDraw.resize(0);
	valueToDraw.push_back( [](std::complex<Real> a){ return std::real(a);                                      } );
	valueToDraw.push_back( [](std::complex<Real> a){ return std::imag(a);                                      } );
	valueToDraw.push_back( [](std::complex<Real> a){ return std::abs(a);                                       } );
	valueToDraw.push_back( [](std::complex<Real> a){ return std::pow(std::real(a),2);                          } );
	valueToDraw.push_back( [](std::complex<Real> a){ return std::pow(std::imag(a),2);                          } );
	valueToDraw.push_back( [](std::complex<Real> a){ return std::real(a*std::conj(a));                         } );

	colorToDraw.resize(0);                                                                  // draw FFT in yellowish color
	colorToDraw.push_back( [this](Vector3r col){ Vector3r FFTcol(1,1,1); if(opt->renderFFT) FFTcol=Vector3r(1.1,1.1,0.7); return Vector3r(col.cwiseProduct(Vector3r(0.4,0.4,1.0)).cwiseProduct(FFTcol)); } ); // display partReal               in bluish   color
	colorToDraw.push_back( [this](Vector3r col){ Vector3r FFTcol(1,1,1); if(opt->renderFFT) FFTcol=Vector3r(1.1,1.1,0.7); return Vector3r(col.cwiseProduct(Vector3r(1.0,0.4,0.4)).cwiseProduct(FFTcol)); } ); // display partImaginary          in reddish  color
	colorToDraw.push_back( [this](Vector3r col){ Vector3r FFTcol(1,1,1); if(opt->renderFFT) FFTcol=Vector3r(1.1,1.1,0.7); return Vector3r(col                                    .cwiseProduct(FFTcol)); } ); // display abs value              original color
	colorToDraw.push_back( [this](Vector3r col){ Vector3r FFTcol(1,1,1); if(opt->renderFFT) FFTcol=Vector3r(1.1,1.1,0.7); return Vector3r(col.cwiseProduct(Vector3r(0.4,0.4,1.0)).cwiseProduct(FFTcol)); } ); // display partReal      (squred) in bluish   color
	colorToDraw.push_back( [this](Vector3r col){ Vector3r FFTcol(1,1,1); if(opt->renderFFT) FFTcol=Vector3r(1.1,1.1,0.7); return Vector3r(col.cwiseProduct(Vector3r(1.0,0.4,0.4)).cwiseProduct(FFTcol)); } ); // display partImaginary (square) in reddish  color
	colorToDraw.push_back( [this](Vector3r col){ Vector3r FFTcol(1,1,1); if(opt->renderFFT) FFTcol=Vector3r(1.1,1.1,0.7); return Vector3r(col                                    .cwiseProduct(FFTcol)); } ); // for probability use            original color
};

void Gl1_NDimTable::calcNormalVectors(
	  const std::vector<std::vector<Real> >& waveVals  // a 2D matrix of wavefunction values evaluated at certain point in positional spatial space
	, std::vector<std::vector<Vector3r> >& wavNormV    // normal vectors necessary for propoer OpenGL rendering of the faces
)
{
	// Now I have waveVals[i][j] filled with values to plot. So calculate all normals. First normals for triangles
	//      *        1                                                     
	//    / |1\    / | \     wavNormV[i][j] stores normal for vertex[i][j] averaged from all 4 neigbour normals (except for edges)
	//   *--x--*  4--0--2
	//    \ | /    \ | /
	//      *        3    
	Vector3r p0(0,0,0),p1(0,0,0),p2(0,0,0),p3(0,0,0),p4(0,0,0);
	Vector3r           n1(0,0,0),n2(0,0,0),n3(0,0,0),n4(0,0,0);
	for(struct{size_t i;Real x;}_={0,opt->start.x()} ; _.i<data->dim()[0] ; _.i++, _.x+=opt->step.x() ) {
		for(struct{size_t j;Real y;}__={0,opt->start.y()} ; __.j<data->dim()[1] ; __.j++, __.y+=opt->step.y() ) {
			                              p0=Vector3r( _.x              ,__.y              ,waveVals[ _.i  ][__.j  ]);
			if((__.j+1)<data->dim()[1]) { p1=Vector3r( _.x              ,__.y+opt->step.y(),waveVals[ _.i  ][__.j+1]);} else{ p1=p0;};
			if(( _.i+1)<data->dim()[0]) { p2=Vector3r( _.x+opt->step.x(),__.y              ,waveVals[ _.i+1][__.j  ]);} else{ p2=p0;};
			if((__.j  )>=1             ){ p3=Vector3r( _.x              ,__.y-opt->step.y(),waveVals[ _.i  ][__.j-1]);} else{ p3=p0;};
			if(( _.i  )>=1             ){ p4=Vector3r( _.x-opt->step.x(),__.y              ,waveVals[ _.i-1][__.j  ]);} else{ p4=p0;};
			n1 = (p2-p0).cross(p1-p0);
			n2 = (p3-p0).cross(p2-p0);
			n3 = (p4-p0).cross(p3-p0);
			n4 = (p1-p0).cross(p4-p0);
			int count=0;
			if(n1.squaredNorm()>0) { n1.normalize(); count++;};
			if(n2.squaredNorm()>0) { n2.normalize(); count++;};
			if(n3.squaredNorm()>0) { n3.normalize(); count++;};
			if(n4.squaredNorm()>0) { n4.normalize(); count++;};
			wavNormV[ _.i][__.j] = Vector3r(n1+n2+n3+n4)/(1.0*count);
		}
	}
}

void Gl1_NDimTable::interpolateExtraWaveValues(
	  const std::vector<std::vector<Real> >& waveVals  // a 2D matrix of wavefunction values evaluated at certain point in positional spatial space
	,  std::vector<std::vector<Real> >& extraWaveVals  // a 2D matrix shifted by +0.5,+0.5 from the previous one.
)
{
	const size_t CHOSEN_RANGE=6; // FIXME - add option to select between sinc256 and spline36

	for(size_t i=CHOSEN_RANGE/2 ; i<(data->dim()[0]-CHOSEN_RANGE/2) ; i++ ) // skip borders of width CHOSEN_RANGE/2 - cannot interpolate there
	{
		for(size_t j=CHOSEN_RANGE/2 ; j<(data->dim()[1]-CHOSEN_RANGE/2) ; j++ )
		{
			extraWaveVals[i][j]=spline6InterpolatePoint2D<Real,Real>(waveVals,i+0.5,j+0.5);
		}
	}
}

void Gl1_NDimTable::interpolateExtraNormalVectors(
	const std::vector<std::vector<Vector3r> >& wavNormV,// a 2D matrix of normal vectors necessary for proper OpenGL rendering of the faces
	std::vector<std::vector<Vector3r> >& extraWavNormV  // a 2D matrix shifted by +0.5,+0.5 from the previous one
)
{
	const size_t CHOSEN_RANGE=6; // FIXME - add option to select between sinc256 and spline36

	for(size_t i=CHOSEN_RANGE/2 ; i<(data->dim()[0]-CHOSEN_RANGE/2) ; i++ ) // skip borders of width CHOSEN_RANGE/2 - cannot interpolate there
	{
		for(size_t j=CHOSEN_RANGE/2 ; j<(data->dim()[1]-CHOSEN_RANGE/2) ; j++ )
		{
			extraWavNormV[i][j]=spline6InterpolatePoint2D<Vector3r,Real>(wavNormV,i+0.5,j+0.5);
		}
	}
}

void Gl1_NDimTable::prepareGlSurfaceMaterial()
{
	GLfloat mat[4];
	mat[0] = opt->renderSpecular/100.0;
	mat[1] = opt->renderSpecular/100.0;
	mat[2] = opt->renderSpecular/100.0;
	mat[3] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat);
	mat[0] = opt->renderAmbient/100.0;
	mat[1] = opt->renderAmbient/100.0;
	mat[2] = opt->renderAmbient/100.0;
	mat[3] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat);
	mat[0] = opt->renderDiffuse/100.0;
	mat[1] = opt->renderDiffuse/100.0;
	mat[2] = opt->renderDiffuse/100.0;
	mat[3] = 1;
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat);
	GLfloat sh=opt->renderShininess;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS,&sh);
}

void Gl1_NDimTable::glDrawSurface(
	  const std::vector<std::vector<Real> >& waveVals      // a 2D matrix of wavefunction values evaluated at certain point in positional spatial space
	, const std::vector<std::vector<Vector3r> >& wavNormV  // normal vectors necessary for proper OpenGL rendering of the faces
	, Vector3r col                                         // color in which to draw the surface
)
{
	// now draw surface
	prepareGlSurfaceMaterial();

	glEnable(GL_CULL_FACE);
	if(opt->renderSmoothing){ glShadeModel(GL_SMOOTH);};
	//draw front
	glCullFace(GL_BACK);
	glColor3v(col);
	for(struct{size_t i;Real x;}_={0,opt->start.x()} ; _.i<data->dim()[0]-1 ; _.i++, _.x+=opt->step.x() ) {
		glBegin(GL_TRIANGLE_STRIP);
		for(struct{size_t j;Real y;}__={0,opt->start.y()} ; __.j<data->dim()[1] ; __.j++, __.y+=opt->step.y() ) {
			glNormal3v(                        wavNormV[ _.i  ][__.j]);
			glVertex3f( _.x              ,__.y,waveVals[ _.i  ][__.j]);
			glNormal3v(                        wavNormV[ _.i+1][__.j]);
			glVertex3f( _.x+opt->step.x(),__.y,waveVals[ _.i+1][__.j]);
		}
		glEnd();
	}
	//********************* draw back side of this surface
	//glCullFace(GL_BACK); // unnecessary
	glColor3v(Vector3r(col.cwiseProduct(Vector3r(0.5,0.5,0.5)))); // back has darker colors
	for(struct{size_t i;Real x;}_={0,opt->start.x()} ; _.i<data->dim()[0]-1 ; _.i++, _.x+=opt->step.x() ) {
		glBegin(GL_TRIANGLE_STRIP);
		// NOTE: this j goes downwards - size_t is unsigned, so I must not use j>=0 condition, because j rolls to +∞ after j--
		// FIXME - is that an example of when not to use `size_t` type ? I had to declare extra `size_t jj(__.j-1);` below, to solve this
		// and I had to start counting from `data->dim()[1]`, while I should start from `data->dim()[1]-1`
		for(struct{size_t j;Real y;}__={data->dim()[1],opt->start.y()+opt->step.y()*(data->dim()[1]-1)} ; __.j>0 ; __.j--, __.y-=opt->step.y() ) {
			size_t jj(__.j-1);
			glNormal3v(                        wavNormV[ _.i  ][  jj  ]);
			glVertex3f( _.x              ,__.y,waveVals[ _.i  ][  jj  ]);
			glNormal3v(                        wavNormV[ _.i+1][  jj  ]);
			glVertex3f( _.x+opt->step.x(),__.y,waveVals[ _.i+1][  jj  ]);
		}
		glEnd();
	}
	glShadeModel(GL_FLAT);
}

void Gl1_NDimTable::glDrawSurfaceInterpolated(
	  const std::vector<std::vector<Real> >&     waveVals       // a 2D matrix of wavefunction values evaluated at certain point in positional spatial space
	, const std::vector<std::vector<Vector3r> >& wavNormV       // normal vectors necessary for proper OpenGL rendering of the faces
	, const std::vector<std::vector<Real> >&     extraWaveVals  // same, but shifted by +0.5,+0.5
	, const std::vector<std::vector<Vector3r> >& extraWavNormV  // same, but shifted by +0.5,+0.5
	, Vector3r col                                              // color in which to draw the surface
)
{
	Real stepx  = opt->step.x()    ;
	Real stepx2 = opt->step.x()*0.5;
	Real stepy  = opt->step.y()    ;
	Real stepy2 = opt->step.y()*0.5;
	Real stepy3 = opt->step.y()*1.5;
	Real stepy4 = opt->step.y()*2.0;
	const size_t CHOSEN_RANGE=6; // FIXME - add option to select between sinc256 and spline36

	// now draw surface
	prepareGlSurfaceMaterial();

	glEnable(GL_CULL_FACE);
	if(opt->renderSmoothing){ glShadeModel(GL_SMOOTH);};
	//draw front
	glCullFace(GL_BACK);
	glColor3v(col);
//
//   2 ... 4  ? (8)  triangles marked with "?" are not drawn in this loop
//   | \ / | ⋱ ⋰ |
// i ↑  3  ↓  6  ↑   the (7) and (8) are done in the next loop
// x | ⋰ ⋱ | / \ |
// ↑ 1  ?  5 ...(7)
// +→y,j
	for(size_t i=CHOSEN_RANGE/2 ; i<(data->dim()[0]-1-CHOSEN_RANGE/2) ; i++ ) // skip margin CHOSEN_RANGE/2 where interpolation was impossible
	{
		Real x=opt->start.x()+i*stepx;
		glBegin(GL_TRIANGLE_STRIP);
		for(size_t j=CHOSEN_RANGE/2 ; j<(data->dim()[1]-CHOSEN_RANGE/2-2) ; j+=2 /* must draw two quadrants at a time */ )
		{
			Real y=opt->start.y()+j*stepy;
			glNormal3v(                       wavNormV[i  ][j  ]); // 1
			glVertex3f(x       ,y       ,     waveVals[i  ][j  ]); // 1
			glNormal3v(                       wavNormV[i+1][j  ]); // 2
			glVertex3f(x+stepx ,y       ,     waveVals[i+1][j  ]); // 2
			glNormal3v(                  extraWavNormV[i  ][j  ]); // 3
			glVertex3f(x+stepx2,y+stepy2,extraWaveVals[i  ][j  ]); // 3
			glNormal3v(                       wavNormV[i+1][j+1]); // 4
			glVertex3f(x+stepx ,y+stepy ,     waveVals[i+1][j+1]); // 4
			glNormal3v(                       wavNormV[i  ][j+1]); // 5
			glVertex3f(x       ,y+stepy ,     waveVals[i  ][j+1]); // 5
			glNormal3v(                  extraWavNormV[i  ][j+1]); // 6
			glVertex3f(x+stepx2,y+stepy3,extraWaveVals[i  ][j+1]); // 6
			if((j+2)>=(data->dim()[1]-CHOSEN_RANGE/2-2)) { // near the end draw the (7) and (8)
			glNormal3v(                       wavNormV[i  ][j+2]); // 7
			glVertex3f(x       ,y+stepy4,     waveVals[i  ][j+2]); // 7
			glNormal3v(                       wavNormV[i+1][j+2]); // 8
			glVertex3f(x+stepx ,y+stepy4,     waveVals[i+1][j+2]); // 8
			}
		}
		glEnd();
// now draw triangles marked with "?"
		glBegin(GL_TRIANGLES);
		for(size_t j=CHOSEN_RANGE/2 ; j<(data->dim()[1]-CHOSEN_RANGE/2-2) ; j+=2 /* must draw two quadrants at a time */ )
		{
			Real y=opt->start.y()+j*stepy;
			glNormal3v(                       wavNormV[i  ][j  ]); // 1
			glVertex3f(x       ,y       ,     waveVals[i  ][j  ]); // 1
			glNormal3v(                  extraWavNormV[i  ][j  ]); // 3
			glVertex3f(x+stepx2,y+stepy2,extraWaveVals[i  ][j  ]); // 3
			glNormal3v(                       wavNormV[i  ][j+1]); // 5
			glVertex3f(x       ,y+stepy ,     waveVals[i  ][j+1]); // 5

			glNormal3v(                       wavNormV[i+1][j+1]); // 4
			glVertex3f(x+stepx ,y+stepy ,     waveVals[i+1][j+1]); // 4
			glNormal3v(                       wavNormV[i+1][j+2]); // 8
			glVertex3f(x+stepx ,y+stepy4,     waveVals[i+1][j+2]); // 8
			glNormal3v(                  extraWavNormV[i  ][j+1]); // 6
			glVertex3f(x+stepx2,y+stepy3,extraWaveVals[i  ][j+1]); // 6
		}
		glEnd();
	}
	//********************* draw back side of this surface
	//glCullFace(GL_BACK); // unnecessary
	glColor3v(Vector3r(col.cwiseProduct(Vector3r(0.5,0.5,0.5)))); // back has darker colors
	for(size_t i=CHOSEN_RANGE/2 ; i<(data->dim()[0]-1-CHOSEN_RANGE/2) ; i++ )
	{
		Real x=opt->start.x()+i*stepx;
		glBegin(GL_TRIANGLE_STRIP);
		for(size_t j=(data->dim()[1]-CHOSEN_RANGE/2-1/*3*/) ; j>CHOSEN_RANGE/2 ; j-=2 )
		{
			Real y=opt->start.y()+j*stepy;
			glNormal3v(                       wavNormV[i  ][j  ]); // 1
			glVertex3f(x       ,y       ,     waveVals[i  ][j  ]); // 1
			glNormal3v(                       wavNormV[i+1][j  ]); // 2
			glVertex3f(x+stepx ,y       ,     waveVals[i+1][j  ]); // 2
			glNormal3v(                  extraWavNormV[i  ][j-1]); // 3
			glVertex3f(x+stepx2,y-stepy2,extraWaveVals[i  ][j-1]); // 3
			glNormal3v(                       wavNormV[i+1][j-1]); // 4
			glVertex3f(x+stepx ,y-stepy ,     waveVals[i+1][j-1]); // 4
			glNormal3v(                       wavNormV[i  ][j-1]); // 5
			glVertex3f(x       ,y-stepy ,     waveVals[i  ][j-1]); // 5
			glNormal3v(                  extraWavNormV[i  ][j-2]); // 6
			glVertex3f(x+stepx2,y-stepy3,extraWaveVals[i  ][j-2]); // 6
			if((j-2)<=(CHOSEN_RANGE/2)) { // near the end draw the (7) and (8)
			glNormal3v(                       wavNormV[i  ][j-2]); // 7
			glVertex3f(x       ,y-stepy4,     waveVals[i  ][j-2]); // 7
			glNormal3v(                       wavNormV[i+1][j-2]); // 8
			glVertex3f(x+stepx ,y-stepy4,     waveVals[i+1][j-2]); // 8
			}
		}
		glEnd();
// now draw triangles marked with "?"
		glBegin(GL_TRIANGLES);
		for(size_t j=(data->dim()[1]-CHOSEN_RANGE/2-1/*3*/) ; j>CHOSEN_RANGE/2 ; j-=2 )
		{
			Real y=opt->start.y()+j*stepy;
			glNormal3v(                       wavNormV[i  ][j  ]); // 1
			glVertex3f(x       ,y       ,     waveVals[i  ][j  ]); // 1
			glNormal3v(                  extraWavNormV[i  ][j-1]); // 3
			glVertex3f(x+stepx2,y-stepy2,extraWaveVals[i  ][j-1]); // 3
			glNormal3v(                       wavNormV[i  ][j-1]); // 5
			glVertex3f(x       ,y-stepy ,     waveVals[i  ][j-1]); // 5

			glNormal3v(                       wavNormV[i+1][j-1]); // 4
			glVertex3f(x+stepx ,y-stepy ,     waveVals[i+1][j-1]); // 4
			glNormal3v(                       wavNormV[i+1][j-2]); // 8
			glVertex3f(x+stepx ,y-stepy4,     waveVals[i+1][j-2]); // 8
			glNormal3v(                  extraWavNormV[i  ][j-2]); // 6
			glVertex3f(x+stepx2,y-stepy3,extraWaveVals[i  ][j-2]); // 6
		}
		glEnd();
	}
	glShadeModel(GL_FLAT);
}

void Gl1_NDimTable::drawSurface(
	  const std::vector<std::vector<Real> >& waveVals
	, Vector3r col
)
{
	std::vector<std::vector<Vector3r> > wavNormV(/*size=*/data->dim()[0],/*init=*/std::vector<Vector3r>(data->dim()[1]));
	calcNormalVectors(waveVals,wavNormV);
	if(not opt->renderInterpolate)
	{
		glDrawSurface(waveVals,wavNormV,col);
	} else {
		std::vector<std::vector<Vector3r> > extraWavNormV(/*size=*/waveVals.size(),/*init=*/std::vector<Vector3r>(waveVals[0].size()));
		std::vector<std::vector<Real    > > extraWaveVals(/*size=*/waveVals.size(),/*init=*/std::vector<Real    >(waveVals[0].size()));
		interpolateExtraWaveValues   (waveVals,extraWaveVals);
		interpolateExtraNormalVectors(wavNormV,extraWavNormV);	
		glDrawSurfaceInterpolated(waveVals,wavNormV,extraWaveVals,extraWavNormV,col);
	}
}

void Gl1_NDimTable::glDrawMarchingCube(MarchingCube& mc,Vector3r col,std::string drawStyle,bool forceWire)
{
	prepareGlSurfaceMaterial();
	glColor3v(col);

	bool WIRE(forceWire == true or drawStyle == "wire");

	const vector<Vector3r>& triangles 	= mc.getTriangles();
	int nbTriangles				= mc.getNbTriangles();
	const vector<Vector3r>& normals 	= mc.getNormals();
	if(opt->renderSmoothing){ glShadeModel(GL_SMOOTH);};
	glEnable(GL_NORMALIZE);
	if(WIRE) {
		if(not opt->renderWireLight) glDisable(GL_LIGHTING);
	} else {
		glBegin(GL_TRIANGLES);
	}
	for(int i=0;i<3*nbTriangles;++i)
	{
		if(WIRE) glBegin(GL_LINE_STRIP);
		glNormal3v(normals[  i]);
		glVertex3v(triangles[i]);
		glNormal3v(normals[++i]);
		glVertex3v(triangles[i]);
		glNormal3v(normals[++i]);
		glVertex3v(triangles[i]);
		if(WIRE) glEnd();
	}
	if(WIRE) {
		if(not opt->renderWireLight) glEnable(GL_LIGHTING);
	} else {
		glEnd();
	}

// FIXME(2) - maybe draw using this approach instead?  http://webhome.csc.uvic.ca/~pouryash/depot/HPC_Course/OpenGLDrawingMethods.pdf
//glColorPointer(3, GL_FLOAT,0,mesh.vColor);
//glEnableClientState(GL_COLOR_ARRAY);
//glNormalPointer(GL_FLOAT, 0,mesh.vNorm);
//glEnableClientState (GL_NORMAL_ARRAY);
//glVertexPointer (3 , GL_FLOAT, 0, mesh.vPos);
//glEnableClientState (GL_VERTEX_ARRAY);
//glDrawElements (GL_TRIANGLES, ( GLsizei) mesh.  ctTriangles * 3, GL_UNSIGNED_SHORT , mesh.triangles);
//glDisableClientState (GL_COLOR_ARRAY);
//glDisableClientState (GL_NORMAL_ARRAY);
//glDisableClientState (GL_VERTEX_ARRAY);

};

void Gl1_NDimTable::drawNDimTable(
	  NDimTable<Complexr>& data_arg
	, shared_ptr<QMDisplayOptions>& opt_arg
	, const Vector3r& col
	, bool forceWire
	, const GLViewInfo&)
{
	timeLimit.readWallClock();
	if(data_arg.rank() > 3) { std::cerr << "ERROR: dim problem with drawing stuff higher than 3D\n"; return; };
	opt  = opt_arg; // needed by partsToDraw lambda functions
	data = &data_arg;

	Real scalingFactor = (opt->partsScale >= 0 ? ((opt->partsScale==0)?(1):(opt->partsScale)) : -1.0/opt->partsScale);
	for(size_t draw=0 ; draw<partsToDraw.size() ; draw++) {
		if( partsToDraw[draw]() ) {
			switch(data->rank()) {
				// FIXME(2) - add following
				// 1D phase  , 2D phase  , 3D phase
				// 1D argand , 2D argand , 3D Dirac-Argand
				// FIXME(2) - add drawing momentum space
				case 1 :
					// FIXME(2) - add points, with point density reflecting the value
					// 1D points ← points scattered randomly, using density
					// 1D nodes  ← just points at the nodes
					// 1D big nodes  ← small spheres at the nodes, radius 1/10 of smallest step_x,step_y,step_z
					// if(points == true) ... else ...
					// 1D lines
					if(not opt->renderWireLight) glDisable(GL_LIGHTING);
					if(drawStyle[draw]()!="points") {
						if(drawStyle[draw]()=="nodes") glBegin(GL_POINTS); else glBegin(GL_LINE_STRIP);
						glColor3v( colorToDraw[draw](col) );
						for(struct{size_t i;Real x;}_={0,opt->start.x()} ; _.i<data->dim()[0] ; _.i++ , _.x+=opt->step.x() ) {
							glVertex3d(_.x,0,valueToDraw[draw] ((data->at(_.i))) *scalingFactor);
							if(drawStyle[draw]()=="bars")
								glVertex3d(_.x,0, 0);
						}
						glEnd();
					} // else "points"
					if(not opt->renderWireLight) glEnable(GL_LIGHTING);
				break;
				case 2:
					// FIXME(2) - add points, with point density reflecting the value
					// 2D points
					// if(points == true) ... else ...
					// 2D lines
					if(forceWire == true or drawStyle[draw]()=="wire") {
						if(not opt->renderWireLight) glDisable(GL_LIGHTING);
						glNormal3f(0,0,1);
						glColor3v( colorToDraw[draw](col) );
						for(struct{size_t i;Real x;}_={0,opt->start.x()} ; _.i<data->dim()[0] ; _.i++, _.x+=opt->step.x() ) {
							glBegin(GL_LINE_STRIP);
							for(struct{size_t j;Real y;}__={0,opt->start.y()} ; __.j<data->dim()[1] ; __.j++, __.y+=opt->step.y() ) {
								glVertex3d(_.x,__.y,valueToDraw[draw] ((data->at(_.i,__.j))) *scalingFactor);
							}
							glEnd();
							if(timeLimit.tooLong(opt->renderMaxTime)) break;
						}
						for(struct{size_t j;Real y;}__={0,opt->start.y()} ; __.j<data->dim()[1] ; __.j++, __.y+=opt->step.y() ) {
							glBegin(GL_LINE_STRIP);
							for(struct{size_t i;Real x;}_={0,opt->start.x()} ; _.i<data->dim()[0] ; _.i++, _.x+=opt->step.x() ) {
								glVertex3d(_.x,__.y,valueToDraw[draw] ((data->at(_.i,__.j))) *scalingFactor);
							}
							glEnd();
							if(timeLimit.tooLong(opt->renderMaxTime)) break;
						}
						if(not opt->renderWireLight) glEnable(GL_LIGHTING);
					} else {
					// 2D surface
						// FIXME!! ↓ is it possible to skip this copying of data to calculate valueToDraw[draw](...) ?
						waveValues2D.resize(data->dim()[0]);
						FOREACH(std::vector<Real>& xx, waveValues2D) {xx.resize(data->dim()[1]);};
						for(size_t i=0 ; i<data->dim()[0] ; i++ ) {
							for(size_t j=0 ; j<data->dim()[1] ; j++ ) {
								waveValues2D[i][j]=valueToDraw[draw] ((data->at(i,j))) *scalingFactor;
							}
							if(timeLimit.tooLong(opt->renderMaxTime)) break;
						}
						drawSurface(waveValues2D,colorToDraw[draw](col));
					}
				break;
				case 3:
					// FIXME(2) - add points, with point density reflecting the value
					// 3D points
					// if(points == true) ... else ...
					// 3D lines
					// 3D surface
		// FIXME - draw transparent MarchingCube for each layer?
		//	http://www.videotutorialsrock.com/opengl_tutorial/reference.php
					if(true /* points == false */) {
						// FIXME!! ↓ is it possible to skip this copying of data to calculate valueToDraw[draw](...) ?
						Vector3r minMC(opt->start.x()+opt->step.x()*0.5,opt->start.y()+opt->step.y()*0.5,opt->start.z()+opt->step.z()*0.5);
						Vector3r maxMC(opt->end.x()  +opt->step.x()*1.5,opt->end.y()  +opt->step.y()*1.5,opt->end.z()  +opt->step.z()*1.5);
						mc.init(data->dim()[0],data->dim()[1],data->dim()[2],minMC,maxMC);
						// about waveValues3D FIXME(2) - resolve storage problems
						mc.resizeScalarField(waveValues3D,data->dim()[0],data->dim()[1],data->dim()[2]);
						for(size_t i=0 ; i<data->dim()[0] ; i++ ) {
							for(size_t j=0 ; j<data->dim()[1] ; j++ ) {
								for(size_t k=0 ; k<data->dim()[2] ; k++ ) {
//// FIXME(1) - is it possible to skip this copying of data to calculate valueToDraw[draw](...) ?
//// (potrzebne mi też będą kontrakcje na życzenie - tylko gdy rysuję)
//// FIXME - here's crash sometimes when step is changed "live" in inspect window
									waveValues3D[i][j][k]=valueToDraw[draw] (data->at(i,j,k));
								}
							}
							if(timeLimit.tooLong(opt->renderMaxTime)) break;
						}
						mc.computeTriangulation(waveValues3D,opt->threshold3D);
						// FIXME(2) - drawSurface or drawWires
						glDrawMarchingCube(mc,colorToDraw[draw](col),drawStyle[draw](),forceWire);
					}
				break;
				default:
					if(timeLimit.messageAllowed(5))
						std::cerr << "4D or more dimensions plotting is not ready yet\n";
				break;
			}
		}
	}
	if(menuSelection(opt->stepRender)!="hidden") {
		switch(data->rank()) {
			case 1:	glBegin(GL_LINE_STRIP); // "frame"
				glNormal3f(0,0,1);
				glColor3v( /*colorToDraw[draw]*/(col) );
				glVertex3d(opt->start.x(),0,0); glVertex3d(opt->end.x(),0,0);
				glEnd();
				if(menuSelection(opt->stepRender)=="stripes" or menuSelection(opt->stepRender)=="mesh") {
					glBegin(GL_LINES);
					for(struct{size_t i;Real x;}_={0,opt->start.x()} ; _.i<data->dim()[0] ; _.i++ , _.x+=opt->step.x() ) {
						glVertex3d(_.x,0, -opt->step.x()*0.3); glVertex3d(_.x,0,  opt->step.x()*0.3);
					}
					glEnd();
				};
			break;
			case 2:	glBegin(GL_LINE_LOOP);
				glNormal3f(0,0,1);
				glColor3v( /*colorToDraw[draw]*/(col) );
				glVertex3d(opt->start.x(),opt->start.y(),0); glVertex3d(opt->start.x(),opt->end.y()  ,0);
				glVertex3d(opt->end.x()  ,opt->end.y()  ,0); glVertex3d(opt->end.x()  ,opt->start.y(),0);
				glEnd();
				if(menuSelection(opt->stepRender)=="stripes" or menuSelection(opt->stepRender)=="mesh") {
					glBegin(GL_LINES);
					for(struct{size_t i;Real x;}_={0,opt->start.x()} ; _.i<data->dim()[0] ; _.i++, _.x+=opt->step.x() ) {
						glVertex3d(_.x,opt->start.y(),0); glVertex3d(_.x,opt->end.y(),0);
					}
					for(struct{size_t j;Real y;}__={0,opt->start.y()} ; __.j<data->dim()[1] ; __.j++, __.y+=opt->step.y() ) {
						glVertex3d(opt->start.x(),__.y,0); glVertex3d(opt->end.x(),__.y,0);
					}
					glEnd();
					if(timeLimit.tooLong(opt->renderMaxTime)) break;
				};
			break;
			case 3:	if(true/*menuSelection(opt->stepRender)=="stripes" or menuSelection(opt->stepRender)=="mesh"*/) {
					bool frameOnly = (menuSelection(opt->stepRender)=="frame");
					glBegin(GL_LINES);
					glColor3v( /*colorToDraw[draw]*/(col) );
					for(struct{size_t i;Real x;}_={0,opt->start.x()} ; _.i<data->dim()[0] ; _.i++, _.x+=opt->step.x() ) {
						if(frameOnly and _.i!=0 and _.i!=data->dim()[0]-1) continue;
						glVertex3d(_.x      ,opt->start.y(),opt->start.z()); glVertex3d(_.x      ,opt->end  .y(),opt->start.z());
						glVertex3d(_.x      ,opt->start.y(),opt->end  .z()); glVertex3d(_.x      ,opt->end  .y(),opt->end  .z());
						glVertex3d(_.x      ,opt->start.y(),opt->start.z()); glVertex3d(_.x      ,opt->start.y(),opt->end  .z());
						glVertex3d(_.x      ,opt->end  .y(),opt->start.z()); glVertex3d(_.x      ,opt->end  .y(),opt->end  .z());
					}
					for(struct{size_t j;Real y;}__={0,opt->start.y()} ; __.j<data->dim()[1] ; __.j++, __.y+=opt->step.y() ) {
						if(frameOnly and __.j!=0 and __.j!=data->dim()[1]-1) continue;
						glVertex3d(opt->start.x(),__.y     ,opt->start.z()); glVertex3d(opt->end  .x(),__.y     ,opt->start.z());
						glVertex3d(opt->start.x(),__.y     ,opt->end  .z()); glVertex3d(opt->end  .x(),__.y     ,opt->end  .z());
						glVertex3d(opt->start.x(),__.y     ,opt->start.z()); glVertex3d(opt->start.x(),__.y     ,opt->end  .z());
						glVertex3d(opt->end  .x(),__.y     ,opt->start.z()); glVertex3d(opt->end  .x(),__.y     ,opt->end  .z());
					}
					for(struct{size_t k;Real z;}___={0,opt->start.z()} ; ___.k<data->dim()[2] ; ___.k++, ___.z+=opt->step.z() ) {
						if(frameOnly and ___.k!=0 and ___.k!=data->dim()[2]-1) continue;
						glVertex3d(opt->start.x(),opt->start.y(),___.z    ); glVertex3d(opt->end  .x(),opt->start.y(),___.z    );
						glVertex3d(opt->start.x(),opt->end  .y(),___.z    ); glVertex3d(opt->end  .x(),opt->end  .y(),___.z    );
						glVertex3d(opt->start.x(),opt->start.y(),___.z    ); glVertex3d(opt->start.x(),opt->end  .y(),___.z    );
						glVertex3d(opt->end  .x(),opt->start.y(),___.z    ); glVertex3d(opt->end  .x(),opt->end  .y(),___.z    );
					}
					glEnd();
					if(timeLimit.tooLong(opt->renderMaxTime)) break;
				}
				if(menuSelection(opt->stepRender)=="mesh")
					if(timeLimit.messageAllowed(10))
						std::cerr << "Drawing mesh inside 3D is not ready yet\n";
			break;
			default:
				if(timeLimit.messageAllowed(5))
					std::cerr << "4D or more dimensions frame plotting is not ready yet\n";
			break;
		}
	}
};

#endif


