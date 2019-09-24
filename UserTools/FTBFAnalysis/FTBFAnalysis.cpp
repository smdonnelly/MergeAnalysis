#include "FTBFAnalysis.h"

using namespace std;

FTBFAnalysis::FTBFAnalysis():Tool(),_event_no(0),_file_number(0),_display_config(0),_display(nullptr){}


bool FTBFAnalysis::Initialise(std::string configfile, DataModel &data){

  /////////////////// Usefull header ///////////////////////
  if(configfile!="")  m_variables.Initialise(configfile); //loading config file
  //m_variables.Print();

  m_data= &data; //assigning transient data pointer
  /////////////////////////////////////////////////////////////////

  // setup the output files
  std::string OutFile = "lappdout.root";	 //default input file
  m_variables.Get("OutputFile", OutFile);
  OutFile.erase(OutFile.size()-5);
  OutFile = OutFile + "00.root";
  cout << "FTBF filename: " << OutFile << endl;

  //not sure why. got from Malte's LAPPDSim
  myTR = new TRandom3();

  m_variables.Get("EventDisplay", _display_config);

  if (_display_config > 0)
  {
        _display = new FTBFLAPPDDisplay(OutFile, _display_config);
  }

_event_no = 0;
  return true;
}


bool FTBFAnalysis::Execute(){
  cout << "Executing analysis " << endl;
  

  string storename; 
  m_variables.Get("store_name", storename);
  m_data->Stores.at(storename)->Get("AllLAPPDChannels", all_lappd_channels);
  m_data->Stores.at(storename)->Get("LAPPDWaveforms", LAPPDWaveforms);
  m_data->Stores.at(storename)->Get("sample_time_map", sample_time_map);

  int nnls_analysis;
  m_variables.Get("nnls_analysis", nnls_analysis); // if you want to plot nnls solutions

    
    //The files become too large, if one tries to save all WCSim events into one file.
    //Every 100 events get a new file.
    if(_event_no == (20 * (_file_number + 1)))
    {
        _display->OpenNewFile(_file_number);
        _file_number++;
    }

    
    
    
    if(nnls_analysis == 1)
    {
      map<unsigned long, NnlsSolution> nnlssoln;
      m_data->Stores.at(storename)->Get("nnls_solution", nnlssoln);
      _display->PlotNnlsWaves(_event_no, LAPPDWaveforms, all_lappd_channels, sample_time_map, nnlssoln);
      _display->ChiSquaredAnalysis(_event_no, LAPPDWaveforms, sample_time_map, nnlssoln);
    } 
    else
    {
      _display->PlotRawWaves(_event_no, LAPPDWaveforms, all_lappd_channels, sample_time_map);

    }

    if (_display_config > 0)
    {
        _display->FinaliseHistoAllLAPPDs();
    }

  
    _event_no++;
    return true;

}



bool FTBFAnalysis::Finalise()
{
  return true;
}
