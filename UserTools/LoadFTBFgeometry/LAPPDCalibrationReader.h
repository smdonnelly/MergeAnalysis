#ifndef LAPPDCalibrationReader_H
#define LAPPDCalibrationReader_H

#include <iostream>
#include <string>
#include <vector>

#include "TFile.h"
#include "TTree.h"

using namespace std;

class LAPPDCalibrationReader
{
private:

 unsigned int _board_name; //board "name" number, like 36. metadata really
 unsigned int _board_id; //board number, like 0-3, of ACDC that references the .acdc data file
 unsigned int _channel_id; //channel number on that board
 int _lappd_id;
 int _strip_no;
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
 vector<double>* _sample_times = 0; 
 TTree* _calibtree;
 TString _calibfilename; 
 TFile* _tf;

public:
	LAPPDCalibrationReader(){;}
	LAPPDCalibrationReader(TString calibfilename);
	~LAPPDCalibrationReader();

	void LoadEntry(int e);

	bool GetSingleEnded(){return _single_ended;}
	bool GetIgnoreChannel(){return _ignore_channel;}
	bool GetStripSide(){return _strip_side;}
	bool GetSyncChannel(){return _sync_channel;}
	double GetTransversePosition(){return _transverse_position;}
	double GetParallelPosition(){return _parallel_equidistant_position;}
	unsigned int GetBoardID(){return _board_id;}
	unsigned int GetBoardName(){return _board_name;}
	unsigned int GetChannelID(){return _channel_id;}
	vector<double> GetSampleTimes(){return *(_sample_times);}
	double GetVelocity(){return _velocity;}
	int GetLappdID(){return _lappd_id;}
	int GetStripNo(){return _strip_no;}

	int GetEntries(){return _calibtree->GetEntries();}

	void CloseRootFile();


};

#endif