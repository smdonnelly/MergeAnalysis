/*
 * FTBFLAPPDDisplay.h
 *
 *  Created on: Aril 25, 2019
 *      Author: stenderm
 */
#ifndef SRC_FTBFLAPPDDisplay_H_
#define SRC_FTBFLAPPDDisplay_H_

#include <string>
#include <vector>
#include "TCanvas.h"
#include "TH2D.h"
#include "TApplication.h"
#include "TFile.h"
#include "LAPPDHit.h"
#include "Waveform.h"
#include "FTBFLAPPDDisplay.h"
#include "TRint.h"
#include "TROOT.h"
#include <boost/lexical_cast.hpp>
#include "Position.h"
#include "TMath.h"
#include "Channel.h"
#include "Waveform.h"

class FTBFLAPPDDisplay{
public:
  FTBFLAPPDDisplay(std::string filePath, int confignumber);
  ~FTBFLAPPDDisplay();
  void InitialiseHistoAllLAPPDs(int eventNumber);
  void OpenNewFile(int filenumber);
  void MCTruthDrawing(int eventnumber, unsigned long actualTubeNo, std::vector <MCLAPPDHit> mchits);
  void FinaliseHistoAllLAPPDs();
  void RecoDrawing(int eventCounter, map<unsigned long, Waveform<double>> LAPPDWaves, map<unsigned long, Channel>* geometry_channels_lappds, map<unsigned long, vector<double>>* sample_time_map);
private:
  TApplication* _LAPPD_sim_app;
  TCanvas* _LAPPD_MC_all_canvas;
  TCanvas* _LAPPD_MC_canvas;
  TCanvas* _LAPPD_MC_time_canvas;
  TCanvas* _LAPPD_all_waveforms_canvas;
  TCanvas* _LAPPD_waveform_canvas;
  TH2D* _all_hits;
  TFile* _output_file;
  int _config_number;
  string _output_file_name;
};


#endif /* SRC_FTBFLAPPDDisplay_H_ */
