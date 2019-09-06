#include "LoadFTBFgeometry.h"
#include "TFile.h"
#include "LAPPDCalibrationReader.h"

LoadFTBFgeometry::LoadFTBFgeometry():Tool(){}


bool LoadFTBFgeometry::Initialise(std::string configfile, DataModel &data){

  /////////////////// Usefull header ///////////////////////
  if(configfile!="")  m_variables.Initialise(configfile); //loading config file
  //m_variables.Print();

  m_data= &data; //assigning transient data pointer
  /////////////////////////////////////////////////////////////////
  int get_ok = 1;

 get_ok = m_variables.Get("verbose", verbosity);


 //create a store for the FTBF data
 m_data->Stores["FTBFEvent"]=new BoostStore(false,2);

  
  get_ok = m_variables.Get("ChannelConfigFile", calibfilename);
  if(!get_ok)
  {
  	cout << "Could not find ACDC calibration file and thus cannot associate boards with lappds, channels with timestamps, channels with positions ... " << endl;
  	cout << "Please construct and include a ChannelConfigFile in your ToolChainConfig" << endl;
  	return false;
  }


	
  return true;
}


bool LoadFTBFgeometry::Execute(){


	//variable names similar to annie geom for 
	//comparison. basically want to construct
	//a geometry with two lappds and nothing else
	int numtankpmts = 0;
	int numlappds = 2;
	int nummrdpmts = 0;
	int numvetopmts = 0;

	//this is the center of the TOF stand.
	//the LAPPDs will be placed relative to center
	double tof_stand_xcenter = 0; //m
	double tof_stand_ycenter = 0; //m
	double tof_stand_zcenter = 0; //m
	
	Position tof_stand_center(tof_stand_xcenter, tof_stand_ycenter, tof_stand_zcenter);

	//Default values in Geometry.h are 0, 
	//and not used in Geometry.cpp. Just making
	//these the default values to avoid confusion
	double tank_radius = 0; //m
	double tank_halfheight = 0; //m
	double pmt_enclosed_radius = 0;
	double pmt_enclosed_halfheight = 0;
	double mrd_width  = 0;
	double mrd_height = 0;
	double mrd_depth  = 0;
	double mrd_start  = 0;

	if(verbosity>1)
	{
		cout<<"built FTBF geometry with tank center = ";
		tof_stand_center.Print(false);
		cout << " and " << numlappds << " lappds" << endl;
	} 

	
	// construct the ToolChain Goemetry
	// ================================
	Geometry* ftbfgeom = new Geometry(0,
									   tof_stand_center,
									   tank_radius,
									   tank_halfheight,
									   pmt_enclosed_radius,
									   pmt_enclosed_halfheight,
									   mrd_width,
									   mrd_height,
									   mrd_depth,
									   mrd_start,
									   numtankpmts,
									   nummrdpmts,
									   numvetopmts,
									   numlappds,
									   geostatus::FULLY_OPERATIONAL);
	if(verbosity>1){
		cout<<"constructed ftbf geometry with info:"<< endl; 
		ftbfgeom->Print();
	}
	m_data->Stores.at("FTBFEvent")->Header->Set("FTBFGeometry",ftbfgeom,true);
	
	
	// Construct the Detectors and Channels
	// ====================================
	LAPPDCalibrationReader acdc_calib_reader(calibfilename);
	map<unsigned long, vector<double>>* sample_time_map = new map<unsigned long, vector<double>>;
	// lappds
	for(int lappdi=0; lappdi<numlappds; lappdi++){
		
		// Construct the detector associated with this tile
		unsigned long uniquedetectorkey = ftbfgeom->ConsumeNextFreeDetectorKey();
		int tubeno = lappdi; //TODO: load tube number from calibration file/dataobject
		lappd_tubeid_to_detectorkey.emplace(tubeno,uniquedetectorkey); 
		detectorkey_to_lappdid.emplace(uniquedetectorkey,tubeno);

		//input measured values or reconstruction values
		//for LAPPD positions. 

		//ORIGIN INFO: 
		//Beam axis is at x=y=0. 
		//Will tune LAPPD based on this recursively
		std::string LocString;
		Position thislappd_pos; 
		Direction lappd_direc(0,0,1); //assume for the moment they are the same
		if(tubeno == 0)
		{
			LocString = "FRONT";
			thislappd_pos.SetX(0.);
			thislappd_pos.SetY(0.);
			thislappd_pos.SetZ(0.5); //m by default
		} 
		else if(tubeno == 1)
		{
			LocString = "BACK";
			thislappd_pos.SetX(0.);
			thislappd_pos.SetY(0.);
			thislappd_pos.SetZ(-0.5); //m by default
		} 
		else
		{
			cout << "Could not find LAPPD tube number from config file" << endl;
			cout << "Placing this LAPPD at the origin" << endl; //default constructor is 0,0,0 for position
			LocString = "UNKNOWN";	
		} 


		Detector adet(uniquedetectorkey,
					  "LAPPD",
					  LocString,
					  thislappd_pos,
					  lappd_direc,
					  "LAPPD",
					  detectorstatus::ON,
					  0.);
		

		
		// construct all the channels associated with this LAPPD.
		// Loop through the ACDC calibration file which associates
		// acdc's with LAPPDs and also stores calibration information. 
		// This calib file is parsed by the LAPPDCalibration reader 
		// initialized above before the lappd loop. 
		for(int entry=0; entry<acdc_calib_reader.GetEntries(); entry++){
			acdc_calib_reader.LoadEntry(entry);
			//this calibration file holds ALL channel information
			//on all boards. Only process the current entry
			//if it is indeed associated with this LAPPD number
			if(acdc_calib_reader.GetLappdID() != tubeno) continue;
			if(verbosity>4)
			{
				cout << "Parsing configuration of channel " << acdc_calib_reader.GetChannelID();
				cout << " on board " << acdc_calib_reader.GetBoardID();
				cout << " , ACDC " << acdc_calib_reader.GetBoardName() << endl;
			}
			


			unsigned long uniquechannelkey = ftbfgeom->ConsumeNextFreeChannelKey();

			
			int stripside = (int)(acdc_calib_reader.GetStripSide());   // StripSide=0 for LHS (x<0), StripSide=1 for RHS (x>0)
			int stripnum = acdc_calib_reader.GetStripNo();    // Strip number: add 2 channels per strip as we go

			//Let “x0" be the point on the conductor joining the two digitizers 
			//such that if a delta function were generated at “x0”, 
			//the signals arrive at the digitizers at the same time. 
			double xpos = acdc_calib_reader.GetParallelPosition(); //this is x0 as described above
			double ypos = acdc_calib_reader.GetTransversePosition();
			
			// record data on ACDC numbers and ACC numbers, and HV crate numbers
			int ACC_Card_Num = 0; //for all, only used one ACC
			int ACC_Crate_Num = 0; //only one crate
			int ACDC_Card_Num = (int)acdc_calib_reader.GetBoardID();
			int ACC_Chan_Num = (int)acdc_calib_reader.GetBoardID();; //this is damn redundance, so I may be misunderstanding something
			int ACDC_Crate_Num = 0; //don't really know what this represents
			int ACDC_Chan_Num = acdc_calib_reader.GetChannelID(); //references the acdc data file
			int LAPPD_HV_Crate_Num = 0; // all comes from the same crate
			int LAPPD_HV_Card_Num = tubeno; //labeled by LAPPD number
			int LAPPD_HV_Chan_Num = tubeno; //same

			//channel status
			auto chstatus = channelstatus::ON;
			if(acdc_calib_reader.GetIgnoreChannel()) chstatus = channelstatus::OFF;
			else if(acdc_calib_reader.GetSyncChannel()) chstatus = channelstatus::SYNC;
			else chstatus = channelstatus::ON;

			Channel lappdchannel(uniquechannelkey,
								 Position(xpos,ypos,0.),
								 stripside,
								 stripnum,
								 ACDC_Crate_Num,
								 ACDC_Card_Num,
								 ACDC_Chan_Num,
								 ACC_Crate_Num,
								 ACC_Card_Num,
								 ACC_Chan_Num,
								 LAPPD_HV_Crate_Num,
								 LAPPD_HV_Card_Num,
								 LAPPD_HV_Chan_Num,
								 chstatus);

			
			// Add this channel to the geometry
			if(verbosity>4) cout<<"Adding channel "<<uniquechannelkey<<" to detector "<<uniquedetectorkey<<endl;
			adet.AddChannel(lappdchannel);


			sample_time_map->insert(pair<unsigned long, vector<double>>(uniquechannelkey, acdc_calib_reader.GetSampleTimes()));

		}
		if(verbosity>4) cout<<"Adding detector "<<uniquedetectorkey<<" to geometry"<<endl;
		// Add this detector to the geometry
		ftbfgeom->AddDetector(adet);
		if(verbosity>4) cout<<"printing geometry channels"<<endl;
		if(verbosity>4) ftbfgeom->PrintChannels();
	}

  	acdc_calib_reader.CloseRootFile();

  // for other WCSim tools that may need the WCSim Tube IDs
	m_data->Stores.at("FTBFEvent")->Header->Set("lappd_tubeid_to_detectorkey",lappd_tubeid_to_detectorkey);
	// inverse
	m_data->Stores.at("FTBFEvent")->Header->Set("detectorkey_to_lappdid",detectorkey_to_lappdid);
	m_data->Stores.at("FTBFEvent")->Set("sample_time_map", sample_time_map);

	
  return true;
}


bool LoadFTBFgeometry::Finalise(){

  return true;
}
