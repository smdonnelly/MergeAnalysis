#include "NnlsSolution.h"

NnlsSolution::~NnlsSolution()
{
	component_times.clear();
	component_scales.clear();
	templateWaveform.ClearSamples(); //template used in the nnls algo, at the time binning used in the algorithm
	templateTimes.clear(); //time series of length of templateWaveform
	fullNnlsWaveform.ClearSamples(); //the full solution, sum of all components
}

//add an nnls component to the components vector. 
//each of these components is a copy of the template waveform
//at time "t" and scaled with factor "s". 
void NnlsSolution::AddComponent(double t, double s)
{
	component_times.push_back(t);
	component_scales.push_back(s);
}

//sum of all components, full waveform fit
void NnlsSolution::SetFullSoln(Waveform<double> fullsoln)
{
	fullNnlsWaveform.SetSamples(fullsoln.Samples());
}

void NnlsSolution::SetTemplate(Waveform<double> tmpwfm, vector<double> temptimes)
{
	templateWaveform.SetSamples(tmpwfm.Samples());
	templateTimes = temptimes;
}


//assumes that the full solution is a constant timestep
//timing based on the timestep of the template. 
//Returns a vector representing the sample times of
//the full solution waveform (len of full soln = len of returned vector)
vector<double> NnlsSolution::GetFullSolutionTimes()
{
	vector<double> fullwfm_times;
	if(templateTimes.empty())
	{
		cerr << "cannot get template times from NnlsSolution object. No template saved" << endl;
		return fullwfm_times;
	}

	double template_timestep = templateTimes.at(1) - templateTimes.at(0);

	for(int i = 0; i < (int)fullNnlsWaveform.GetSamples()->size(); i++)
	{
		fullwfm_times.push_back(i*template_timestep);
	}

	return fullwfm_times;
}