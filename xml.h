#include <TROOT.h>
#include <Riostream.h>
#include <TXMLEngine.h>
#include <TString.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <map>
using namespace std;

map<TString, TString> xml2map(TString);
void map2xml(TString, map<TString, TString>);
