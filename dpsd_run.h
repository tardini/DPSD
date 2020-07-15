#include "TString.h"
#include <map>

using namespace std;


extern short *flg_npeaks, *pul_len, *flg_led, *flg_ledpks, *flg_neut, *flg_sat;
extern double *DDATA, *t_dpsd;
extern double *PulseShape, *PulseHeight;
extern int jtbeg, nxCh, nyCh;
extern int totallength, n_TimeBins, n_TimeLED, n_pulses, pulse_len;
extern double TimeBin;
extern Int_t xWinLen;

extern map<TString, Int_t*> phs;
extern map<TString, Double_t*> cnt;

void dpsd_run(Int_t n_acq, TString xmlfile, Bool_t wsf, Bool_t wrt, TString s_exp);
void ng_sep();
void StoreSF(TString s_exp);
void StoreRoot();
void PUTLOG (bool puttime, TString logstr);
