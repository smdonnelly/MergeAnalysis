#include "EstimateNPE.h"

EstimateNPE::EstimateNPE():Tool(){}


bool EstimateNPE::Initialise(std::string configfile, DataModel &data){

  /////////////////// Useful header ///////////////////////
  if(configfile!="") m_variables.Initialise(configfile); // loading config file
  //m_variables.Print();

  m_data= &data; //assigning transient data pointer
  /////////////////////////////////////////////////////////////////

  return true;
}


bool EstimateNPE::Execute(){
	m_variables.Get("store_name", storename);

	

	//temporary for my own testing
	map<unsigned long, double> channel_charges;
	m_data->Stores.at(storename)->Get("channel_charges", channel_charges);

	return true;
}


bool EstimateNPE::Finalise(){

  return true;
}
