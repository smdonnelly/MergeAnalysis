#include "LAPPDBaselineSubtract.h"

LAPPDBaselineSubtract::LAPPDBaselineSubtract():Tool(){}


bool LAPPDBaselineSubtract::Initialise(std::string configfile, DataModel &data){

  /////////////////// Usefull header ///////////////////////
  if(configfile!="")  m_variables.Initialise(configfile); //loading config file
  //m_variables.Print();

  m_data= &data; //assigning transient data pointer
  /////////////////////////////////////////////////////////////////

  

  m_variables.Get("Nsamples", DimSize);
  m_variables.Get("SampleSize",Deltat);
  m_variables.Get("LowBLfitrange", LowBLfitrange);
  m_variables.Get("HiBLfitrange",HiBLfitrange);

  return true;
}

//This function has a switch case for 
//various baseline subtraction methods. 
//it then stores baseline subtracted
//data into a new map in the store. 
//It also secretly removes non-physical
//spikes in the data. 
bool LAPPDBaselineSubtract::Execute(){


  string storename; 
  m_variables.Get("store_name", storename);

  // get raw lappd data
  map<unsigned long, Waveform<double>> LAPPDWaveforms;
  m_data->Stores.at(storename)->Get("LAPPDWaveforms",LAPPDWaveforms);

  bool isBLsub = true;
  m_data->Stores.at(storename)->Header->Set("isBLsubtracted",isBLsub);

  int remove_spikes = 1;
  m_variables.Get("remove_spikes", remove_spikes);

  string subtraction_method;
  m_variables.Get("subtraction_method", subtraction_method);

  // the filtered Waveform
  map<unsigned long, Waveform<double>> blsublappddata;

  map<unsigned long, Waveform<double>> :: iterator itr;
  for (itr = LAPPDWaveforms.begin(); itr != LAPPDWaveforms.end(); ++itr)
  {
    unsigned long channelno = itr->first;
    Waveform<double> bwav = itr->second;

    Waveform<double> input_wave; 

    if(remove_spikes == 1)
    {
      input_wave = RemoveSpikes(bwav);
    }
    else
    {
      input_wave = bwav;
    }
    
    Waveform<double> blswav;
    if(subtraction_method == "median")
    {
      blswav = SubtractMedian(input_wave);
    }
    else if(subtraction_method == "sin")
    {
      blswav = SubtractSine(input_wave);
    }
    

    blsublappddata.insert(pair <unsigned long, Waveform<double>>(channelno,blswav));
  }

  m_data->Stores.at(storename)->Set("LAPPDWaveforms",blsublappddata);

  return true;
}


//If the raw waveform has a sample that is >=1200mV
//then linearly interpolate to the next good sample value. 
Waveform<double> LAPPDBaselineSubtract::RemoveSpikes(Waveform<double> iwav) {
  Waveform<double> rmWav;

  vector<double> iwav_vec = *(iwav.GetSamples());
  vector<double> adjusted_vec;

  int left_good_sample;
  int right_good_sample;
  bool caught = false;
  double slope;
  double shift;
  double absmax = 1200.0; //mV
  for(int i = 0; i < (int)iwav_vec.size(); i++)
  {
    if(abs(iwav_vec.at(i)) >= absmax)
    {
      //if this is first time crossing
      if(!caught)
      {
        left_good_sample = i-1;
        caught = true;
      }
    }
    else if(abs(iwav_vec.at(i)) < absmax)
    {
      //if we had a caught spike
      //but now are below, modify the
      //waveform
      if(caught)
      {
        right_good_sample = i;
        //something went horribly wrong with logic
        //if this happens. double check code
        if(right_good_sample == left_good_sample)
        {
          cout << "something wrong with logic. cannot remove spikes" << endl;
          rmWav.SetSamples(iwav_vec);
          return rmWav;
        }
        //interpolate between good samples
        double y1 = iwav_vec.at(left_good_sample);
        double y2 = iwav_vec.at(right_good_sample);
        slope = (y2 - y1)/(right_good_sample - left_good_sample + 1); //denom is never 0
        shift = y1 - slope*left_good_sample;

        //add corrected samples to adjusted_vector
        for(int j = left_good_sample+1; j < right_good_sample; j++)
        {
          adjusted_vec.push_back(slope*j + shift);
        }

        //reinitialize to normal state
        caught = false;

      }
      else
      {
        adjusted_vec.push_back(iwav_vec.at(i));
      }
    }
    else
    {
      continue;
    }
  }

  rmWav.SetSamples(adjusted_vec);
  return rmWav;

}


Waveform<double> LAPPDBaselineSubtract::SubtractMedian(Waveform<double> iwav) {
  Waveform<double> subWav;

  vector<double> thesamples = *(iwav.GetSamples());
  //sort the vector
  sort(thesamples.begin(), thesamples.end());


  int nsamples = (int)thesamples.size();
  double median = 0;
  //if it's even, take average of two middle samples
  if(nsamples % 2 == 0)
  {
    median = 0.5*(thesamples.at((int)nsamples/2) + thesamples.at((int)nsamples/2 - 1));
  }
  else
  {
    median = thesamples.at((int)nsamples/2);
  }

  //subtract median from all samples in raw wav
  for(int i = 0; i < (int)thesamples.size(); i++)
  {
    subWav.PushSample(iwav.GetSample(i) - median);
  }

  return subWav;

}



Waveform<double> LAPPDBaselineSubtract::SubtractSine(Waveform<double> iwav) {

  Waveform<double> subWav;

  int nbins = iwav.GetSamples()->size();
  double starttime=0.;
  double endtime = starttime + ((double)nbins)*100.;
  TH1D* hwav_raw = new TH1D("hwav_raw","hwav_raw",nbins,starttime,endtime);

  for(int i=0; i<nbins; i++){
    hwav_raw->SetBinContent(i+1,iwav.GetSample(i));
    hwav_raw->SetBinError(i+1,0.1);

  }

  TF1* sinit = new TF1("sinit","([0]*sin([2]*x+[1]))",0,DimSize*Deltat);
  sinit->SetParameter(0,0.4);
  sinit->SetParameter(1,0.0);
  sinit->SetParameter(2,0.0);
  sinit->SetParameter(2,0.00055);
  sinit->SetParLimits(2,0.0003,0.0008);
  sinit->SetParLimits(0,0.,1.0);

  hwav_raw->Fit("sinit","QNO","",LowBLfitrange,HiBLfitrange);
  //cout<<"Parameters: "<< sinit->GetParameter(3)<<" "<<LowBLfitrange<<" "<<HiBLfitrange<<endl;

  for(int j=0; j<nbins; j++){

    subWav.PushSample((hwav_raw->GetBinContent(j+1))-(sinit->Eval(hwav_raw->GetBinCenter(j+1))));
  }

  delete hwav_raw;
  delete sinit;
  return subWav;
}


bool LAPPDBaselineSubtract::Finalise(){

  return true;
}
