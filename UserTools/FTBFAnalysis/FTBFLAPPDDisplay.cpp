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
_LAPPD_MC_time_canvas(nullptr),_LAPPD_all_waveforms_canvas(nullptr),_LAPPD_waveform_canvas(nullptr),_output_file(nullptr),
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
	return;
}

/**
 * Method RecoDrawing:   This method draws the waveforms. One histogram for the left and the right side of each strip:
 *                       Strip number as y-axis and time as x-axis and voltage as colour code.
 * @param eventCounter   Number of the event used for the names of the histograms
 * @param tubeNumber     The detector ID of the LAPPD also used for the names of the histograms.
 * @param waveformVector The vector, from which the waveforms can be retrieved
 */
void FTBFLAPPDDisplay::PlotRawWaves(int eventCounter, map<unsigned long, Waveform<double>> LAPPDWaves, map<unsigned long, Channel>* geometry_channels_lappds, map<unsigned long, vector<double>>* sample_time_map)
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



void FTBFLAPPDDisplay::PlotNnlsWaves(int eventCounter, map<unsigned long, Waveform<double>> LAPPDWaves, map<unsigned long, Channel>* geometry_channels_lappds, map<unsigned long, vector<double>>* sample_time_map, map<unsigned long, NnlsSolution> nnlssoln)
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
		NnlsSolution* this_nnls_soln = new NnlsSolution;
		this_nnls_soln = &(nnlssoln.at(data_ch));
		Waveform<double>* nnls_wave = new Waveform<double>;
		nnls_wave = this_nnls_soln->GetFullSoln();//full nnls solution waveform

		//Initialisation of the histograms
		std::string chanNumber = boost::lexical_cast < std::string > (data_ch);
		std::string boardNumber = boost::lexical_cast < std::string > (this_ch.GetSignalCard());
		std::string histname; 
		if(this_ch.GetStripSide() == 1) histname = "event" + eventnumber + "board" + boardNumber + "chan" + chanNumber + "right";
		else histname = "event" + eventnumber + "board" + boardNumber + "chan" + chanNumber + "left";

		std::string nnlsname = histname+"_nnls";
		vector<double> these_times = sample_time_map->at(data_ch);
		vector<double> nnls_times = nnlssoln.at(data_ch).GetFullSolutionTimes();
		int nbins = these_times.size();
		double min_time = these_times.front();
		double max_time = these_times.back();

		int nbins_nnls = nnls_wave->GetSamples()->size(); 
		TH1D* waveformhist = new TH1D(histname.c_str(), histname.c_str(), nbins, min_time-0.5, max_time+0.5);
		TH1D* nnlshist = new TH1D(nnlsname.c_str(), nnlsname.c_str(), nbins_nnls, min_time-0.5, max_time+0.5); //assumes nnls wave is within time bounds

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

		//fill the nnls histogram
		for(int samp = 0; samp < (int)(nnls_wave->GetSamples()->size()); samp++)
		{
			nnlshist->Fill(nnls_times.at(samp), nnls_wave->GetSample(samp)); //individual hist
		}
		//Cosmetics
		waveformhist->GetXaxis()->SetTitle("Time [ps]");
		waveformhist->GetYaxis()->SetTitle("Voltage [mV]");
		waveformhist->GetYaxis()->SetTitleOffset(1.4);
		waveformhist->Write();

		nnlshist->GetXaxis()->SetTitle("Time [ps]");
		nnlshist->GetYaxis()->SetTitle("Voltage [mV]");
		nnlshist->GetYaxis()->SetTitleOffset(1.4);
		nnlshist->Write();


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
	heatmapHist->GetXaxis()->SetTitle("Time [ps]");
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


void FTBFLAPPDDisplay::ChiSquaredAnalysis(int eventCounter, map<unsigned long, Waveform<double>> LAPPDWaves, map<unsigned long, vector<double>>* sample_time_map, map<unsigned long, NnlsSolution> nnlssoln)
{
	_output_file->cd();
  double sampling_noise = 1.0; //mV constant noise

  //histogram to save all of the ChiSq/ndfs
  string evno = boost::lexical_cast < std::string > (eventCounter);
  string histname = "all_chisq";
  //check if the histogram already exists. 
  //we want to pile all events' chisq on the same
  //hist. 
  TH1D* all_chisq;
  if(_output_file->GetListOfKeys()->Contains(histname.c_str()))
  {
  	all_chisq = (TH1D*)_output_file->Get(histname.c_str());
  }
  else
  {
  	all_chisq = new TH1D(histname.c_str(), histname.c_str(), 100, 50, 1000);
  }

  map<unsigned long, Waveform<double>>::iterator wavit;
  for(wavit = LAPPDWaves.begin(); wavit != LAPPDWaves.end(); ++wavit)
  {
    unsigned long ch = wavit->first;
    Waveform<double> thewav = wavit->second;
    vector<double> thetimes = sample_time_map->at(ch);
    NnlsSolution soln = nnlssoln.at(ch);
    vector<double> nnlstimes = soln.GetFullSolutionTimes();
    Waveform<double> nnlswav = *(soln.GetFullSoln());
    int ndf = 0; //number of degrees of freedom
    double chisq = 0; //chi squared

    //loop through nnls wave and find the value
    //of the raw wave at each time. 
    for(int ti = 0; ti < (int)nnlstimes.size(); ti++)
    {
    	double t = nnlstimes.at(ti);
    	double nnlsvalue = nnlswav.GetSample(ti);
    	double t0, t1;
    	double rawvalue;
    	bool found = false;
    	ndf++;

    	for(int i = 0; i < (int)thetimes.size() - 1; i++)
		{
			t0 = thetimes.at(i);
			t1 = thetimes.at(i+1);
			if(t >= t0 and t < t1)
			{
				rawvalue = thewav.GetSample(i);
				found = true;
				break;
			}
		}

		if(found)
		{
			//add to ChiSq:
			//find difference squared between
			//fit and the raw wav. If the raw wav
			//is positive valued, it isn't considered
			//by the nnls. I would still call a degree of
			//freedom, but take difference of nnls w.r.t. 0. 
			if(rawvalue >= 0)
			{
				chisq += (nnlsvalue)*(nnlsvalue)/(sampling_noise*sampling_noise);
			}
			else
			{
				chisq += (rawvalue - nnlsvalue)*(rawvalue - nnlsvalue)/(sampling_noise*sampling_noise);
			}

		}
    }

    //fill the hist with this chisq value
    //all_chisq->Fill(chisq/(double)ndf);
    all_chisq->Fill(chisq);
    cout << "ndf is " << ndf << endl;
    //reinitialize
    ndf = 0;
    chisq = 0;
  }
 
	
	all_chisq->GetXaxis()->SetTitle("ChiSq/NDF");
	all_chisq->GetYaxis()->SetTitle("n waveforms");
	all_chisq->GetYaxis()->SetTitleOffset(1.4);
	all_chisq->Write();

}


