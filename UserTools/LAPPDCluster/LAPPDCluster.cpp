#include "LAPPDCluster.h"

LAPPDCluster::LAPPDCluster():Tool(),_geom(nullptr){}


bool LAPPDCluster::Initialise(std::string configfile, DataModel &data){

  /////////////////// Useful header ///////////////////////
  if(configfile!="") m_variables.Initialise(configfile); // loading config file
  //m_variables.Print();

  m_data= &data; //assigning transient data pointer
  /////////////////////////////////////////////////////////////////
  TString storename_;
  m_variables.Get("store_name", storename_);
  storename=storename_;
  TString GL;
  m_variables.Get("geom_name",GL);
  GeometryLabel=GL;
  bool geomfound;
  string CL;
  m_variables.Get("ClusterLabel",CL);
  ClusterLabel = CL;


  return true;
}


bool LAPPDCluster::Execute(){

  //  cout<<"executing lappdcluster!!!"<<endl;
  bool geomfound;
  //  cout<<"Cluster Store name "<< storename<< " Cluster geometry name "<< GeometryLabel<< endl;
  geomfound=m_data->Stores.at(storename)->Header->Get(GeometryLabel, _geom);

  if(!geomfound)
    {
      cout << "Geometry " << GeometryLabel << " in store " << storename << " was not found while attempting to Cluster data" << endl;
    }

  std::map <unsigned long, vector<LAPPDPulse>> SimpleRecoLAPPDPulses;
  m_data->Stores.at(storename)->Get(ClusterLabel,SimpleRecoLAPPDPulses);
  //m_data->Stores[storename]->Get("SimpleRecoLAPPDPulses",SimpleRecoLAPPDPulses);
  // cout<<"We got "<< SimpleRecoLAPPDPulses.size()<< " Pulses" << endl;
  std::map <unsigned long, vector<LAPPDPulse>> :: iterator pulseitr;


  //  cout<<"Grabbed RecoLAPPDPulses "<<SimpleRecoLAPPDPulses.size()<<endl;

  vector<unsigned long> chanhand;
  std::map <unsigned long, vector<LAPPDHit>> Hits;


  

  // cout<<"!!!That is all pulses we have!!!"<<endl;
  for (pulseitr = SimpleRecoLAPPDPulses.begin(); pulseitr != SimpleRecoLAPPDPulses.end(); ++pulseitr){
    vector<LAPPDHit> thehits;
    vector<double> localposition;
    double ParaPosition=-5555;
    double PerpPosition=-5555;

    unsigned long chankey = pulseitr->first;

    bool handled = false;
    for(int i=0; i<chanhand.size(); ++i)
      {
        unsigned long key = chanhand[i];
        if(chankey==key)
	  {
	    handled = true;
	  }
      }
    if (handled)
      {
        cout<<chankey<<" is looped."<<endl;
        continue;
      }

    // cout<<"pulseitr chankey is "<<chankey<<endl;

    vector<LAPPDPulse> vPulse = pulseitr->second;

    // cout<<"iterating!!!   "<<chankey<<" "<<vPulse.size()<<endl;

    // if(vPulse.size()>1) {cout<<"VPULSE HAS A SIZE OF: "<<vPulse.size()<<endl;}
    for(int jj=0; jj<vPulse.size(); jj++){
      LAPPDPulse apulse = vPulse.at(jj);
      // cout<<"the charge of this pulse is: "<<apulse.GetCharge()<<endl;
      // cout<< "The Time of this Pulse is " <<apulse.GetTime() <<endl;
    }


    //breaks here... still. geometry not loaded in correctly.
    Channel* mychannel= _geom->GetChannel(chankey);
    // cout<<"the strip number is :"<<mychannel->GetStripNum()<<" and the side is: "<<mychannel->GetStripSide()<<endl;

    std::map<unsigned long , LAPPDPulse> cPulse;
    std::map <unsigned long, vector<LAPPDPulse>> :: iterator oppoitr;

    for (oppoitr = SimpleRecoLAPPDPulses.begin(); oppoitr != SimpleRecoLAPPDPulses.end(); ++oppoitr){

      unsigned long oppochankey = oppoitr->first;

      // cout<<"oppochankey is "<<oppochankey<<endl;

      vector<LAPPDPulse> oppovPulse = oppoitr->second;
      Channel* oppochannel = _geom->GetChannel(oppochankey);
      int mystripnum = mychannel->GetStripNum();
      int oppostripnum = oppochannel->GetStripNum();

      // cout<<"mystripnum is "<<mystripnum<<endl;
      // cout<<"oppostripnum is "<<oppostripnum<<endl;



      if( (oppochankey != chankey) && (mystripnum == oppostripnum) ){
	// cout<<"channel "<<chankey<<" and "<<oppochankey<<" are on the same strip."<<endl;
        cPulse.insert(pair <unsigned long,LAPPDPulse> (chankey,vPulse.at(0)));
        cPulse.insert(pair <unsigned long,LAPPDPulse> (oppochankey,oppovPulse.at(0)));
        //chanhand.push_back(oppochankey);
      }
      if ( (oppochankey != chankey) && (std::abs(oppostripnum-mystripnum)==1) ){
        if (mychannel->GetStripSide() == oppochannel->GetStripSide()){
	  // cout<<"channel "<<chankey<<" and "<<oppochankey<<" are on the same sides of adjacent strips."<<endl;
          cPulse.insert(pair <unsigned long,LAPPDPulse> (oppochankey,oppovPulse.at(0)));
          //chanhand.push_back(oppochankey);
        }
        else {
	  // cout<<"channel "<<chankey<<" and "<<oppochankey<<" are on the opposite sides of adjacent strips."<<endl;
          cPulse.insert(pair <unsigned long,LAPPDPulse> (oppochankey,oppovPulse.at(0)));
          //chanhand.push_back(oppochankey);
        }
      }

    }

    //Finding the Maxpulse
    // cout<<"Finding Maxpulse!!!"<<endl;
    double maxcharge = 0;
    unsigned long maxchankey=0;
    LAPPDPulse maxpulse;

    std::map<unsigned long , LAPPDPulse> :: iterator itr;

    // cout<<"What is in cPulse???"<<endl;
    //for (itr = cPulse.begin(); itr != cPulse.end(); ++itr){
    //cout<< itr->first <<endl;
    //cout<< itr->second.GetTime() <<endl;
    //}

    for (itr = cPulse.begin(); itr != cPulse.end(); ++itr){
      unsigned long mychankey = itr->first;
      LAPPDPulse mypulse = itr->second;
      // cout<<"!!!mychankey is "<<mychankey<<" !!!"<<endl;
      if (maxcharge>mypulse.GetCharge()){
	// cout<<"maxcharge "<<mypulse.GetCharge()<<endl;
        maxchankey = mychankey;
        maxpulse = mypulse;
      }
    }

    //cout<<maxchankey<<" is the maxchankey!!!!!!"<<endl;
    // cout<<"chankey is "<<chankey<<" !!!!!!"<<endl;



    if (maxchankey == chankey){
      //cout<<chankey<<" is the maxchankey!!!!!!"<<endl;
      for (itr = cPulse.begin(); itr != cPulse.end(); ++itr){
        chanhand.push_back(itr->first);
      }
    }
    else {
      // cout<<chankey<<" is not the maxchankey LOOP BREAKS"<<endl;
      continue;
    }


    maxcharge = maxpulse.GetCharge();

    //cout<<"the maxchankey is "<<maxchankey<<endl;
    //cout<<"Maxpulse is at "<<maxpulse.GetChannelID()<<endl;

    Channel* maxchannel= _geom->GetChannel(maxchankey);
    std::map<int,double> neighbourpulses;

    for (itr = cPulse.begin(); itr != cPulse.end(); ++itr){
      unsigned long thechankey = itr->first;
      LAPPDPulse mypulse = itr->second;

      Channel* mychannel = _geom->GetChannel(thechankey);
      //cout<<"thechankey "<<thechankey<<" maxchankey "<<maxchankey<<endl;
      //cout<<"MYStrip "<<mychannel->GetStripNum()<<" MAXStrip "<<maxchannel->GetStripNum()<<endl;
      //cout<<" "<<endl;
      if( (thechankey != maxchankey) && (mychannel->GetStripNum() == maxchannel->GetStripNum()) ) {
        //cout<<"WERTWER "<<maxpulse.GetTime()<<" "<<mypulse.GetTime()<<endl;
        //cout<<mychannel->GetStripSide()<<" "<<maxchannel->GetStripSide()<<endl;
        //cout<<" "<<endl;
        if ( (mychannel->GetStripSide()==0) && (maxchannel->GetStripSide()==1) ){
          //cout<<"case 1"<<endl;
          ParaPosition = ((mypulse.GetTime() - maxpulse.GetTime()) * 0.53 * (299.792458))/2.0;
        }
        if ( (mychannel->GetStripSide()==1) && (maxchannel->GetStripSide()==0) ){
          //cout<<"case 2"<<endl;
          ParaPosition = ((maxpulse.GetTime() - mypulse.GetTime()) * 0.53 * (299.792458))/2.0;
        }
        //cout<<leftpulse.GetTime()<<" "<<rightpulse.GetTime()<<endl;
      }
      if( (thechankey != maxchankey) && (abs(mychannel->GetStripNum() - maxchannel->GetStripNum()) == 1) && (mychannel->GetStripSide() == maxchannel->GetStripSide()) ) {
        neighbourpulses.insert(pair <int,double> (mychannel->GetStripNum(),mypulse.GetPeak()));
      }
    }
    neighbourpulses.insert(pair <int,double> (maxchannel->GetStripNum(),maxpulse.GetPeak()));

    double SumAbove=0.;
    double SumBelow=0.;

    if(neighbourpulses.size()>1){
      std::map<int,double>::iterator neighbouritr;
      for (neighbouritr = neighbourpulses.begin(); neighbouritr != neighbourpulses.end(); ++neighbouritr){
        int Strip = neighbouritr->first;
        double Peak = neighbouritr->second;
        //cout<<"!!!!!!prepcalculation "<<Strip<<" "<<Peak<<endl;
        SumAbove += ((double)Strip*Peak);
        SumBelow += (Peak);
      }
      if(SumBelow>0) {PerpPosition = (SumAbove / SumBelow);}
    }
    else {
      PerpPosition = maxchannel->GetStripNum();
    }

    //cout<<"Positions: "<<ParaPosition<<" "<<PerpPosition<<endl;
    localposition.push_back(ParaPosition);
    localposition.push_back(PerpPosition);


    //Putting information into LAPPDHit
    LAPPDHit myhit;
    myhit.SetTubeId(maxpulse.GetTubeId());
    myhit.SetTime(maxpulse.GetTime());
    myhit.SetCharge(maxcharge);
    myhit.SetLocalPosition(localposition);
    thehits.push_back(myhit);
    Hits.insert(pair <unsigned long,vector<LAPPDHit>> (chankey,thehits));
    chanhand.push_back(chankey);
    //cout<< "HANDLED" << endl;



  }
  m_data->Stores[storename]->Set("Clusters",Hits);
  return true;
}


bool LAPPDCluster::Finalise(){

  return true;
}
