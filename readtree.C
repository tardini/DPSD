#include <string>

void readtree(){
    
  Int_t neutflag, nx, marker, ledref;
  Double_t tim, intshort, intlong, inttot, inttot2;
  string *tfile, *dfile, *lfile;
  //    TString *lfile;
    TFile *file_uni = TFile::Open("29795test.root","READ","BIN FILE OUTPUT",1);

    TTree *pha = (TTree *)file_uni->Get("pha_tree");
    TTree *head = (TTree *)file_uni->Get("head_tree");

    //    lfile=0;
    head->SetBranchAddress("nx",&nx);
    head->SetBranchAddress("marker",&marker);
    head->SetBranchAddress("led_ref_ch",&ledref);
    head->SetBranchAddress("lfile", &lfile);
    head->SetBranchAddress("tfile", &tfile);
    head->SetBranchAddress("dfile", &dfile);
    head->GetEntry();
    cout << nx << " " << marker << " " << ledref << endl;
    cout << *lfile << endl;
    cout << *tfile << endl;
    cout << *dfile << endl;

    pha->SetBranchAddress("time",&tim);
    pha->SetBranchAddress("int_short",&intshort);
    pha->SetBranchAddress("int_long",&intlong);
    pha->SetBranchAddress("int_tot", &inttot);
    pha->SetBranchAddress("int_tot_raw",&inttot2);
    pha->SetBranchAddress("neut_flag",&neutflag);
    Long64_t nentries = pha->GetEntries();
    cout << endl << nentries << endl;
    cout << endl;
    for (Int_t i=nentries-5; i < nentries; i++) {
      pha->GetEntry(i);
      cout << tim << " " << intshort << " " << neutflag <<  " " << inttot <<  " " << inttot2 << endl;
    }
}
