#ifndef LoadFTBFgeometry_H
#define LoadFTBFgeometry_H

#include <string>
#include <iostream>

#include "Tool.h"
#include "Geometry.h"
#include "TTree.h"


/**
 * \class LoadFTBFgeometry
 *
 * This is a balnk template for a Tool used by the script to generate a new custom tool. Please fill out the descripton and author information.
*
* $Author: B.Richards $
* $Date: 2019/05/28 10:44:00 $
* Contact: b.richards@qmul.ac.uk
*/
class LoadFTBFgeometry: public Tool {


 public:

  LoadFTBFgeometry(); ///< Simple constructor
  bool Initialise(std::string configfile,DataModel &data); ///< Initialise Function for setting up Tool resorces. @param configfile The path and name of the dynamic configuration file to read in. @param data A reference to the transient data class used to pass information between Tools.
  bool Execute(); ///< Executre function used to perform Tool perpose. 
  bool Finalise(); ///< Finalise funciton used to clean up resorces.

  


 private:
 	
 	//ID maps
 	std::map<int,unsigned long> lappd_tubeid_to_detectorkey;
	// inverse
	std::map<unsigned long,int> detectorkey_to_lappdid;

	int verbosity=1;

	TString calibfilename;
	



};


#endif
