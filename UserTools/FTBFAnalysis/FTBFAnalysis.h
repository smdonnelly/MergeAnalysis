#ifndef FTBFAnalysis_H
#define FTBFAnalysis_H

#include <string>
#include <iostream>

#include "Tool.h"
#include <TRandom3.h>
#include <TH2.h>
#include <TH1.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TString.h>
#include "FTBFLAPPDDisplay.h"

class FTBFAnalysis: public Tool {


 public:

  FTBFAnalysis();
  bool Initialise(std::string configfile,DataModel &data);
  bool Execute();
  void ChiSquaredAnalysis(int _event_no, map<unsigned long, NnlsSolution> nnlssoln, map<unsigned long, Waveform<double>> LAPPDWaveforms);
  bool Finalise();



 private:
 	int _display_config;
 	int _event_no;
  int _file_number;
  FTBFLAPPDDisplay* _display;
  map<unsigned long, Waveform<double>> LAPPDWaveforms; 
  map<unsigned long, Channel>* all_lappd_channels; //from the geometry class
  map<unsigned long, vector<double>>* sample_time_map; //from calibration file

  TRandom3* myTR;


};


#endif
