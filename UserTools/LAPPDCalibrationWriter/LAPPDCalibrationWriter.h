//This tool is used to write a channel calibration 
//root file for the LAPPD ACDC readout boards. 



#ifndef LAPPDCalibrationWriter_H
#define LAPPDCalibrationWriter_H

#include <string>
#include <iostream>

#include "Tool.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"


/**
 * \class LAPPDCalibrationWriter
 *
 * This is a balnk template for a Tool used by the script to generate a new custom tool. Please fill out the descripton and author information.
*
* $Author: B.Richards $
* $Date: 2019/05/28 10:44:00 $
* Contact: b.richards@qmul.ac.uk
*/
class LAPPDCalibrationWriter: public Tool {


 public:

  LAPPDCalibrationWriter(); ///< Simple constructor
  bool Initialise(std::string configfile,DataModel &data); ///< Initialise Function for setting up Tool resorces. @param configfile The path and name of the dynamic configuration file to read in. @param data A reference to the transient data class used to pass information between Tools.
  bool Execute(); ///< Executre function used to perform Tool perpose. 
  bool Finalise(); ///< Finalise funciton used to clean up resorces.
  void HandEntry();
  void DefaultSimple();
  void FTBFPionRun();


 private:
 	
 
 TTree* _calibtree;
 TString _calibfilename; 


 int _event_iter; //make sure this tool only runs once

 unsigned int _board_name; //board "name" number, like 36. metadata really
 unsigned int _board_id; //board number, like 0-3, of ACDC
 unsigned int _channel_id; //channel number on that board
 int _lappd_id; //number identification of LAPPD, like "tubeno" for geometry class and detector key generation
 int _strip_no; //number id of the particular strip, to be redundant
 bool _ignore_channel; //on or off in "Channel" class
 bool _single_ended; //is this data single ended readout (reflection mode)
 bool _strip_side; //which side of the lappd strip is this channel on. 0 is LHS, 1 is RHS
 bool _sync_channel; //is this a channel used for synchronization of boards
 double _velocity; //average mcp pulse velocity on the stripline in mm/ns 
 //parallel position relative to anode center where 
 //a pulse reaches both ADC channels at the same time. 
 //This quantity IS the entire strip+readout trace+board trace 
 //length calibration for a two sided readout
 double _parallel_equidistant_position; 
 double _transverse_position; //relative to anode center
 //the times of all samples relative to 0th sample.
 //this generalizes to a non-constant sampling rate,
 //calibrating these quantities gives an extra 10ps or so
 //timing resolution of electronics
 vector<double> _sample_times; 
 


};


#endif
