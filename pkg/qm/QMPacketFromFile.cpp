// 2017 © Janek Kozicki <cosurgi@gmail.com>

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <iterator>

#include "QMPacketFromFile.hpp"

#include <lib/time/TimeLimit.hpp>
#include <lib/smoothing/Spline6Interpolate.hpp>
//#include <lib/smoothing/Sinc16Interpolate.hpp>


// HUGE FIXME - po co dublować operację ładowania z pliku ??????????????
TimeLimit timeLimitGF; // FIXME - remove when finshed fixing

YADE_PLUGIN(
	(QMPacketFromFile)
	(St1_QMPacketFromFile)
	);

/*********************************************************************************
*
* L O A D   P A C K E T   S H A P E   F R O M   F I L E
*
*********************************************************************************/
CREATE_LOGGER(QMPacketFromFile);
// !! at least one virtual function in the .cpp file
QMPacketFromFile::~QMPacketFromFile(){};

void QMPacketFromFile::readFileInLevelFormat() {
	std::ifstream file;
	std::string line;
	std::vector<Real> row;
	std::cerr << "Reading potential data from file: \""<< filename << "\"\n";
	std::vector<std::vector<Real> > fileData;
	file.open(filename);
	int file_line=0;
	bool should_read_now=false;
	bool found_vib=false;
	fileDataX.clear();
	fileDataVal.clear();
	getline (file, line); file_line++;
	if(line.size() != 0) {
		std::cerr << "expected empty line here, the line number is: " << file_line << "\n";
		exit(0);
	};
	if(file.is_open()) {
		while (getline (file, line)) {
			file_line++;
			int vibrational=-1;
			Real energy_read=0;
			//getline (file, line); // while does this.
			std::istringstream iss(line);
			std::vector<string> tokens;
			std::copy(std::istream_iterator<string>(iss),std::istream_iterator<string>(),std::back_inserter(tokens));
			try {
				vibrational=boost::lexical_cast<int>(tokens[2]);
				energy_read=boost::lexical_cast<Real>(tokens[6]);
			}
			catch(const boost::bad_lexical_cast &) {
				std::cerr << "line: '"<<line<<"'\ncannot read vibrational level: '"<<tokens[2]<<"' or energy: '"<< tokens[6] <<"', the line number is: " << file_line << "\n";
				exit(0);
			};
			std::cerr << "Found vibrational level " << vibrational << " with energy= " << energy_read << "\n";
			if(vibrational == fileLevelVibrational) {
				std::cerr << "reading it\n";
				should_read_now=true;
			} else {
				std::cerr << "skipping it\n";
				should_read_now=false;
			};
			getline (file, line); file_line++;
			std::cerr << "Skipping useless line: '" <<line<<"'\n";

			getline (file, line); file_line++;
			while(line.size() != 0) {
				tokens.clear();
				std::istringstream iss(line);
				std::copy(std::istream_iterator<string>(iss),std::istream_iterator<string>(),std::back_inserter(tokens));
				if(tokens.size() % 2 != 0) {
					std::cerr << "Error: expected even number of columns in input line "<< file_line <<": '"<<line<<"'\n";
					exit(0);
				}
				for(int i=0 ; i < tokens.size() ; i+=2 ) {
					if(should_read_now) {
						try {
							fileDataX  .push_back( boost::lexical_cast<Real>(tokens[i  ])*multX + shiftX   );
							fileDataVal.push_back( boost::lexical_cast<Real>(tokens[i+1]) + shiftVal );
							found_vib=true;
						} catch(boost::bad_lexical_cast &) {
							std::cerr << "line: '"<<line<<"'\ncannot read wavefunction data x='"<<tokens[i]<<"' value='"<< tokens[i+1] <<"', the line number is: " << file_line << "\n";
							exit(0);
						}
					}
				}
				getline (file, line); file_line++;
			}
			std::cerr << file_line << "\n";
		}
		file.close();
	} else {
		std::cerr << "ERROR: can't read file with this name: \"" << filename << "\"\n";
		exit(1);
	}
	if(not found_vib) {
		std::cerr << "ERROR: can't find required vibrational level: " << fileLevelVibrational << "\n";
		exit(1);
	}
};

void QMPacketFromFile::readFileInStandardFormat() {
	std::ifstream file;
	std::string line;
	std::vector<Real> row;
	std::cerr << "Reading potential data from file: \""<< filename << "\"\n";
	std::vector<std::vector<Real> > fileData;
	file.open(filename);
	if(file.is_open()) {
		while (getline (file, line)) {
			if(line.size() == 0) continue;
			size_t pos=0;
			while( (pos+1) < line.size() and line[pos]==' ') { pos++; }; // skip ' ' at start of the line
			if(line[pos] == '#') continue;                               // skip commented lines
			std::stringstream ss(line);
			while(not ss.eof()) {
				Real val=0;
				ss >> val;
				// if(not ss.eof())   // didn't work: it skipped adding numbers if there wasn't en extra empty character after last number
				//                       so instead we have an extra column with '0'
				row.push_back(val);
			}
			fileData.push_back(row);
			row.clear();
			//cout << line << endl;
		}
		file.close();
	} else {
		std::cerr << "ERROR: can't read file with this name: \"" << filename << "\"\n";
		exit(1);
	}
// // print fileData for debugging ...
//	for(size_t i=0 ; i < fileData.size() ; i++) {
//		for(size_t j=0 ; j < fileData[i].size() ; j++) {
//			std::cout << fileData[i][j] << " ";
//		}
//		std::cout << "\n";
//	}

	// now extract the interesting X,val information
	fileDataX.clear();
	fileDataVal.clear();
	int  _col_x  = columnX   -1;
	int  _val_y  = columnVal -1;
	for(size_t i=0 ; i < fileData.size() ; i++) {
		if(_col_x >= 0 and _col_x < (int)fileData[i].size()  and  _val_y >=0 and _val_y < (int)fileData[i].size() ) {
			fileDataX  .push_back( fileData[i][_col_x]*multX + shiftX   );
			fileDataVal.push_back( fileData[i][_val_y] + shiftVal );
		} else {
			std::cerr << "ERROR 2 in  QMPacketFromFile::readFileIfDidnt    columnX   = " << _col_x+1 << " is wrong\n";
			std::cerr << "        or  QMPacketFromFile::readFileIfDidnt    columnVal = " << _val_y+1 << " is wrong.\n";
		}
	}
};

void QMPacketFromFile::readFileIfDidnt() {
	if(fileLoaded) return;
	std::cerr << "\nQMPacketFromFile::readFileIfDidnt\n";
	std::cerr << "  fileLoaded           = " << fileLoaded << "\n";
	std::cerr << "  filename             = " << filename   << "\n";
	std::cerr << "  columnX              = " << columnX    << "\n";
	std::cerr << "  shiftX               = " << shiftX     << "\n";
	std::cerr << "  columnVal            = " << columnVal  << "\n";
	std::cerr << "  shiftVal             = " << shiftVal   << "\n";
	std::cerr << "  fileLevelFormat      = " << fileLevelFormat << "\n";
	std::cerr << "  fileLevelVibrational = " << fileLevelVibrational << "\n";
	if(filename == "" ) {
		std::cerr << "WARNING: can't read file with this EMPTY name: \"" << filename << "\" doing nothing.\n";
		return;
	};
	if(fileLevelFormat) {
		readFileInLevelFormat();
	} else {
		readFileInStandardFormat();
	}
	fileLoaded=true;
	std::cerr << "Loading file "<< filename <<" finished\n";


	if(expandRightDecay) {
		Real decayFact = 0;
		Real delta_x   = 0;
		int  count     = 0;
		bool must_zero = false;
		for(int i=fileDataVal.size()-10 ; i<fileDataVal.size()-1 ; i++) {
			if(fileDataVal[i+1] != 0)
				decayFact += fileDataVal[i]/fileDataVal[i+1];
			else
				must_zero = true;
			delta_x   += std::abs( fileDataX[i] - fileDataX[i+1] );
			count++;
		}
		decayFact /= count;
		if(must_zero)
			decayFact = 1e5;
		delta_x   /= count;
		std::cerr << "decay expand, using " << count << " points, delta x = " << delta_x << " decay factor = " << decayFact << "\n";
		for(int i=0 ; i<expandRightDecayPoints ; i++) {
			fileDataX.push_back  ( (*fileDataX  .rbegin())+delta_x  );
			fileDataVal.push_back( (*fileDataVal.rbegin())/decayFact);
		}
		// add zero after adding all those points
		fileDataX.push_back  ( (*fileDataX  .rbegin())+delta_x  );
		fileDataVal.push_back( 0                                );
	}

	if(normalizeWaveFunction) {
		std::cerr << "Normalizing wavefunction, using " << gridSize << " points\n";
		// FIXME - nieco naiwne całkowanie:
		Real sum_all = 0; // Real sum_all_sqrt = std::sqrt(std::accumulate(fileDataVal.begin(), fileDataVal.end(), 0))*;
		for(int i = 0 ; i<fileDataVal.size() ; i++) {
			sum_all += std::pow(fileDataVal[i],2.0)*std::abs(fileDataX[i]-fileDataX[i + ( i==0 ? 1 : -1 )  ]);
		}
		Real sum_all_sqrt = std::sqrt(sum_all);
		for(Real& val : fileDataVal) {
			val/=sum_all_sqrt;
		}
	}
};

/*********************************************************************************
*
* L O A D   P A C K E T   S H A P E   F R O M   F I L E   F U N C T O R 
*
*********************************************************************************/

Complexr St1_QMPacketFromFile::waveFunctionValue_1D_positionRepresentation(Vector3r pos , const QMParameters* pm, const QMState* qms )
{
	if(timeLimitGF.messageAllowed(12)) std::cerr << "UWAGA: analityczny pakiet ciągle 'rekonstruuje' postać, to bez sensu!! St1_QMPacketFromFile::waveFunctionValue_1D_positionRepresentation,  QMParameters is " << std::string(pm?pm->getClassName():"") << " , QMState is " << std::string(qms?qms->getClassName():"") << "\n";
	//const QMStPotentialFromFile*        state = static_cast <const QMStPotentialFromFile*>(qms);
	QMPacketFromFile* barrier = const_cast<QMPacketFromFile*>(dynamic_cast<const QMPacketFromFile*>(qms));
	if(not barrier) { throw std::runtime_error("\n\nERROR: St1_QMPacketFromFile::getValPos() nas no QMPacketFromFile, but rather `"
		+std::string(qms?qms->getClassName():"")+"`.\n\n");};

//	std::cerr << barrier->fileData.size() << "," << barrier->filename << "," << barrier->columnX << "," << barrier->columnVal <<"\n";

	barrier->readFileIfDidnt();

	Real x   = pos[0]                  ,y =pos[1]                  ,z =pos[2];
	// to teraz interpolujemy
	int idx_row = 0;
	while(  (idx_row < (int)barrier->fileDataX.size())  and  (barrier->fileDataX[idx_row] < x) ) { idx_row++; };
	if(idx_row >=  (int)barrier->fileDataX.size() ) idx_row = barrier->fileDataX.size()-1;
	if((idx_row < 0) or (idx_row >= (int)barrier->fileDataX.size() ) ) {
		std::cerr << "ERROR 1 in St1_QMPacketFromFile::getValPos     ( idx_row < 0 ) or  ( idx_row >= barrier->fileData.size() )\n";
		exit(1);
	}

	if(idx_row == (int)(barrier->fileDataX.size()-1) ) return barrier->fileDataVal[idx_row]; // after end of potential
	if(idx_row == 0                                  ) return barrier->fileDataVal[idx_row]; // before start of potential

	Real x_before  = barrier->fileDataX[idx_row-1];
	Real x_after   = barrier->fileDataX[idx_row  ];
	if(not( x >= x_before and x <= x_after)) {
		std::cerr << "ERROR 2  nonsense error  not( x > x_before and x < x_after)\n";
		exit(1);
	}
	// row 'between lines' that will be used for interpolating the value
	Real x_idx_row = (x - x_before)/(x_after - x_before) + idx_row -1;

	if(x_idx_row >= 3 and x_idx_row <  barrier->fileDataX.size()-3) { // Spline6Interpolate is possible
		return spline6InterpolatePoint1D<Real>(barrier->fileDataVal,x_idx_row);
		//return sinc16InterpolatePoint1D <Real>(barrier->fileDataVal,x_idx_row);
	} else { // must use linear interpolation
		// return barrier->fileDataVal[idx_row];//barrier->height;                    // to jest linijka która daje wynik bez interpolowania.
		Real val_before = barrier->fileDataVal[idx_row-1];
		Real val_after  = barrier->fileDataVal[idx_row  ];
		Real t = (x - x_before)/(x_after - x_before);
		// return linearInterpolatePoint1D<Real>(barrier->fileDataVal,x_idx_row);	// FIXME wrzucić to do  linearInterpolatePoint1D in yade/lib/smoothing/
		return (1-t)*val_before + t*val_after;
	}
};
		
Complexr St1_QMPacketFromFile::getValPos(Vector3r pos , const QMParameters* pm, const QMState* qms)
{
	//std::cerr << "St1_QMPacketFromFile::getValPos\n";
	const QMPacketFromFile* p = dynamic_cast<const QMPacketFromFile*>(qms);
	if(not p) { throw std::runtime_error("\n\nERROR: St1_QMPacketFromFile missing QMPacketFromFile"); };
	const QMParticle* par = dynamic_cast<const QMParticle*>(pm);
	if(not par) { throw std::runtime_error("\n\nERROR: St1_QMPacketFromFile nas no QMParticle, but rather `"
		+std::string(pm?pm->getClassName():"")+"`\n\n");};
//std::cerr << "renderuję dla t = " << (p->t-p->t0) << "\n";
	switch(par->dim) {
		case 1 : return waveFunctionValue_1D_positionRepresentation( pos , pm, qms );

		default: break;
	}
	throw std::runtime_error("\n\nSt1_QMPacketFromFile::getValPos() works only in 1 dimension.\n\n");
};

