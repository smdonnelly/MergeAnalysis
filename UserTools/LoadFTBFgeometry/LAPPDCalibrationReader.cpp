#include "LAPPDCalibrationReader.h"



LAPPDCalibrationReader::LAPPDCalibrationReader(TString calibfilename)
{
	_calibfilename = calibfilename;
	 _tf = new TFile(_calibfilename, "READ");
	 _tf->GetObject("acdc_calib", _calibtree);
	 


	_calibtree->SetBranchAddress("board_name",&_board_name);
	_calibtree->SetBranchAddress("board_id",&_board_id);
	_calibtree->SetBranchAddress("channel_id",&_channel_id);
	_calibtree->SetBranchAddress("ignore_channel",&_ignore_channel);
	_calibtree->SetBranchAddress("single_ended",&_single_ended);
	_calibtree->SetBranchAddress("strip_side",&_strip_side);
	_calibtree->SetBranchAddress("sync_channel",&_sync_channel);
	_calibtree->SetBranchAddress("velocity",&_velocity);
	_calibtree->SetBranchAddress("parallel_equidistant_position",&_parallel_equidistant_position);
	_calibtree->SetBranchAddress("transverse_position",&_transverse_position);
	_calibtree->SetBranchAddress("sample_times",&_sample_times);
	_calibtree->SetBranchAddress("lappd_id", &_lappd_id);
	_calibtree->SetBranchAddress("strip_no", &_strip_no);


}


LAPPDCalibrationReader::~LAPPDCalibrationReader()
{
	_tf->Close();
	delete _tf;
	_tf = nullptr;
}