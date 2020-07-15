#include "dpsd_run.h"
#include "dpsd_gui.h"
#include "xml.h"
#include <math.h>
#include <stdlib.h>
#include <iomanip>
#include <string>
#include <map>
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
#include "TSystem.h"
#include "ddwwansic8.h"
#include "ansisfh8.h"

using namespace std;

short *flg_npeaks, *pul_len, *win_len, *flg_led, *flg_ledpks, *flg_neut;
short  *flg_sat, *flg_DD, *flg_DT;
double *DDATA, *t_dpsd;
double *PulseShape, *PulseHeight;
int jtbeg, jtend, nxCh, nyCh;
int totallength, n_TimeBins, n_TimeLED, FileLength, n_pulses, pulse_len, newpulse_len;
double TimeBin, pup_frac;
TString Lfilename, Tfilename, Dfilename;
Int_t xWinLen;

map<TString, Int_t*> phs;
map<TString, Double_t*> cnt;


short *LENGTHES;
//int *LENGTHES;
double *TIMES, *ShortIntegral, *LongIntegral, *TotalIntegral, *TotalIntegralRaw;
Int_t nDD, nDT, nGam, n_acq, n_acq100;
map<TString, TString> setup_typ, setup_lbl;
map<TString, bool>    map_bool;
map<TString, int>     map_int;
map<TString, double>  map_dbl;
map<TString, TString> map_str;

/*
    Print out (with/without time)
*/
void PUTLOG (bool puttime, TString logstr) {
    time_t t;
    time(&t);
    char *ctime(const time_t *time);
    if (puttime){
        cout << "\t" << ctime(&t);
    }
    cout << "\t" << logstr << endl;
}

/*
    Returns trapezoidal integral
*/
double integrate (double *xPulse, int begin, int end) {

    double integr = 0;
    for (int j=begin; j<end-1; j++) {
        integr += xPulse[j]+xPulse[j+1];
    }
    return integr/2;
}


/*
    Averages vector xPulse between begin and end
*/
double ArrayAverage(double *xPulse, int begin, int end){
    double average = 0;
    for (int j=begin; j<end; j++){
        average += xPulse[j];
    }
    return average/(double)(end-begin);
}

/*
    Peak algorithm
*/
int PeakAlgo(int xPeakAlgorythm, int xFront, int xTail, int xThreshold, double *xPulse, int xPulseLen){

    int PileOut = 0;
    int pulsewidth = xFront+xTail;
    int jj, j, jt;
    Double_t maxv;
    Double_t epsilon = 0.001;

    switch (xPeakAlgorythm){
        case 0 :
            for (j=xFront+2; j<xPulseLen-2*pulsewidth-1; j++){
                if (xPulse[j] > xThreshold){
                    jj = 0;
                    maxv = 0.;
                    for ( jt=j; jt<j+pulsewidth; jt++);{
                        if (maxv < xPulse[jt]){
                            maxv = xPulse[jt];
                            jj = jt;
                        }
                    }
                    if (( ArrayAverage(xPulse, jj-xFront-2, jj-xFront) < xThreshold ) &&
                        ( ArrayAverage(xPulse, jj-xFront, jj+xTail) > xThreshold ) &&
                        ( ArrayAverage(xPulse, jj+xTail, jj+pulsewidth) < xThreshold )){
                        PileOut++;
                        j = jj+pulsewidth;
                    }
                }
            }
            break;
        case 1 :
            for (j=xFront; j<xPulseLen-xTail; j++){
                if ( ArrayAverage(xPulse, j, j+xTail) -
                     ArrayAverage(xPulse, j-xFront, j) > xThreshold ){
                    PileOut++;
                    j += xTail;
                }
             }
            break;
        case 2 :
            for (j=0; j<xPulseLen-xFront-1; j++){
                if (xPulse[j+xFront]-xPulse[j] > xThreshold){
                    PileOut++;
                    j += xFront+1;
                }
            }
            break;
        case 3 :
            for (j=0; j<xPulseLen-pulsewidth; j++){
                if ((xPulse[j+xFront] - xPulse[j] > xThreshold + epsilon) &&
                    (xPulse[j+xFront] - xPulse[j+pulsewidth] > xThreshold + epsilon)){
                    PileOut++;
                    j += pulsewidth;
                }
            }
            break;
    }
    return PileOut;
}


void load_xml(TString sname){

    TString clbl, stype;
    TString cval;
    int val;
    double dval;
    map<TString, TString>:: iterator siter;
    map<TString, TString> map_xml;

    map_xml = xml2map(sname);

    for (siter=map_xml.begin(); siter != map_xml.end(); ++siter){
        clbl = siter->first;
        cval = siter->second;
        stype = setup_typ[clbl];
        if (stype == "bool"){
            if (cval == "true"){
                map_bool[clbl] = true;
            }
            else{
                map_bool[clbl] = false;
            }
        }
        if (stype == "int"){
            map_int[clbl] = atoi(cval);
        }
        if (stype == "double"){
            map_dbl[clbl] = atof(cval);
        }
        if (stype == "combo"){
            map_int[clbl] = atoi(cval);
        }
        if (stype == "string"){
            map_str[clbl] = cval;
        }
    }
}

/*
    Read L-file containing the pulse lengths
*/
int rdLfile (void){

    FILE *f;
    f = fopen(Lfilename, "rb");
    if (f == NULL){
        cout << "\tCannot open " << Lfilename << endl;
        return 9;
    }
    fseek(f, 0, 2);
    int iFileLength = ftell(f);
    LENGTHES = new short[iFileLength/2];
    //    LENGTHES = new int[iFileLength/2];
    fseek(f, 0, 0);
    int iBytesRead = fread(LENGTHES, 1, iFileLength, f);
    fclose(f);
    FileLength = iBytesRead/2;

    return 0;
}

/*
 Read T-file containing the time stamps (it contains delta t's!)
*/
int rdTfile (void){
    FILE *f;
    int i;

    f = fopen(Tfilename, "rb");
    if (f == NULL){
        cout << "\tCannot open " << Tfilename << endl;
        return 9;
    }
    fseek(f, 0, 2);
    int iFileLength = ftell(f);
    unsigned int pszBuffer[iFileLength/4];
    TIMES = new double[iFileLength/4];
    fseek(f, 0, 0);
    int iBytesRead = fread(pszBuffer, 1, iFileLength, f);
    fclose(f);
    TIMES[0] = pszBuffer[0]*1E-8;
    for (i=1; i<iBytesRead/4; i++){
        TIMES[i] = TIMES[i-1] + pszBuffer[i]*1E-8;
    }
    return 0;
}

/*
    Initialises arrays
*/
void Init_cnt(){

    int jstr, j_tim;
    TString clbl;
    const char* cnt_label[] = {"Time", "PileUp", "Single", "Neut",
                               "nDD", "Gamma", "gDD", "nDT", "gDT", 
                               "LED", "Uthres", "Neut2", "Gamma2", 0};

    cout << "Init_cnt" << endl;
    for (jstr=0; cnt_label[jstr]; jstr++){
        clbl = cnt_label[jstr];
        cnt[clbl] = new Double_t[n_TimeBins];
        for (j_tim=0; j_tim<n_TimeBins; j_tim++){
            if(clbl == "Time"){
                cnt[clbl][j_tim] = ((Double_t)j_tim+0.5)*TimeBin + t_dpsd[0];
            }
            else{
                cnt[cnt_label[jstr]][j_tim]=0;
            }
        }
    }
}

void Init_sep(){

    int jstr, j_ch, j_pul;
    TString clbl;
    const char* phs_label[4] = {"Gamma", "Neut", "LED", 0};

    for(jstr=0; phs_label[jstr]; jstr++){
        clbl = phs_label[jstr];
        phs[clbl] = new int [nxCh];
        for (j_ch=0; j_ch< nxCh; j_ch++){
            phs[clbl][j_ch] = 0;
        }
    }

    flg_neut = new short[n_pulses];
    flg_DD   = new short[n_pulses];
    flg_DT   = new short[n_pulses];
    for(j_pul=0; j_pul<n_pulses; j_pul++){
        flg_neut[j_pul] = 0;
        flg_DD[j_pul]   = 0;
        flg_DT[j_pul]   = 0;
    }
}

void InitialiseArrays(){

    int j_tim, j_pul;
    t_dpsd           = new double[n_pulses];
    ShortIntegral    = new double[n_pulses];
    LongIntegral     = new double[n_pulses];
    TotalIntegral    = new double[n_pulses];
    TotalIntegralRaw = new double[n_pulses];
    PulseShape       = new double[n_pulses];
    PulseHeight      = new double[n_pulses];
    flg_sat    = new short[n_pulses];
    flg_npeaks = new short[n_pulses];
    pul_len    = new short[n_pulses];
    win_len    = new short[n_pulses];
    flg_led    = new short[n_pulses];
    flg_ledpks = new short[n_pulses];
    cnt["PMgain"]  = new Double_t [n_TimeLED];
    cnt["LEDch"]   = new Double_t [n_TimeLED];
    cnt["TimeLED"] = new Double_t [n_TimeLED];

    nDD = 0;
    nDT = 0;
    nGam = 0;

    for (j_tim=0; j_tim<n_TimeLED; j_tim++){
        cnt["PMgain"] [j_tim] = 0.;
        cnt["LEDch"]  [j_tim] = 0.;
        cnt["TimeLED"][j_tim] = ((Double_t)j_tim+0.5)*map_dbl["LEDdt"] + t_dpsd[0];
    }
    for (j_pul=0; j_pul<n_pulses; j_pul++){
        ShortIntegral   [j_pul] = 0;
        LongIntegral    [j_pul] = 0;
        TotalIntegral   [j_pul] = 0;
        TotalIntegralRaw[j_pul] = 0;
        PulseShape      [j_pul] = 0;
        PulseHeight     [j_pul] = 0;
        flg_sat   [j_pul] = 0;
        flg_npeaks[j_pul] = 0;
        pul_len   [j_pul] = 0;
        win_len   [j_pul] = 0;
        flg_led   [j_pul] = 0;
        flg_ledpks[j_pul] = 0;
    }

}

int rddata (int K, int ntimes){
 
    FILE *f;
    int i;
    int shift=0;
    for (i=0; i<K; i++){
        shift += LENGTHES[i];
    }
    totallength = 0;
    for (i=0; i<ntimes; i++){
        totallength += LENGTHES[i+K];
    }
    cout << "First window length:" << LENGTHES[0] << " Last:" << LENGTHES[K+ntimes-1] << endl;
    short* pszBuffer = new short[totallength];
    DDATA = new double[totallength];
    f = fopen(Dfilename, "rb");
    if (f == NULL){
        cout << "\tcannot open " << Dfilename << endl;
        return 9;
    }
    fseek(f, 2*shift, 0);
    fread(pszBuffer, 1, 2*totallength, f);
    fclose(f);
    for (i=0; i<totallength; i++){
        DDATA[i] = -pszBuffer[i];
    }
    return 0;
}


/*
    Evaluate count rates: total; pile-up; single; total neutron; total gamma; 
    neutron and gammas within DD and DT windows; pile up correction factor;
    LED; underthreshold.
    The LED average channel in the time bin is also stored
*/
void CountRates(){

    int jt_loc = 0;
    int jstr, j_pul, j_tim;

    PUTLOG(0, "Count Rates Calculation");
    Init_cnt();

    for (j_pul=0; j_pul<n_pulses; j_pul++){
        jt_loc = int((t_dpsd[j_pul]-t_dpsd[0])/TimeBin);
        if (jt_loc > jtend-1){
            jt_loc = jtend-1;
        }
        if (flg_led[j_pul] == 0){
            if (flg_npeaks[j_pul] > 1){
                cnt["PileUp"][jt_loc] += 1;
	    }
            if (flg_npeaks[j_pul] == 1){
                cnt["Single"][jt_loc]++;
                if (flg_neut[j_pul] == 1){
                    cnt["Neut"][jt_loc]++;
                    if (flg_DD[j_pul] == 1){
                        nDD++;
                        cnt["nDD"][jt_loc]++;
                    }
                    if (flg_DT[j_pul] == 1){
                        nDT++;
	                cnt["nDT"][jt_loc]++;
                    }
                }
                if (flg_neut[j_pul] == 0){
		    nGam++;
                    cnt["Gamma"][jt_loc]++;
                }
            }
            if (flg_npeaks[j_pul] == 0){
                cnt["Uthres"][jt_loc]++;
            }
        }
	else{
	    cnt["LED"][jt_loc]++;
	}
    }
    const char* cnt_label[] = {"PileUp", "Single", "Neut", "nDD", "Gamma",
	                       "gDD", "nDT", "gDT", "LED", "Uthres", 0};
    for (j_tim=0; j_tim<n_TimeBins; j_tim++){
        for (jstr=0; cnt_label[jstr]; jstr++){
            cnt[cnt_label[jstr]][j_tim] /= TimeBin;
        }

// Re-distribute pile-ups as events for count rate
        pup_frac = 0;
        double total = cnt["Neut"][j_tim] + cnt["Gamma"][j_tim] + cnt["LED"][j_tim];
        if (total > 0){
            pup_frac = 1 + 2.*cnt["PileUp"][j_tim]/total;
            cnt["Neut2"][j_tim]  = cnt["Neut"] [j_tim]*pup_frac;
            cnt["Gamma2"][j_tim] = cnt["Gamma"][j_tim]*pup_frac;
        }
        else{
            cnt["Neut2"][j_tim]  = 0.;
            cnt["Gamma2"][j_tim] = 0.;
        }
    }
}


void StoreSF(TString s_exp){

    Int_t j_tim, jstr;
    uint32_t shot=n_acq, stride=1, length, typ=2, control=3;
    int32_t err=0, diaref=0, ed=-1;
    char exp[8], diag[4], tname[9], tim[18], mode[4],
      mode1[5], mode2[9], dummy[2], sfh[120];
    char* sgname;
    map<TString, float*> sf;
    TString ssfh;
    int32_t sfhref=0;
    const char* sig_label[] = {"neut1", "neut2", "neutD-T", "gamma1", "gamma2", "pileup", "led", 0};

    strcpy(exp, s_exp);
    strcpy(diag,"NSP");
    strcpy(tname,"time");
    strcpy(mode,"new");
    strcpy(dummy," ");
    strcpy(mode1,"lock");
    strcpy(mode2,"maxspace");
    //    ssfh.Form("%s/DPSD/NSP00000.sfh", getenv("HOME"));
    ssfh = "NSP00000.sfh";
    strcpy(sfh, ssfh);

    cout << endl << "Storing shotfile, exp=" << exp << endl << endl;

    cout << "SFH file " << ssfh << " setting n_tim=" << n_TimeBins << endl;

// Modifying length of time array in shotfile header

    length = n_TimeBins;
    sfhopen (sfh, &sfhref);
    sfherror(err, dummy);
    sfhmodtim (sfhref, tname, length);
    for (jstr=0; sig_label[jstr]; jstr++){
        sgname = strdup(sig_label[jstr]);
        sf[sgname] = new float[n_TimeBins];
        sfhmodtim (sfhref, sgname, length);
        sfherror(err, dummy);
    }
    sfhclose (sfhref);

    sf["time"] = new float[n_TimeBins];

    for (j_tim=0; j_tim<n_TimeBins; j_tim++){
        sf["time"][j_tim]   = cnt["Time"][j_tim];
        sf["neut1"][j_tim]  = cnt["Neut"][j_tim];
        sf["neut2"][j_tim]  = cnt["Neut2"][j_tim];
        sf["neutD-T"][j_tim]= cnt["nDT"][j_tim];
        sf["gamma1"][j_tim] = cnt["Gamma"][j_tim];
        sf["gamma2"][j_tim] = cnt["Gamma2"][j_tim];
        sf["led"][j_tim]    = cnt["LED"][j_tim];
        sf["pileup"][j_tim] = cnt["PileUp"][j_tim];
    }

// Writing shotfile

    wwopen_ (&err, exp, diag, &shot, mode, &ed, &diaref, tim, (uint64_t)4, (uint64_t)3, (uint64_t)3, (uint64_t)18);
    if (err != 0) xxerror_ (&err, &control, dummy, (uint64_t)1);

    wwtbase_ (&err, &diaref, tname, &typ, &length, sf["time"], &stride, (uint64_t)8);
    if (err != 0) xxerror_ (&err, &control, dummy, 1);

    for (jstr=0; sig_label[jstr]; jstr++){
        sgname = strdup(sig_label[jstr]);
        wwsignal_ (&err, &diaref, sgname, &typ, &length, sf[sgname], &stride, (uint64_t)8);
        if (err != 0) xxerror_ (&err, &control, dummy, (uint64_t)1);
    }

    wwclose_ (&err, &diaref, mode1, mode2, (uint64_t)4, (uint64_t)8);
    if (err != 0){
        xxerror_ (&err, &control, dummy, (uint64_t)1);
    }
    else{
      cout << "Stored NSP shotfile, shot=" << n_acq << "  exp=" << exp << "  ed=" << ed << endl;
    }

    return;
}

/*
    Stores ROOT file with 4 TTrees:
    - header (user optins, GUI entries)
    - pulse info
    - PHS spectra
    - count rates
*/
void StoreRoot(){

    TString clbl, clbl1;
    int j_pul, j_ch, j_tim, jstr;
    Short_t pul_length, new_length, led_flag,sat_flag, neut_flag, dd_flag, dt_flag, num_pulses;
    Double_t int_short, int_long, int_tot, int_tot_raw;
    Double_t time;
    Double_t LED_Factor;

// PHA root file

    TString rootfile;
    rootfile.Form("%s/DPSD/output/%idpsd.root", getenv("HOME"), n_acq);

    cout << endl << "Storing " << rootfile.Data() << endl << endl;
    TFile *out_root = new TFile(rootfile, "RECREATE", "BIN FILE OUTPUT", 1);
    TTree *head_tree = new TTree("head_tree", "Run info");
    TTree *pul_tree  = new TTree("pul_tree" , "Pulses info");
    TTree *cnt_tree  = new TTree("cnt_tree" , "Count rates");
    TTree *phs_tree  = new TTree("phs_tree" , "PHS");
    string lfile, tfile, dfile;

// Run info

    const char* branch_list[] = {
        "LEDcorrection", "SubtBaseline", "DDlo", "DDup", "DTlo", "DTup",
        "LEDFront", "LEDTail", "LEDreference", "LEDxmin", "LEDxmax",
        "LEDymin", "LEDymax", "LineChange", "LongGate", "ShortGate", "Marker",
        "SaturationHigh", "SaturationLow",
        "BaselineStart", "BaselineEnd", "Threshold", "Front", "Tail",
        "xChannels", "yChannels", "ToFWindowLength",
        "MaxDifference", "Slope1", "Slope2", "Offset", "TBeg", "TEnd", "TimeBin",
        "LEDdt", "TotalGate", "PeakAlgorythm",
        0
    };

    cout << "Storing header" << endl;

    head_tree->Branch("lfile", &lfile);
    head_tree->Branch("tfile", &tfile);
    head_tree->Branch("dfile", &dfile);

    for (jstr=0; branch_list[jstr]; jstr++){
        clbl = branch_list[jstr];
        clbl1.Form("%s/I", clbl.Data());
        if (setup_typ[clbl] == "bool"){
            Int_t entry = map_bool[clbl];
            head_tree->Branch(clbl.Data(), &entry, clbl1.Data());
        }
        if (setup_typ[clbl] == "int"){
            head_tree->Branch(clbl.Data(), &map_int[clbl], clbl1.Data());
        }
        if (setup_typ[clbl] == "combo"){
            head_tree->Branch(clbl.Data(), &map_int[clbl], clbl1.Data());
        }
        if (setup_typ[clbl] == "double"){
            clbl1.Form("%s/D", clbl.Data());
            head_tree->Branch(clbl.Data(), &map_dbl[clbl], clbl1.Data());
        }
    }

    lfile = Lfilename;
    tfile = Tfilename;
    dfile = Dfilename;
    head_tree->Fill();
    head_tree->Write();
    head_tree->Delete();

// Pulses characteristics

    cout << "Storing pulse characteristics" << endl;

    pul_tree->Branch("time"       , &time       , "time/D");
    pul_tree->Branch("int_short"  , &int_short  , "int_short/D");
    pul_tree->Branch("int_long"   , &int_long   , "int_long/D");
    pul_tree->Branch("int_tot"    , &int_tot    , "int_tot/D");
    pul_tree->Branch("int_tot_raw", &int_tot_raw, "int_tot_raw/D");
    pul_tree->Branch("sat_flag"   , &sat_flag   , "sat_flag/S");
    pul_tree->Branch("num_pulses" , &num_pulses , "num_pulses/S");
    pul_tree->Branch("pul_length" , &pul_length , "pul_length/S");
    pul_tree->Branch("new_length" , &new_length , "new_length/S");
    pul_tree->Branch("led_flag"   , &led_flag   , "led_flag/S");
    pul_tree->Branch("neut_flag"  , &neut_flag  , "neut_flag/S");
    pul_tree->Branch("dd_flag"    , &dd_flag    , "dd_flag/S");
    pul_tree->Branch("dt_flag"    , &dt_flag    , "dt_flag/S");
    for (j_pul=0; j_pul<n_pulses; j_pul++){
        time = t_dpsd[j_pul];
        int_short  = ShortIntegral[j_pul];
        int_long   = LongIntegral[j_pul];
        int_tot    = TotalIntegral[j_pul];
        int_tot_raw= TotalIntegralRaw[j_pul];
        sat_flag   = flg_sat[j_pul];
        num_pulses = flg_npeaks[j_pul];
        pul_length = pul_len[j_pul];
        new_length = win_len[j_pul];
        neut_flag  = flg_neut[j_pul];
        dd_flag    = flg_DD[j_pul];
        dt_flag    = flg_DT[j_pul];
        led_flag   = flg_led[j_pul];
        pul_tree->Fill();
    }
    pul_tree->Write();
    pul_tree->Delete();

// PHS

    cout << "Storing PHS" << endl;

    Double_t phs_gamma, phs_neut;

    phs_tree->Branch("gamma", &phs_gamma, "gamma/D");
    phs_tree->Branch("neut" , &phs_neut , "neut/D");
    for (j_ch=0; j_ch<nxCh; j_ch++){
        phs_gamma = phs["Gamma"][j_ch];
        phs_neut  = phs["Neut"][j_ch];
        phs_tree->Fill();
    }
    phs_tree->Write();
    phs_tree->Delete();

// CNT

    cout << "Storing count rates" << endl;

    Double_t cnt_time, cnt_total, cnt_pileup, cnt_single, cnt_neut, cnt_nDD;
    Double_t cnt_nDT, cnt_gamma, cnt_uthres, cnt_LED, cnt_tLED, cnt_LEDch, cnt_PMgain;

    cnt_tree->Branch("time"  , &cnt_time  , "time/D");
    cnt_tree->Branch("pileup", &cnt_pileup, "pileup/D");
    cnt_tree->Branch("single", &cnt_single, "single/D");
    cnt_tree->Branch("neut"  , &cnt_neut  , "neut/D");
    cnt_tree->Branch("nDD"   , &cnt_nDD   , "nDD/D");
    cnt_tree->Branch("nDT"   , &cnt_nDT   , "nDT/D");
    cnt_tree->Branch("gamma" , &cnt_gamma , "gamma/D");
    cnt_tree->Branch("uthres", &cnt_uthres, "uthres/D");
    cnt_tree->Branch("LED"   , &cnt_LED   , "LED/D");
    cnt_tree->Branch("timeLED",&cnt_tLED  , "timeLED/D");
    cnt_tree->Branch("LEDch" , &cnt_LEDch , "LEDch/D");
    cnt_tree->Branch("PMgain", &cnt_PMgain, "PMgain/D");

    for (j_tim=0; j_tim<n_TimeBins; j_tim++){
        cnt_time   = cnt["Time"][j_tim];
        cnt_pileup = cnt["PileUp"][j_tim];
        cnt_single = cnt["Single"][j_tim];
        cnt_neut   = cnt["Neut"][j_tim];
        cnt_nDD    = cnt["nDD"][j_tim];
        cnt_nDT    = cnt["nDT"][j_tim];
        cnt_gamma  = cnt["Gamma"][j_tim];
        cnt_uthres = cnt["Uthres"][j_tim];
        cnt_LED    = cnt["LED"][j_tim];
        cnt_tree->Fill();
    }
    for (j_tim=0; j_tim<n_TimeLED; j_tim++){
        cnt_tLED   = cnt["TimeLED"][j_tim];
        cnt_LEDch  = cnt["LEDch"] [j_tim];
        cnt_PMgain = cnt["PMgain"][j_tim];
        cnt_tree->Fill();
    }
    cnt_tree->Write();
    cnt_tree->Delete();

// Close TFile

    out_root->Close();

    return;

}

/*
Perform neutron-gamma separation based on pulse shape
*/
void ng_sep(){

  int j_pul, j_ch;

    Double_t* Separatrix = new Double_t[nxCh];

    Separatrix[0] = map_dbl["Offset"];
    for (j_ch=1; j_ch<=map_int["LineChange"]; j_ch++){
        Separatrix[j_ch] = Separatrix[j_ch-1] + map_dbl["Slope1"];
    }
    for (j_ch=map_int["LineChange"]+1; j_ch<nxCh; j_ch++){
        Separatrix[j_ch] = Separatrix[j_ch-1] + map_dbl["Slope2"];
    }

    PUTLOG(0, "n-gamma SEPARATION");
    Init_sep();
    for (j_pul=0; j_pul<n_pulses; j_pul++){
        if ( (flg_sat[j_pul] == 0) && (flg_npeaks[j_pul] == 1) && (LongIntegral[j_pul] > 0) ){
            int nheight = (int)PulseHeight[j_pul];
            if ( (nheight>0) && (nheight<nxCh) ){
                if (PulseShape[j_pul] < Separatrix[nheight] ){
                    flg_neut[j_pul] = 1;
                }
                if ( (nheight >= map_int["DDlo"]) && (nheight < map_int["DDup"]) ){
                    flg_DD[j_pul]=1;
                }
                if ( (nheight >= map_int["DTlo"]) && (nheight < map_int["DTup"]) ){
                    flg_DT[j_pul]=1;
                }
                if (flg_led[j_pul]>0){
                    phs["LED"][nheight]++;
                }
                else{
                    if (flg_neut[j_pul]==1){
                        phs["Neut"][nheight]++;
                    }
                    else{
                        phs["Gamma"][nheight]++;
                    }
                }
            }
        }
    }

    CountRates();
}

/*
    Program steps:
    1) read parameters from GUI
    2) open data, time stamp and length files
    3) Evaluate the baseline if enabled
    4) Look for saturated pulses
    5) recognize single LED pulses and integrate them if LEDevaluation
    6) recognize single pulses and integrates them
    7) correct the    total integral of all pulses if LEDcorrection
    8) neutron/gamma separation
    9) Evaluate count rates
    10) Store output files
*/

void dpsd_run(Int_t nacq, TString xmlfile, Bool_t wsf, Bool_t wrt, TString s_exp){

    PUTLOG(1, "DPSD STARTED");

    n_acq = nacq;

    Int_t jstr;
    TString clbl;
    Double_t *pulse;

    const char* gui_entries[] = {
        "LEDcorrection", "bool",   "LED correction",
        "SubtBaseline",  "bool",   "Subtract baseline",
        "Path",          "string", "Base dir",
        "DDlo",          "int",    "DD min ch",
        "DDup",          "int",    "DD max ch",
        "DTlo",          "int",    "DT min ch",
        "DTup",          "int",    "DT max ch",
        "BaselineEnd",   "int",    "Baseline end",
        "Front",         "int",    "Front",
        "LEDFront",      "int",    "LED front",
        "LEDTail",       "int",    "LED tail",
        "LEDreference",  "int",    "LED ref ch",
        "LEDxmin",       "int",    "LED xmin in PHA",
        "LEDxmax",       "int",    "LED xmax in PHA",
        "LEDymin",       "int",    "LED ymin in PHA",
        "LEDymax",       "int",    "LED ymax in PHA",
        "LineChange",    "int",    "x of slope change",
        "LongGate",      "int",    "Long gate",
        "ShortGate",     "int",    "Short gate",
        "Marker",        "int",    "Marker",
        "SaturationHigh","int",    "Saturation high",
        "SaturationLow", "int",    "Saturation low",
        "BaselineStart", "int",    "Baseline start",
        "Tail",          "int",    "Tail",
        "Threshold",     "int",    "Threshold",
        "xChannels",     "int",    "# x bins",
        "yChannels",     "int",    "# y bins",
        "WindowLength",  "int",    "Window length",
        "ToFWindowLength","int",   "ToF window length",
        "PHSxmax",       "int",    "PHS graph xmax",
        "MaxDifference", "double", "Max difference",
        "Slope1",        "double", "1st slope",
        "Slope2",        "double", "2nd slope",
        "Offset",        "double", "Offset 1st line",
        "TBeg",          "double", "Time beg",
        "TEnd",          "double", "Time end",
        "TimeBin",       "double", "Time sampling",
        "LEDdt",         "double", "LED averaging time",
        "TotalGate",     "combo",  "Gate type",
        "PeakAlgorythm", "combo",  "Peak Algorythm",
        0
    };

    for (jstr = 0; gui_entries[jstr]; jstr++){
        clbl = gui_entries[jstr];
        jstr++;
        setup_typ[clbl] = gui_entries[jstr];
        jstr++;
        setup_lbl[clbl] = gui_entries[jstr];
    }

    double MDiff = fabs(map_dbl["MaxDifference"]);
    double average, average1, average2;
    double LEDaver;

    int LEDamount=0, LEDsumm=0;
    int j, j_pul, j_pula, jt, jtLED, jtLED_old = -1, jj;
    int isumm = 0;
    int jtmark=0, maxpos, max_SG, max_LG;

    delete [] DDATA;

    load_xml(xmlfile);
    n_acq100 = int(n_acq/100);
    Lfilename.Form("%s/%i/%i/%il.bin", map_str["Path"].Data(), n_acq100, n_acq, n_acq);
    Tfilename.Form("%s/%i/%i/%it.bin", map_str["Path"].Data(), n_acq100, n_acq, n_acq);
    Dfilename.Form("%s/%i/%i/%i.bin" , map_str["Path"].Data(), n_acq100, n_acq, n_acq);
    nxCh = map_int["xChannels"];
    nyCh = map_int["yChannels"];
    xWinLen = map_int["WindowLength"]; // just for plotting
    TimeBin = map_dbl["TimeBin"];

    int BLstart = map_int["BaselineStart"];

// Read input

    PUTLOG(0, "Opening L-File "+Lfilename);
    if (rdLfile() == 9){
        return;
    }
    PUTLOG(0, "Opening T-File "+Tfilename);
    if (rdTfile() == 9){
        return;
    }

    jtbeg = -1;
    jtend = FileLength;
    for (j_pul=1; j_pul<FileLength; j_pul++){
        if ((TIMES[j_pul] > map_dbl["TBeg"]) && (jtbeg == -1)){
            jtbeg = j_pul-1;
        }
        if ((TIMES[j_pul] > map_dbl["TEnd"]) && (jtend == FileLength)){
            jtend = j_pul;
            break;
        }
    }
    cout << "TBeg = TIMES[" << jtbeg << "] = " << TIMES[jtbeg] << endl;
    cout << "TEnd = TIMES[" << jtend << "] = " << TIMES[jtend] << endl;
    n_pulses = jtend-jtbeg+1;

    n_TimeBins = int( (TIMES[jtend] - TIMES[jtbeg])/ TimeBin);
    n_TimeLED  = int( (TIMES[jtend] - TIMES[jtbeg])/ map_dbl["LEDdt"]) + 1;
    cout << "n_TimeBins = " << n_TimeBins << endl;
    cout << "n_TimeLED  = " << n_TimeLED  << endl;

// Initialise arrays

    PUTLOG(0, "Initialising arrays");
    InitialiseArrays();
    Init_cnt();

// Selecting relevant time interval

    for(j_pul=0; j_pul<n_pulses; j_pul++){
        pul_len[j_pul] = LENGTHES[j_pul+jtbeg];
        t_dpsd[j_pul] = TIMES[j_pul+jtbeg];
    }

    PUTLOG(1, "Opening D-File "+Dfilename);
    if (rddata(jtbeg, n_pulses) == 9){
        exit(0);
    }

    PUTLOG(1, "Starting Time loop");
    cout << "# pulses = " << n_pulses << endl;

    double dxCh = (double)nxCh/(double)map_int["Marker"];

#ifdef GUI
    fProgrBar->SetRange((Float_t)0, (Float_t)(n_pulses-1));
#endif

//===========
// Time loop
//===========

    isumm = 0;
    for (j_pula=0; j_pula<n_pulses; j_pula++){
#ifdef GUI
        if(j_pula % 1000 == 0){
            fProgrBar->SetPosition((Float_t)j_pula);
            fProgrBar->ShowPosition(kTRUE,kTRUE, "%.0f pulses");
            gSystem->ProcessEvents();
        }
#endif
        pulse_len = pul_len[j_pula];
        pulse = new Double_t[pulse_len];
        for (jt=0; jt<pulse_len; jt++){
            if (jt > map_int["ToFWindowLength"] && map_int["ToFWindowLength"] > 0){
                pulse[jt] = 0.;
            }
            else{
                pulse[jt] = DDATA[isumm+jt];
            }
        }

/*
Baseline evaluation and subtraction
*/
        if (map_bool["SubtBaseline"]){
            average = 0;
            for (jt=0; jt<BLstart; jt++){
                average += pulse[jt];
            }
            for (jt=pulse_len-map_int["BaselineEnd"]; jt<pulse_len; jt++){
                average += pulse[jt];
            }
            average = average/( BLstart + map_int["BaselineEnd"] );
// Substracting average from DDATA segment
            for (jt=0; jt<pulse_len; jt++){
                pulse[jt] = pulse[jt]-average;
                DDATA[isumm+jt] = pulse[jt];
            }
        }

/*
Saturation Detection
*/
        for (jt=0; jt<pulse_len; jt++){
            if ((pulse[jt] > map_int["SaturationHigh"]) || (pulse[jt] < map_int["SaturationLow"])){
                flg_sat[j_pula] = 1;
                break;
            }
        }

/*
Integrals Counting
*/

        maxpos = TMath::LocMax(pulse_len, pulse);
        max_SG = TMath::Min(pulse_len, maxpos+map_int["ShortGate"]);
        max_LG = TMath::Min(pulse_len, maxpos+map_int["LongGate"]);

        switch (map_int["TotalGate"]){

            case 0:
                break;

            case 1:
                average1 = ArrayAverage(pulse, 0, BLstart);
                for (j=maxpos; j<pulse_len-BLstart; j++){
                    average2 = ArrayAverage(pulse, j, j+BLstart);
                    if ( fabs(average2-average1) < MDiff){
                        newpulse_len = BLstart/2 + j; //CUTTING THE REST
                        break;
                    }
                    if (j == pulse_len-BLstart-1){
                        newpulse_len = pulse_len - BLstart/2;
                    }
                }
                break;

            case 2:
                average1 = ArrayAverage(pulse, 0, BLstart);
                for (j=maxpos; j<pulse_len-BLstart; j++){
                    average2 = ArrayAverage(pulse, j, j+BLstart);
                    if (fabs(average2-average1) < MDiff){
                        if (max_LG > j+BLstart/2 ){
                            newpulse_len = max_LG;
                        }
                        else{
                            newpulse_len = BLstart/2 + j;
                        }     // CUTTING THE REST
                    break;
                    }
                    if (j == pulse_len-BLstart-1){
                        newpulse_len = pulse_len - BLstart/2;
                    }
                }
                break;

            case 3:
                ShortIntegral[j_pula] = 1;
                LongIntegral [j_pula] = 1;
                TotalIntegral[j_pula] = pulse[maxpos];
                newpulse_len = pulse_len;
                break;

            default: ;
        }

        if (map_int["TotalGate"] < 3){
            ShortIntegral[j_pula] = integrate (pulse, maxpos, max_SG);
            LongIntegral [j_pula] = integrate (pulse, maxpos, max_LG);
            TotalIntegral[j_pula] = integrate (pulse, 0, newpulse_len);
        }

        win_len[j_pula] = newpulse_len;

/*
LED evaluation
*/

        if (LongIntegral > 0){
            PulseHeight[j_pula] = TotalIntegral[j_pula]*dxCh;
            TotalIntegralRaw[j_pula] = TotalIntegral[j_pula];
            PulseShape[j_pula]    = ShortIntegral[j_pula]*(double)nyCh/LongIntegral[j_pula];
        }
        if (
	    //             (PulseHeight[j_pula] > map_int["LEDxmin"]) && (PulseHeight[j_pula] < map_int["LEDxmax"]) &&
             (PulseHeight[j_pula] > map_int["LEDxmin"]) &&
             (PulseShape[j_pula]  > map_int["LEDymin"]) && (PulseShape[j_pula]  < map_int["LEDymax"])
             ){
            flg_led[j_pula] = 1;
            flg_ledpks[j_pula] = PeakAlgo(map_int["PeakAlgorythm"], map_int["LEDFront"], map_int["LEDTail"], map_int["Threshold"], pulse, pulse_len);
        }

/*
Pile-up detection
*/

        if (flg_led[j_pula] > 0){
            flg_npeaks[j_pula] = flg_ledpks[j_pula];
        } 
        else{
            flg_npeaks[j_pula] = PeakAlgo(map_int["PeakAlgorythm"], map_int["Front"], map_int["Tail"], map_int["Threshold"], pulse, pulse_len);
        }

/*
LED correction
*/

        if (map_bool["LEDcorrection"]){
	    jtLED = (int)( (t_dpsd[j_pula]-t_dpsd[0])/map_dbl["LEDdt"]); 
            if (flg_ledpks[j_pula] == 1){
	      //            if (flg_led[j_pula] == 1){
                LEDsumm += TotalIntegral[j_pula] ;
                LEDamount ++ ;
            }
// New Time Bin
            if ( (jtLED > jtLED_old) || (j_pula == jtend) ) {
                if (LEDamount > 0){
                    LEDaver = (Double_t)LEDsumm/(Double_t)LEDamount;
                    cnt["LEDch"][jtLED] = (Double_t)LEDaver*dxCh;
                    if (LEDsumm > 0){
	                cnt["PMgain"][jtLED] = cnt["LEDch"][jtLED]/(Double_t)map_int["LEDreference"];
                    }
                }
                for (jt=jtmark; jt<j_pula; jt++){
                    TotalIntegral[jt] /= cnt["PMgain"][jtLED];
                    PulseHeight[jt]   /= cnt["PMgain"][jtLED];
                }

                jtmark = j_pula;
                LEDsumm   = 0;
                LEDamount = 0;
            }
// LED single pulse

            jtLED_old = jtLED;
        }

        isumm += pulse_len;
        delete [] pulse;
    }     // End of time loop over single pulses

#ifdef GUI
    fProgrBar->SetPosition((Float_t)(n_pulses-1));
    fProgrBar->ShowPosition(kTRUE,kTRUE, "%.0f pulses");
    gSystem->ProcessEvents();
#endif

    ng_sep();

    if(wrt){
        StoreRoot();
    }
    if(wsf){
        StoreSF(s_exp);
    }

    if(nDD > 0){
        cout << "n_DD = " << nDD << "   n_DT = " << nDT << "   nDT/nDD = " << (double)nDT/(double)nDD << endl;
        cout << "nGamma = " << nGam << endl;
    }
    PUTLOG(1, "\nDPSD COMPLETED\n");
    return;
}
