#include "LAPPDIntegratePulse.h"

LAPPDIntegratePulse::LAPPDIntegratePulse():Tool(){}

// THIS CODE INTEGRATES AN LAPPD WAVEFORM OVER A FIXED RANGE DEFINED IN THE CONFIG FILE

bool LAPPDIntegratePulse::Initialise(std::string configfile, DataModel &data){

  /////////////////// Usefull header ///////////////////////
  if(configfile!="")  m_variables.Initialise(configfile); //loading config file
  //m_variables.Print();

  m_data= &data; //assigning transient data pointer
  /////////////////////////////////////////////////////////////////

  

  return true;
}


bool LAPPDIntegratePulse::Execute(){

  //storename for various functions and saving
  m_variables.Get("store_name", storename);
  m_variables.Get("termination", termination);//termination resistance in ohms
  string integ_type;
  m_variables.Get("integration_type", integ_type);
  if(integ_type == "ranged")
  {
    RawConstRangeIntegral();
  }
  else if(integ_type == "nnls")
  {
    map<unsigned long, NnlsSolution> nnlssoln;
    m_data->Stores.at(storename)->Get("nnls_solution", nnlssoln);
    TotalChargeFromNNLS(nnlssoln);
  }
  else
  {
    cout << "Unknown integration mode for LAPPDIntegratePulse: " << integ_type << ". Did nothing" << endl;
  }

  return true;
}


bool LAPPDIntegratePulse::Finalise(){

  return true;
}



//Integrates the full NNLS solution waveforms
//to find the total charge on each channel. 
void LAPPDIntegratePulse::TotalChargeFromNNLS(map<unsigned long, NnlsSolution> nnlssoln)
{

  map<unsigned long, double> channel_charges;
  map<unsigned long, NnlsSolution>::iterator itr;

  double grand_total_charge = 0; //total charge from all channels summed
  for(itr = nnlssoln.begin(); itr != nnlssoln.end(); ++itr)
  {
    unsigned long ch = itr->first;
    Waveform<double>* fullsln = itr->second.GetFullSoln();
    vector<double> times = itr->second.GetFullSolutionTimes();
    double sumcharge = 0; //units of mV*psec
    //integrate wrt times
    for(int i = 0; i < int(times.size()) - 1; i++)
    {
      //square reimannian on left sample
      sumcharge += fullsln->GetSample(i)*(times.at(i+1) - times.at(i));
    }
    sumcharge = sumcharge/termination; //units of mV*psec/ohms
    grand_total_charge+= sumcharge;
    channel_charges.insert(pair<unsigned long, double>(ch, sumcharge));
  }
  cout << "Total grand charge: " << grand_total_charge << "mV*psec/ohm" << endl;
  m_data->Stores.at(storename)->Set("channel_charges", channel_charges);
  m_data->Stores.at(storename)->Set("event_charge", grand_total_charge);
  return;
}

//I created this function to separate old code that
//does raw integration on ANNIEEvent waveforms with
//new code that either does a raw, constant time range integration
//or nnls integration total or nnls component integration
void LAPPDIntegratePulse::RawConstRangeIntegral()
{
  // Make note that the pulse has been integrated
  bool isIntegrated = true;
  m_data->Stores["ANNIEEvent"]->Header->Set("isIntegrated",isIntegrated);

  // Get from the configuration file the parameters of the waveform
  m_variables.Get("Nsamples", DimSize);
  m_variables.Get("SampleSize",Deltat);

  // Get from the config file, the range over which to integrate
  m_variables.Get("IntegLow",lowR);
  m_variables.Get("IntegHi",hiR);

  // get raw lappd data
  map<unsigned long, Waveform<double>> rawlappddata;
  bool testval =  m_data->Stores["ANNIEEvent"]->Get("LAPPDWaveforms",rawlappddata);

  map<unsigned long, double> thecharge;

  map<unsigned long, Waveform<double>> :: iterator itr;
  for (itr = rawlappddata.begin(); itr != rawlappddata.end(); ++itr){
    unsigned long channelno = itr->first;
    Waveform<double> bwav = itr->second;

    // integrate the pulse from the low range to high range in units of mV*psec
    double Qmvpsec = CalcIntegral(bwav,lowR,hiR);
    // convert to coulomb
    double Qcoulomb = Qmvpsec/(1000.*50.*1e12);
    //convert to number of electrons
    double Qelectrons = Qcoulomb/(1.60217733e-19);


    // store the charge vector by channel
    thecharge.insert(pair<unsigned long, double> (channelno,Qelectrons));
  }

    // add the charge information to the Boost Store
    m_data->Stores["ANNIEEvent"]->Set("channel_charges",thecharge);

    return;
}

double LAPPDIntegratePulse::CalcIntegral(Waveform<double> hwav, double lowR, double hiR){

  double sT=0.; // currently hard coded

  int lowb = (lowR - sT)/Deltat;
  int hib = (hiR - sT)/Deltat;

  double tQ=0.;

  if( (lowb>=0) && (hib<hwav.GetSamples()->size()) ){
    for(int i=lowb; i<hib; i++){
      tQ+=((-hwav.GetSample(i))*Deltat);
    }
  } else std::cout<<"OUT OF RANGE!!!!";

  return tQ;
}
