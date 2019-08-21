#include "LAPPDParseACC.h"
#include "Geometry.h"

LAPPDParseACC::LAPPDParseACC():Tool(){}

using namespace std;



bool LAPPDParseACC::Initialise(std::string configfile, DataModel &data){


  /////////////////// Usefull header ///////////////////////
  if(configfile!="")  m_variables.Initialise(configfile); //loading config file
  //m_variables.Print();

  m_data= &data; //assigning transient data pointer
  /////////////////////////////////////////////////////////////////

  //datafilename
  string path;
  m_variables.Get("lappd_data_filepath", path); // does not have a "/" at the end
  string name;
  m_variables.Get("lappd_data_filename", name); // does not have a suffix ".acdc" 
  string filebase = path + "/" + name;


  //open the ACDC filestream
  string line; //dummy line variable
  string datapath = filebase + ".acdc";
  dfs.open(datapath.c_str());
  if(!dfs.is_open())
  {
    cout << "Could not load acdc data file." << endl;
    return false;
  }
  //process the header already
  getline(dfs, line);

  //open metadata filestream
  string metapath = filebase + ".meta";
  mfs.open(metapath.c_str());
  if(!mfs.is_open())
  {
    cout << "Could not load acdc data file." << endl;
    return false;
  }
  //process the header already
  getline(mfs, meta_header);

  return true;
}


bool LAPPDParseACC::Execute(){


  //find the geometry based on a string
  //given by the config file
  string geoname;
  m_variables.Get("geometry_name", geoname);
  string storename; 
  m_variables.Get("store_name", storename);
  bool geomfound;
  Geometry* geom;
  geomfound = m_data->Stores.at(storename)->Header->Get(geoname,geom);
  if(!geomfound)
  {
    cout << "Geometry " << geoname << " in store " << storename << " was not found while attempting to prase ACDC data" << endl;
    return false;
  }

  //isolate the LAPPDs and their channel lists
  //so that when we parse an event in the data file, 
  //we look for that board/channel and re-key the data
  //stream to match the geometry channel keys. 
  map<unsigned long, Channel>* all_lappd_channels; 
  map<string, map<unsigned long,Detector*> >* AllDetectors = geom->GetDetectors();
  map<string, map<unsigned long,Detector*> >::iterator itGeom;
  for(itGeom = AllDetectors->begin(); itGeom != AllDetectors->end(); ++itGeom)
  {
    if(itGeom->first == "LAPPD")
    {
      map<unsigned long,Detector*> LAPPDDetectors = itGeom->second;
      map<unsigned long, Detector*>::iterator itDet;
      for(itDet = LAPPDDetectors.begin(); itDet != LAPPDDetectors.end(); ++itDet)
      {
        //here are the channel objects for this particular LAPPD
        map<unsigned long, Channel>* lappdchannels = itDet->second->GetChannels();
        //now loop through and insert into the all_lappd_channels 
        //map to make a cumulative map of all channels
        map<unsigned long, Channel>::iterator itCh;
        for(itCh = lappdchannels->begin(); itCh != lappdchannels->end(); ++itCh)
        {
          all_lappd_channels->insert(pair<unsigned long, Channel>(itCh->first, itCh->second));
        }
      }
    }
  }


  //print to test

  map<unsigned long, Channel>::iterator itCh;
  for(itCh = all_lappd_channels->begin(); itCh != all_lappd_channels->end(); ++itCh)
  {
    Channel thech = itCh->second;
    unsigned long k = itCh->first;
    cout << "Board " << thech.GetSignalCard() << " channel " << thech.GetSignalChannel() << " has key " << k << " and channel id " << thech.GetChannelID() << " strip num " << thech.GetStripNum() << " on side " << thech.GetStripSide() << endl;
  }



  /*

  //structure: 
  //rawData[event][board][channel] = Waveform<double> 
  map<int, map<int, map<int, Waveform<double>>>> rawData;



  int event, board, channel;
  string line;
  int line_counter = 0;
  double adccounts_to_mv = 1.2*1000.0/4096.0;

  //start of this loop is just after
  //the header of the datafile
  Waveform<double> tempwav;
  while(getline(dfs, line))
  {
    istringstream iss(line); //the current line in the file
    int temp_bit; //the current integer/bit in the line
    int char_count = -1; //counts the present integer/bit number in the line

    //top of this loop is the start of the line
    //in the data file. 
    while(iss >> temp_bit)
    {
      char_count++;

      //if we are on the event number
      if(char_count == 0)
      {
        event = temp_bit;
        continue;
      }
      if(char_count == 1)
      {
        //this is the board number
        //not implemented yet
        board = temp_bit;
        continue;
      }
      if(char_count == 2)
      {
        //this is the channel number
        channel = temp_bit;
        continue;
      }
      //if none of the above if statements fire
      //then we are in a sample (0 - 255) of ADC counts
      tempwav.PushSample(temp_bit*adccounts_to_mv);
    }

    //check to make sure the size of the tempwav is correct
    int tempsize = tempwav.GetSamples()->size();
    if(tempsize != n_cells)
    {
      cout << "Warning, event " << event << " board " \
      << board << " channel " << channel << \
      " has the incorrect number of samples: " << tempsize << endl;
    }
    //push to data, reinitialize
    rawData[event][board][channel] = tempwav;
    tempwav.ClearSamples();
    line_counter++;
  }

  m_data->Stores["ANNIEEvent"]->Set("RawLAPPDData_ftbf", rawData);
  


  //Metadata 
  //structure metadata[event][board]["key string"] = unsigned int 

  map<int, map<int, map<string, unsigned int>>> metadata;
  stringstream headers(meta_header);
  string header; //temp variable for the particular header column parsing the stringstream headers
  vector<string> header_vec; //a vector containing header column info
  bool first_loop = true; //flag to fill the header_vec only the first time around
  while(getline(mfs, line))
  {
    istringstream iss(line); //the current line in the file
    unsigned int temp_bit;
    int char_count = -1;
    while(iss >> temp_bit)
    {
      headers >> header;
      if(first_loop) header_vec.push_back(header);
      char_count++;

      //if we are on the event number
      if(char_count == 0)
      {
        event = temp_bit;
        continue;
      }
      if(char_count == 1)
      {
        //this is the board number
        //not implemented yet
        board = temp_bit;
        continue;
      }
      //if none of the above if statements fire
      //then we are in a metadata key
      metadata[event][board][header_vec.at(char_count)] = temp_bit;
    }
    first_loop = false;
  }

  m_data->Stores["ANNIEEvent"]->Header->Set("metaData", metadata);


  


  isLoaded = true;
  m_data->Stores["ANNIEEvent"]->Set("isLoaded", isLoaded); //have loaded the entire data file
  */
  return true;
}


bool LAPPDParseACC::Finalise(){
  dfs.close();
  mfs.close();

  return true;
}
