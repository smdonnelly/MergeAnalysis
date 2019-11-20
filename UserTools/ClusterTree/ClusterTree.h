#ifndef ClusterTree_H
#define ClusterTree_H

#include <string>
#include <iostream>

#include "Tool.h"
#include "TTree.h"
#include "TFile.h"
#include "TString.h"
/**
 * \class ClusterTree
 *
 * This is a blank template for a Tool used by the script to generate a new custom tool. Please fill out the description and author information.
*
* $Author: B.Richards $
* $Date: 2019/05/28 10:44:00 $
* Contact: b.richards@qmul.ac.uk
*/
class ClusterTree: public Tool {


 public:

  ClusterTree(); ///< Simple constructor
  bool Initialise(std::string configfile,DataModel &data); ///< Initialise Function for setting up Tool resources. @param configfile The path and name of the dynamic configuration file to read in. @param data A reference to the transient data class used to pass information between Tools.
  bool Execute(); ///< Execute function used to perform Tool purpose.
  bool Finalise(); ///< Finalise function used to clean up resources.
  TFile* nottf;
  TTree* fMyTree;   //!pointer to the Tree


  // declare the tree variables

  Int_t         NHits;
  Double_t      hQ[40];  
  Double_t      hT[40];  
  double_t      hxpar[40];
  double_t      hxperp[40];
  double_t      htime[40];
  double_t      hdeltime[40];
  double_t      hvpeak[40];

 private:





};


#endif
