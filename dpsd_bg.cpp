#include "dpsd_run.h"
#include <stdlib.h>
#include "Riostream.h"

int main(int argc, const char* argv[] ) {

    TString xml_dir, xmlfile, s_exp;
    Int_t nacq;
    Bool_t wsf=true, wrt=false;

    s_exp =  "AUGD";
    xml_dir.Form("%s/DPSD/xml/", getenv("HOME"));
    if(argc > 1){
      nacq = atoi(argv[1]);
    }
    else{
        cout << "Please enter #acq" << endl;
        cin >> nacq;
    }
    if(argc > 2){
        xmlfile = xml_dir.Data() + (TString)argv[2];
    }
    else{
        xmlfile = "shot.xml";
        xmlfile = xml_dir + xmlfile;
    }
    cout << nacq << " " << xmlfile.Data() << endl;
    dpsd_run(nacq, xmlfile, wsf, wrt, s_exp);
}
