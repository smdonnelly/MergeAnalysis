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
#include "NnlsSolution.h"

class FTBFLAPPDDisplay{
public:
  FTBFLAPPDDisplay(std::string filePath, int confignumber);
  ~FTBFLAPPDDisplay();
  void OpenNewFile(int filenumber);
  void FinaliseHistoAllLAPPDs();
  void PlotRawWaves(int eventCounter, map<unsigned long, Waveform<double>> LAPPDWaves, map<unsigned long, Channel>* geometry_channels_lappds, map<unsigned long, vector<double>>* sample_time_map);
  void PlotNnlsWaves(int eventCounter, map<unsigned long, Waveform<double>> LAPPDWaves, map<unsigned long, Channel>* geometry_channels_lappds, map<unsigned long, vector<double>>* sample_time_map, map<unsigned long, NnlsSolution> nnlssoln);
  void ChiSquaredAnalysis(int eventCounter, map<unsigned long, Waveform<double>> LAPPDWaves, map<unsigned long, vector<double>>* sample_time_map, map<unsigned long, NnlsSolution> nnlssoln);

private:
  TApplication* _LAPPD_sim_app;
  TCanvas* _LAPPD_MC_all_canvas;
  TCanvas* _LAPPD_MC_canvas;
  TCanvas* _LAPPD_MC_time_canvas;
  TCanvas* _LAPPD_all_waveforms_canvas;
  TCanvas* _LAPPD_waveform_canvas;
  TFile* _output_file;
  int _config_number;
  string _output_file_name;
};


#endif /* SRC_FTBFLAPPDDisplay_H_ */
