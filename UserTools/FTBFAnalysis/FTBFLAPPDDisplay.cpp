/*
 * FTBFLAPPDDisplay.cpp
 *
 *  Created on: April 25, 2019
 *      Author: stenderm
 */

#include "FTBFLAPPDDisplay.h"
#include <thread>

/**
 * Constructor FTBFLAPPDDisplay: Initialises the class with a TApplication, the canvases and the output file.
 * @param filePath           Path and name of the output root file.
 */
FTBFLAPPDDisplay::FTBFLAPPDDisplay(std::string filePath, int confignumber):_LAPPD_sim_app(nullptr),_LAPPD_MC_all_canvas(nullptr),_LAPPD_MC_canvas(nullptr),
_LAPPD_MC_time_canvas(nullptr),_LAPPD_all_waveforms_canvas(nullptr),_LAPPD_waveform_canvas(nullptr),_all_hits(nullptr),_output_file(nullptr),
_output_file_name(filePath),_config_number(confignumber)
{
  //TApplication
	int myargc = 0;
	char *myargv[] =
	{ (const char*) "somestring" };
	_LAPPD_sim_app = new TApplication("LAPPDSimApp", &myargc, myargv);
  //Canvases only need to be created, if one wants to display the plots while the program is running
	if (_config_number == 2)
	{
    //size of all canvases is 1280x720
		Double_t canvwidth = 1280;
		Double_t canvheight = 720;

    //Canvas for the histogram of the MC hits of all LAPPDs in one plot
		_LAPPD_MC_all_canvas = new TCanvas("AllLAPPDMCCanvas", "All LAPPDs at once", canvwidth, canvheight);

    //Canvas for the histogram: transverse versus parallel coordinate with arrival time as colour code for each LAPPD
		_LAPPD_MC_canvas = new TCanvas("LAPPDMCCanvas", "2D temperature for one LAPPD", canvwidth, canvheight);
		_LAPPD_MC_canvas->SetRightMargin(0.15);

    //Canvas for the histograms: transverse coordinate versus time and parallel coordinate versus time with number of events as colour code
		_LAPPD_MC_time_canvas = new TCanvas("LAPPDMCTimeCanvas", "LAPPD time versus parallel and transvere coordinate", canvwidth, canvheight);
		_LAPPD_MC_time_canvas->Divide(2, 1, 0.01, 0.01, 0);
    _LAPPD_MC_time_canvas->GetPad(1)->SetRightMargin(0.15);
    _LAPPD_MC_time_canvas->GetPad(2)->SetRightMargin(0.15);

    //Canvas for all waveforms in one plot: Strip number versus time with voltage as colour code
		_LAPPD_all_waveforms_canvas = new TCanvas("LAPPDAllWaveformCanvas", "All waveforms", canvwidth, canvheight);
		_LAPPD_all_waveforms_canvas->Divide(2, 1, 0.015, 0.01, 0);
		_LAPPD_all_waveforms_canvas->GetPad(1)->SetRightMargin(0.15);
		_LAPPD_all_waveforms_canvas->GetPad(2)->SetRightMargin(0.15);

    //Canvas for single waveforms: Voltage versus time
    _LAPPD_waveform_canvas = new TCanvas("LAPPDWaveformCanvas", "One waveform", canvwidth, canvheight);
    _LAPPD_waveform_canvas->Divide(2, 1, 0.015, 0.01, 0);
	}

  //initial opening of the output file
  const char *filename = filePath.c_str();
	_output_file = new TFile(filename, "Recreate");
}

/**
 * Destructor FTBFLAPPDDisplay: Closes the output file, deletes the canvas and the TApplication.
 */

FTBFLAPPDDisplay::~FTBFLAPPDDisplay()
{
	if (_config_number == 2)
	{
		if (gROOT->FindObject("LAPPDSimCanvas") != nullptr)
		{
			delete _LAPPD_MC_time_canvas;
		}
		if (gROOT->FindObject("AllLAPPDMCCanvas") != nullptr)
		{
			delete _LAPPD_MC_all_canvas;
		}
		if (gROOT->FindObject("LAPPDMCCanvas") != nullptr)
		{
			delete _LAPPD_MC_canvas;
		}
		if (gROOT->FindObject("LAPPDAllWaveformCanvas") != nullptr)
		{
			delete _LAPPD_all_waveforms_canvas;
		}
    if (gROOT->FindObject("LAPPDWaveformCanvas") != nullptr)
    {
      delete _LAPPD_waveform_canvas;
    }
	}
	delete _LAPPD_sim_app;
	_output_file->Close();
	delete _output_file;
	_output_file = nullptr;
}

/**
 * [FTBFLAPPDDisplay::InitialiseHistoAllLAPPDs description]
 * @param eventNumber [description]
 */
void FTBFLAPPDDisplay::InitialiseHistoAllLAPPDs(int eventNumber){
	//Create the name for the histogram for all LAPPDs
	std::string eventnumber = boost::lexical_cast < std::string > (eventNumber);
	string allHitsName = "event" + eventnumber + "AllLAPPDs";
	const char *allHitsNamec = allHitsName.c_str();
	//Initialisation of the histogram for all LAPPDs
	_all_hits = new TH2D(allHitsNamec, allHitsNamec, 200, 0, 180, 200, -1.1, 1.1);
}

/**
 * Method OpenNewFile: This method is needed to avoid the creation of too big .root files.
 *                     The splitting can be adjusted in the LAPPDSim tool.
 * @param filenumber: The number of the current file
 */
void FTBFLAPPDDisplay::OpenNewFile(int filenumber){

  //Close the original file
  _output_file->Close();
  delete _output_file;
  _output_file = nullptr;
  std::string number = boost::lexical_cast < std::string > (filenumber);

  //adjust the name for sensible numbering
  _output_file_name.erase(_output_file_name.size()-7);
  if(filenumber < 10)
  {
    _output_file_name = _output_file_name + "0" + number + ".root";
  }
  else
  {
    _output_file_name = _output_file_name + number + ".root";
  }
  const char *filename = _output_file_name.c_str();
  _output_file = new TFile(filename, "Recreate");
}


/**
 * Method FinaliseHistoAllLAPPDs: Cosmetics and drawing of the histogram for all LAPPDs
 */
void FTBFLAPPDDisplay::FinaliseHistoAllLAPPDs(){
  //Cosmetics
	_output_file->cd();
	_all_hits->GetXaxis()->SetTitle("Radius [m]");
	_all_hits->GetYaxis()->SetTitle("Height [m]");
	_all_hits->GetZaxis()->SetTitle("Arrival time [ns]");
	_all_hits->GetZaxis()->SetTitleOffset(1.4);
	_all_hits->Write();
  //Canvas adjustments
	if(_config_number == 2)
	{
	_LAPPD_MC_all_canvas->cd();
	_all_hits->SetStats(0);
	_all_hits->Draw("COLZ");
	_LAPPD_MC_all_canvas->Modified();
	_LAPPD_MC_all_canvas->Update();
	}
	_all_hits->Clear();
}

/**
 * Method RecoDrawing:   This method draws the waveforms. One histogram for the left and the right side of each strip:
 *                       Strip number as y-axis and time as x-axis and voltage as colour code.
 * @param eventCounter   Number of the event used for the names of the histograms
 * @param tubeNumber     The detector ID of the LAPPD also used for the names of the histograms.
 * @param waveformVector The vector, from which the waveforms can be retrieved
 */
void FTBFLAPPDDisplay::RecoDrawing(int eventCounter, map<unsigned long, Waveform<double>> LAPPDWaves, map<unsigned long, Channel>* geometry_channels_lappds, map<unsigned long, vector<double>>* sample_time_map)
{
  //Creation of the histogram names
	_output_file->cd();

	std::string eventnumber = boost::lexical_cast < std::string > (eventCounter);

	//heatmap histogram just has all of the active channels as the y axis. 
	std::string heatmapName = "eventheatmap" + eventnumber;

	//want to bin the heatmap based on 
	//the times and channel numbers of this entire event
	double maxtime, mintime;
	double maxchan, minchan;
	maxtime = sample_time_map->at(0).back(); //just to initialize
	mintime = sample_time_map->at(0).front();
	maxchan = LAPPDWaves.rbegin()->first; 
	minchan = LAPPDWaves.rbegin()->first; 
	map<unsigned long, Waveform<double>>::iterator waveit;
	for(waveit = LAPPDWaves.begin(); waveit != LAPPDWaves.end(); ++waveit)
	{
		unsigned long thisch = waveit->first;
		if(thisch > maxchan) maxchan = thisch;
		if(thisch < minchan) minchan = thisch;
	}
	//loop to find max time
	map<unsigned long, vector<double>>::iterator timeit;
	for(timeit = sample_time_map->begin(); timeit != sample_time_map->end(); ++timeit)
	{
		vector<double> thistimes = timeit->second;
		double thismax = *max_element(thistimes.begin(), thistimes.end());
		double thismin = *min_element(thistimes.begin(), thistimes.end());
		if(thismax > maxtime) maxtime = thismax;
		if(thismin < mintime) mintime = thismin;
	}

	TH2D* heatmapHist = new TH2D(heatmapName.c_str(), heatmapName.c_str(), 256, mintime-0.5, maxtime+0.5, int(maxchan-minchan+1), minchan-0.5, maxchan+0.5);

	//start filling histograms
	map<unsigned long, Waveform<double>> ::iterator it_waves;
	for(it_waves = LAPPDWaves.begin(); it_waves != LAPPDWaves.end(); ++it_waves)
	{
		unsigned long data_ch = it_waves->first;
		Channel this_ch = geometry_channels_lappds->at(data_ch);
		//Initialisation of the histograms
		std::string chanNumber = boost::lexical_cast < std::string > (data_ch);
		std::string boardNumber = boost::lexical_cast < std::string > (this_ch.GetSignalCard());
		std::string histname; 
		if(this_ch.GetStripSide() == 1) histname = "event" + eventnumber + "board" + boardNumber + "chan" + chanNumber + "right";
		else histname = "event" + eventnumber + "board" + boardNumber + "chan" + chanNumber + "left";

		vector<double> these_times = sample_time_map->at(data_ch);
		int nbins = these_times.size();
		double min_time = these_times.front();
		double max_time = these_times.back();
		TH1D* waveformhist = new TH1D(histname.c_str(), histname.c_str(), nbins, min_time-0.5, max_time+0.5);

		//fill the histogram
		Waveform<double> this_wave = it_waves->second;
		for(int samp = 0; samp < (int)(this_wave.GetSamples()->size()); samp++)
		{
			waveformhist->Fill(these_times.at(samp), this_wave.GetSample(samp)); //individual hist
			//clip the 2D display at 250 mV
			if(this_wave.GetSample(samp) > 100) heatmapHist->Fill(these_times.at(samp), data_ch, 100); 
			else if(this_wave.GetSample(samp) < -250) heatmapHist->Fill(these_times.at(samp), data_ch, -250); 
			else heatmapHist->Fill(these_times.at(samp), data_ch, this_wave.GetSample(samp)); 
			
		}
		//Cosmetics
		waveformhist->GetXaxis()->SetTitle("Time [ns]");
		waveformhist->GetYaxis()->SetTitle("Voltage [mV]");
		waveformhist->GetYaxis()->SetTitleOffset(1.4);
		waveformhist->Write();


	    //Canvas adjustments
	    if(_config_number == 2){
	      _LAPPD_waveform_canvas->cd(1);
	      waveformhist->SetStats(0);
	      waveformhist->Draw("HIST");
	      _LAPPD_waveform_canvas->Modified();
	    	_LAPPD_waveform_canvas->Update();
	      }
	}

  //Cosmetics
	heatmapHist->GetXaxis()->SetTitle("Time [ns]");
	heatmapHist->GetYaxis()->SetTitle("Chan number");
	heatmapHist->GetZaxis()->SetTitle("Voltage [mV]");
	//leftAllWaveforms->GetZaxis()->SetTitleOffset(1.4);
	heatmapHist->Write();


  //Canvas adjustments
	if(_config_number == 2)
	{
	_LAPPD_all_waveforms_canvas->cd(1);
	heatmapHist->SetStats(0);
	heatmapHist->Draw("COLZ");
	_LAPPD_all_waveforms_canvas->Modified();
	_LAPPD_all_waveforms_canvas->Update();
	}

	heatmapHist->Clear();
    

}
