#include "LAPPDCalibrationWriter.h"

LAPPDCalibrationWriter::LAPPDCalibrationWriter():Tool(){}


bool LAPPDCalibrationWriter::Initialise(std::string configfile, DataModel &data){

  /////////////////// Usefull header ///////////////////////
  if(configfile!="")  m_variables.Initialise(configfile); //loading config file
  //m_variables.Print();

  m_data= &data; //assigning transient data pointer
  /////////////////////////////////////////////////////////////////
  _event_iter = -1;
  

  m_variables.Get("ChannelConfigFile", _calibfilename);

  
  _calibtree = new TTree("acdc_calib", "acdc_calib");
  _calibtree->Branch("board_name",&_board_name, "_board_name/i");
  _calibtree->Branch("board_id",&_board_id, "_board_id/i");
  _calibtree->Branch("channel_id",&_channel_id, "_channel_id/i");
  _calibtree->Branch("ignore_channel",&_ignore_channel, "_ignore_channel/O");
  _calibtree->Branch("single_ended",&_single_ended, "_single_ended/O");
  _calibtree->Branch("strip_side",&_strip_side, "_strip_side/O");
  _calibtree->Branch("sync_channel",&_sync_channel, "_sync_channel/O");
  _calibtree->Branch("velocity",&_velocity, "_velocity/D");
  _calibtree->Branch("parallel_equidistant_position",&_parallel_equidistant_position, "_parallel_equidistant_position/D");
  _calibtree->Branch("transverse_position",&_transverse_position, "_transverse_position/D");
  _calibtree->Branch("sample_times",&_sample_times);
  _calibtree->Branch("lappd_id", &_lappd_id);
  _calibtree->Branch("strip_no", &_strip_no);


  return true;
}


bool LAPPDCalibrationWriter::Execute(){

	
	//only run this tool once
	_event_iter++;
	if(_event_iter != 0) return true;

	
	//DefaultSimple();
	FTBFPionRun();
	TFile* _calibrootfile = new TFile(_calibfilename,"RECREATE");

	_calibtree->Write();
	_calibrootfile->Close();
	delete _calibrootfile;
	_calibrootfile = nullptr;

	return true;
}



//cookie cutter default values for a calibration file.
//may not be suitable in all contexts
void LAPPDCalibrationWriter::DefaultSimple()
{
	
	int nchs_per_board = 30;
	vector<int> sync_channels = {29, 30}; //for every board
	_ignore_channel = false; //for every ch 
	_strip_side = false; //at first, then flip at each iteration. 
	_single_ended = false; 
	_transverse_position = 0.0; //calculated in the loop
	_parallel_equidistant_position = 0; //for all in this default calibration
	_strip_no = 0;
	
	_velocity = 114.0; //mm/ns
	double sample_width = 97.0; //ps
	double transverse_spacing = 8.1; // mm
	double halfside = transverse_spacing*14;
	int top_or_bottom = 0;

	//fill sample times which will be the same for all channels
	for(int i = 0; i < 256; i++)
	{
		_sample_times.push_back(i*sample_width);
	}

	//create only 15 transverse positions, correpsonding to the
	//strip numbers. 
	vector<double> trans_pos;
	for(int i = 0; i < 15; i++)
	{
		trans_pos.push_back(i*transverse_spacing);
	}


	//as default, do 4 boards
	int nboards = 4;
	vector<int> board_names = {10, 20, 33, 36};
	vector<int> lappd_ids = {1, 0, 0, 1};

	for(int board = 0; board < nboards; board++)
	{
		//give it a side
		if(board % 2 == 0)
		{
			_strip_side = false;
			if(board == 2) top_or_bottom = 1;
			else top_or_bottom = 0;
		} 
		else
		{
			_strip_side = true;
			if(board == 3) top_or_bottom = 1;
			else top_or_bottom = 0;
		}

		//board attributes
		_board_name = board_names.at(board);
		_lappd_id = lappd_ids.at(board);
		_board_id = board;

		//the only channel attributes that change are
		//it's id and whether it is a sync channel. 
		for(int ch = 1; ch <= nchs_per_board; ch++)
		{
			//check if it is a synch channel
			vector<int>::iterator it = std::find(sync_channels.begin(), sync_channels.end(), ch);
			if(it != sync_channels.end())
			{
				_sync_channel = true;
			}
			else
			{
				_sync_channel = false;
			}

			if(_sync_channel)
			{
				_strip_no = 100; //handle this better in the future maybe? 
				_transverse_position = 1000; //handle this better? 
			} 
			else
			{
				if(ch > 14) _strip_no = ch - 15;
				else _strip_no = ch - 1;
				_transverse_position = trans_pos.at(_strip_no) + top_or_bottom*halfside;
			}

			_channel_id = ch;

			_calibtree->Fill();
		}
	}
	

	return;
}


//This is for Evan's last week of beam in 2019
//Pion Run from organized_last_beam. 
void LAPPDCalibrationWriter::FTBFPionRun()
{
	
	DefaultSimple();

	return;
}

void LAPPDCalibrationWriter::HandEntry()
{
	return;
}


bool LAPPDCalibrationWriter::Finalise(){

	

 	return true;
}
