#ifndef LAPPDIntegratePulse_H
#define LAPPDIntegratePulse_H

#include <string>
#include <iostream>

#include "Tool.h"
#include "NnlsSolution.h"

class LAPPDIntegratePulse: public Tool {


 public:

  LAPPDIntegratePulse();
  bool Initialise(std::string configfile,DataModel &data);
  bool Execute();
  bool Finalise();


 private:

   double CalcIntegral(Waveform<double> hwav, double lowR, double hiR);
   void RawConstRangeIntegral();
   void TotalChargeFromNNLS(map<unsigned long, NnlsSolution> nnlssolns);
   int DimSize;
   double Deltat;
   double lowR;
   double hiR;
   string storename;
   double termination; //termination resistance in ohms

};


#endif
