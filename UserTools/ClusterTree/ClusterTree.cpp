#include "ClusterTree.h"

ClusterTree::ClusterTree():Tool(){}


bool ClusterTree::Initialise(std::string configfile, DataModel &data)
{

  /////////////////// Useful header ///////////////////////
  if(configfile!="") m_variables.Initialise(configfile); // loading config file
  //m_variables.Print();

  m_data= &data; //assigning transient data pointer
  /////////////////////////////////////////////////////////////////
  nottf = new TFile("Analysis.root","RECREATE");
  fMyTree = new TTree("ffmytree","ffmytree");

  // set the branches

  fMyTree->Branch("NHits",            &NHits,             "NHits/I"               );
  fMyTree->Branch("Q",                hQ,                 "Q[NHits]/D"            );
  fMyTree->Branch("xpar",             hxpar,              "Xpar[NHits]/D"         );
  fMyTree->Branch("xperp",            hxperp,             "Xperp[NHits]/D"        );
  fMyTree->Branch("time",             htime,              "time[NHits]/D"         );
  fMyTree->Branch("deltime",          hdeltime,           "deltime[NHits]/D"      );
  fMyTree->Branch("Vpeak",            hvpeak,            "Vpeak[NHits]/D"        );



  return true;
}


bool ClusterTree::Execute()
{
  nottf->cd();
  std::map<unsigned long,vector<LAPPDHit>> Hits;
  m_data->Stores["ANNIEEvent"]->Get("Clusters",Hits);
  map <unsigned long, vector<LAPPDHit>> :: iterator itr;
  vector<LAPPDHit> :: iterator itrr;
//  cout<<"SETUP COMPLETE"<< endl;

  for (itr = Hits.begin(); itr != Hits.end(); ++itr)
    {
    //  cout<<"HANDLING A HIT YO"<<endl;
      unsigned long  channel= itr->first;
      vector<LAPPDHit> hitvect = itr->second;
      int numberhits = itr->second.size();
      NHits = numberhits;
      int m=0;
      for (itrr = itr->second.begin(); itrr!= itr->second.end(); ++itrr)
	{
	  //cout<<"GETTING INFO"<<endl;
	  hT[m]=hitvect[m].GetTime();
	 //  cout<<"GETTING INFO"<<endl;
	  hQ[m]=hitvect[m].GetCharge();
	//  cout<<"GETTING INFO"<<endl;
	  vector<double> time;
	  if(time.size()>0)
	    {
	      time=hitvect[m].GetLocalPosition();
	      hxpar[m]=time[0];
	//      cout<<"GETTING INFO"<<endl;
	      hxperp[m]=time[1];
	    }
	//    cout<<"GETTING INFO"<<endl;
	    htime[m]=hitvect[m].GetTime();
	  m++;
	}
  //    cout<<"FILLING TREE WITH HITS"<<endl;      
      fMyTree->Fill();

    }

  return true;
}


bool ClusterTree::Finalise(){

  nottf->cd();
  fMyTree->Write();



  nottf->Close();
  return true;
}
