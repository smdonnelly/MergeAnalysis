#ifndef EstimateNPE_H
#define EstimateNPE_H

#include <string>
#include <iostream>

#include "Tool.h"


/**
 * \class EstimateNPE
 *
 * This is a blank template for a Tool used by the script to generate a new custom tool. Please fill out the description and author information.
*
* $Author: B.Richards $
* $Date: 2019/05/28 10:44:00 $
* Contact: b.richards@qmul.ac.uk
*/
class EstimateNPE: public Tool {


 public:

  EstimateNPE(); ///< Simple constructor
  bool Initialise(std::string configfile,DataModel &data); ///< Initialise Function for setting up Tool resources. @param configfile The path and name of the dynamic configuration file to read in. @param data A reference to the transient data class used to pass information between Tools.
  bool Execute(); ///< Execute function used to perform Tool purpose.
  bool Finalise(); ///< Finalise function used to clean up resources.


 private:
 	string storename;
 	string spectrum_filename; //root file containing branches with SPE spectrum and background spectrum
 	vector<double> spe_spectrum;
 	double threshold_fraction; //f from saldanha
 	





};


#endif
