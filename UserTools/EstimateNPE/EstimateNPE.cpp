#include "EstimateNPE.h"
#include "TFile.h"
#include "Detector.h"
#include "Geometry.h"
#include <numeric>

EstimateNPE::EstimateNPE():Tool(){}


bool EstimateNPE::Initialise(std::string configfile, DataModel &data){

  /////////////////// Useful header ///////////////////////
  if(configfile!="") m_variables.Initialise(configfile); // loading config file
  //m_variables.Print();

  m_data= &data; //assigning transient data pointer
  /////////////////////////////////////////////////////////////////

 	m_variables.Get("ChannelConfigFile", _calib_filename);
	TFile* tf = new TFile(_calib_filename, "READ");
	tf->cd();
	tf->GetObject("acdc_calib", _calib_tree);


	//set branch addresses for laser on and laser off data
	//and tube number

	_calib_tree->SetBranchAddress("lappd_id", &_lappd_id);
	_calib_tree->SetBranchAddress("laser_on_charges",&_laser_on);
	_calib_tree->SetBranchAddress("laser_off_charges",&_laser_off);

	string str_thresh;
	m_variables.Get("background_f", str_thresh);
	_threshold_fraction = stod(str_thresh);

 

  return true;
}


bool EstimateNPE::Execute(){
	m_variables.Get("store_name", storename);

	
	map<unsigned long, double> channel_charges;
	m_data->Stores.at(storename)->Get("channel_charges", channel_charges);

	//need to know what channel number corresponds to what
	//lappd number (i.e. what spe spectrum to look at)
	string geoname;
	m_variables.Get("geometry_name", geoname);
	bool geomfound;
	Geometry* geom;
	geomfound = m_data->Stores.at(storename)->Header->Get(geoname,geom);
	if(!geomfound)
	{
		cout << "Geometry " << geoname << " in store " << storename << " was not found while attempting to prase ACDC data" << endl;
		return false;
	}
	map<unsigned long,int> detectorkey_to_lappdid;
	m_data->Stores.at(storename)->Header->Get("detectorkey_to_lappdid",detectorkey_to_lappdid);

	//loop through all of the charges
	//and perform statistical analysis
	//on spectra to predict # of pe
	map<unsigned long, double> :: iterator chg_it;
	map<unsigned long, double> channel_npes;
	double grand_total = 0;
	for(chg_it = channel_charges.begin(); chg_it != channel_charges.end(); ++chg_it)
	{
		unsigned long chan = chg_it->first;
		double measured_charge = chg_it->second;
		//find lappd number
		Detector* thelappd = geom->ChannelToDetector(chan);
		unsigned long detector_key = thelappd->GetDetectorID();
		int tube_no = detectorkey_to_lappdid[detector_key];
		//loop through tree until reaching that tube number
		//thus filling the laser spe data.
		bool tube_found = false;
		for(int i = 0; i < _calib_tree->GetEntries(); i++)
		{
			_calib_tree->GetEntry(i);
			if(tube_no == _lappd_id)
			{
				//spe spectra are now populated
				//for the tube for this channel
				tube_found = true;
				break;
			}
		}
		if(tube_found == false)
		{
			cout << "Could not find detector tube number " << tube_no << " in calibration tree for LAPPDs" << endl;
		}
		else
		{
			double npe_estimate = RunEstimator(measured_charge);
			channel_npes.insert(pair<unsigned long, double>(chan, npe_estimate));
			grand_total += npe_estimate;
		}

	}

	m_data->Stores.at(storename)->Set("channel_npes", channel_npes);
	cout << "Grand total npes = " << grand_total << endl;

	
	return true;
}

//assumes that the laser on and laser off data
//have been populated into the vector<double> private
//variables. Uses the statistical methods outlined in 
//arXiv: 1602.03150v2 "Saldanha" to calculate most
//likely number of PEs and its error. 
double EstimateNPE::RunEstimator(double c)
{
	//-----
	//calculate the charge value of the 
	//"0-pe" cut based on where we cross
	//the threshold fraction of triggers from background
	//event (see equation (14))
	//----

	//order both charge distribution vectors
	//assumes charge is positive number
	vector<double> loff = *_laser_off;
	vector<double> lon = *_laser_on;

	sort(loff.begin(), loff.end());
	sort(lon.begin(), lon.end());

	//count and end when passing fractional cut
	double on_trigs = (double)lon.size(); //double for floating precision
	double off_trigs = (double)loff.size();
	int cut_index = (int)(_threshold_fraction*off_trigs);
	double cut_charge = loff.at(cut_index);

	int A_t = count_if(lon.begin(), lon.end(), [&](double const& val){ return val <= cut_charge;});
	double lambda_hat = -1*log(A_t/(_threshold_fraction*on_trigs)); 
	//cout << "A_t is found to be " << A_t << " for charge cut " << cut_charge << " and fraction " << _threshold_fraction << endl;
	//cout << "Lambda hat (occupancy) is found to be " << lambda_hat << endl;

	//for poisson emission from the laser, lambda_hat is estimate of the
	//mean of the laser emission distribution, E[L] from Saldanha eq (9)

	//calculate means and variances of signal and background
	double E_B = 0;
	double V_B = 0;
	double E_T = 0;
	double V_T = 0;
	VarianceAndMean(lon, V_T, E_T);
	VarianceAndMean(loff, V_B, E_B);

	//calculate variance and mean of single PE response function psi
	double E_psi = (E_T - E_B)/lambda_hat;
	double V_psi = (V_T - V_B)/lambda_hat - E_psi*E_psi;
	cout << "V_T = " << V_T << " , V_B = " << V_B << endl;

	cout << " Mean and variance of spe distribution " << E_psi << " , " << V_psi << endl;

	//calculate statistical uncertainties
	double V_psi_unc = pow(E_psi*E_psi - V_psi, 2)*(exp(lambda_hat) + 1 - 2*_threshold_fraction)/(_threshold_fraction*pow(lambda_hat,2)*on_trigs);
	double E_psi_unc = (lambda_hat*(E_psi*E_psi + V_psi) + 2*V_B)/(on_trigs*lambda_hat*lambda_hat) + \
				pow(E_psi, 2)*(exp(lambda_hat) + 1 - 2*_threshold_fraction)/(_threshold_fraction*pow(lambda_hat,2)*on_trigs);


	double npe = abs(c)/E_psi;
	double v_npe = npe*V_psi;
	cout << "Estimating " << npe << " number of photoelectrons for charge " << abs(c) << endl;


	return npe;
}

void EstimateNPE::VarianceAndMean(vector<double> a, double& V, double& E)
{
	double sum = std::accumulate(a.begin(), a.end(), 0.0);
	E = sum / a.size();

	double tempsum = 0;
	double var = 0;
	vector<double>:: iterator vitr;
	for(vitr = a.begin(); vitr != a.end(); ++vitr)
	{
		tempsum += pow(*vitr - E, 2);
	}
	V = tempsum/(a.size() - 2);

	return;
}


bool EstimateNPE::Finalise(){

  return true;
}
