#include "xml.h"

map<TString,TString> xml2map(TString sname) {

// Reading in a xml file and translating it into a map

  TXMLEngine* xml=0;
  XMLNodePointer_t mainnode =  0;
  XMLDocPointer_t xmldoc = 0;
  XMLNodePointer_t child;
  XMLAttrPointer_t attr = 0;
  TString svar,content,satt_val,satt_nam,sval;
  map<TString,TString> map_loc;
  int jattr;

  cout << "Loading " << sname << endl;

  xmldoc = xml->ParseFile(sname);
  if (xmldoc==0){
    delete xml;
    return map_loc;
  }

  mainnode      = xml->DocGetRootElement(xmldoc);

// display all child nodes   
  child = xml->GetChild(mainnode);
  while (child!=0){
    svar          = xml->GetNodeName(child);
    map_loc[svar] = xml->GetNodeContent(child);
    child         = xml->GetNext(child);
  }
  xml->FreeDoc(xmldoc);
  delete xml;
  return map_loc;
}

void map2xml(TString sname, map<TString, TString> map_loc) {

// Stores the map "map_loc" (representing the current GUI entries values)
// into the xml file named "sname"

  TXMLEngine* xml=0;
  XMLDocPointer_t xmldoc = 0;
  XMLNodePointer_t mainnode =  0;
  map<TString, TString>:: iterator siter;
  TString clbl;
  TString cval;
  XMLNodePointer_t xnode;
  mainnode = xml->NewChild(0, 0, "main");

  cout << "Storing " << sname << endl;

  for (siter = map_loc.begin(); siter != map_loc.end(); ++siter){
    clbl=siter->first;
    cval=siter->second;
    xnode = xml->NewChild(mainnode, 0, clbl,cval);
  }

  xmldoc = xml->NewDoc();
  xml->DocSetRootElement(xmldoc, mainnode);
  xml->SaveDoc(xmldoc, sname);

  xml->FreeDoc(xmldoc);
  delete xml;
}
