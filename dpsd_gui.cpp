#include "dpsd_gui.h"
#include "dpsd_run.h"
#include "xml.h"
#include "TGNumberEntry.h"
#include "TGFrame.h"
#include "TGButton.h"
#include "TGButtonGroup.h"
#include "TGLabel.h"
#include "TCanvas.h"
#include "TText.h"
#include "TList.h"
#include "TLegend.h"
#include "TApplication.h"
#include "TSystem.h"
#include "TFrame.h"
#include "TAxis.h"
#include "TGaxis.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TRootEmbeddedCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TStyle.h"
#include "TMath.h"
#include "TXMLEngine.h"
#include "TGFileDialog.h"
#include "TLatex.h"
#include "TSpectrum.h"
#include "TDirectory.h"
#include "TROOT.h"
#include "TGMenu.h"
#include "TGComboBox.h"
#include "TGProgressBar.h"
#include "TGStatusBar.h"
#include "TGSlider.h"
#include "TGToolBar.h"
#include "TGToolTip.h"
#include "TRootHelpDialog.h"
#include "TMessage.h"
#include "TFile.h"
#include "TTree.h"

#include <KeySymbols.h>
#include <math.h>
#include <stdlib.h>
#include <sstream>
#include <string>

map<TString, TString> setup_type, setup_label, map_xml;
map<TString, bool>    d_bool;
map<TString, int>     d_int;
map<TString, double>  d_dbl;
map<TString, TString> d_str;
map<TString, TGTextEntry*>   d_txt;
map<TString, TGNumberEntry*> d_num;
map<TString, TGComboBox*>    d_cmb;
map<TString, TGCheckButton*> d_ckb;
map<TString, TString> leg_lbl;
map<TString, Color_t> dpsdcol;

Bool_t pstop=true;
TString homedir = getenv("HOME");
TString graphdir = homedir + "/DPSD/graphics/";
TString xml_dir = homedir + "/DPSD/xml";
TGPictureButton *play_fig;
TGHSlider *hslider;

TString gif_rt   = graphdir + "root_save.gif";
TString gif_sf   = graphdir + "sf_save.gif";
TString gif_back = graphdir + "backward.gif";
TString gif_fw   = graphdir + "forward.gif";

const char* phs_label[4] = {"Gamma", "Neut", "LED", 0};
TString tslbl;

TGHProgressBar *fProgrBar;
TGStatusBar *fStatusBar;
TGLabel *prog_lbl;
TCanvas *cfit;
TH1F *hfit;
TSpectrum *sp;
dpsd_gui *main_frame;
TRootEmbeddedCanvas *fEC;
Int_t npul=0, nacq;

enum EMyMessageTypes{
    M_FILE_LOAD, M_FILE_SAVE, M_FILE_WSF, M_FILE_WROOT, M_FILE_EXE, M_FILE_EXIT,
    M_HELP_ABOUT,
    P_PULSE, P_TIME, P_WIN, P_GAIN, P_RUN, P_FWD, P_BACK,
    B_NG, B_PHA, SL_ID
};

// General plotting style
TStyle* DPSDstyle() {

    TStyle *dpsdSty = new TStyle("DPSD", "DPSD Styles");
    dpsdSty->SetCanvasColor(4000);
    dpsdSty->SetPalette(1);
    dpsdSty->SetOptStat(0);
    dpsdSty->SetLineWidth(2);
    dpsdSty->SetPadTopMargin(0.12);
    
    dpsdSty->SetTitleW(0.8);
    dpsdSty->SetTitleH(0.05);
    dpsdSty->SetTitleSize(0.05);
    dpsdSty->SetTitleSize(0.05, "x");
    dpsdSty->SetTitleSize(0.05, "y");
    dpsdSty->SetTitleOffset(0.96, "x");
    dpsdSty->SetTitleOffset(1.05, "y");
    
    dpsdSty->SetLabelSize(.04, "X");
    dpsdSty->SetLabelSize(.04, "Y");
    dpsdSty->SetLabelOffset(0.01);
    dpsdSty->SetLabelOffset(0.01, "X");
    dpsdSty->SetLabelOffset(0.01, "Y");

    return dpsdSty;
}

void setDPSDstyle() {
    DPSDstyle();
    gROOT->SetStyle("DPSD");
    gROOT->ForceStyle();
    gErrorIgnoreLevel = kError;
}


dpsd_gui::dpsd_gui( const TGWindow *p, UInt_t w, UInt_t h, UInt_t options) : TGMainFrame(p, w, h, options) {

    int i, j, jstr;

// GUI objects
    TString clbl;
    TString cval;
    ULong_t ucolor;
    TGFont *ufont;

// GUI graphics
    int const linspace = 25, txt_wid = 90, fld_wid = 65, cb_wid = 15;
    int const margin_hor = 5, margin_ver = 5;
    int const fld_hgt = 22, ndline = 35, lbl_wid=85,txt_fld=190;
    int spacing = 8;
    Int_t main_w = 3*fld_wid+3*txt_wid+7*margin_hor;
    Int_t status_h = 25;
    Int_t x2 = 2*margin_hor+txt_wid+fld_wid;
    Int_t nline, nline2;

// Status, menu, tool bars

    TGMenuBar   *fMenuBar;
    TGPopupMenu *fMenuFile;
    TGPopupMenu *fMenuPlot;
    TGPopupMenu *fMenuHelp;
    TGToolBar   *fToolBar;
    TGGC *uGC;
    int remap[6] = {2, 3, 4, 5, 1, 0};
    Int_t menu_h = 28;
    Int_t bars_h = 2*menu_h;
    TGLabel* lbl_txt;

    leg_lbl["Neut"]   = "Neutrons";
    leg_lbl["Neut2"]  = "Neut. (w. pileup)";
    leg_lbl["Gamma"]  = "#gamma";
    leg_lbl["Gamma2"] = "#gamma (w. pileup)";
    leg_lbl["LED"]    = "LED";
    leg_lbl["PileUp"] = "Pile-up";
    leg_lbl["Uthres"] = "Under threshold";
    leg_lbl["LEDpup"] = "LED pile-up";

    dpsdcol["Neut"]   = kMagenta;
    dpsdcol["Neut2"]  = TColor::GetColor("#a0a0a0");
    dpsdcol["Gamma"]  = kGreen;
    dpsdcol["Gamma2"] = TColor::GetColor("#00a0a0");
    dpsdcol["LED"]    = kOrange;
    dpsdcol["PileUp"] = kBlue;
    dpsdcol["Uthres"] = kCyan;
    dpsdcol["LEDpup"] = kBlack;

// GUI entries: xml name, type, TGLabel text

    const char* gui_entries[] = {
        "Slice",         "bool",   "Plot slice",
        "LEDcorrection", "bool",   "LED correction",
        "SubtBaseline",  "bool",   "Subtract baseline",
        "Acq1",          "int",    "Acq start #",
        "Acq2",          "int",    "Acq end #",
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
        setup_type[clbl] = gui_entries[jstr];
        jstr++;
        setup_label[clbl] = gui_entries[jstr];
    }

// GUI style

    ufont = gClient->GetFont("-*-helvetica-medium-r-*-*-12-*-*-*-*-*-iso8859-1");

    GCValues_t gval;
    gval.fMask = kGCForeground | kGCBackground | kGCFillStyle | kGCFont | kGCGraphicsExposures;
    gval.fFillStyle = kFillSolid;
    gval.fGraphicsExposures = kFALSE;
    gClient->GetColorByName("#000000",gval.fForeground);
    gClient->GetColorByName("#c0c0c0",gval.fBackground);
    gval.fFont = ufont->GetFontHandle();

//==========================================
// Menu-, status- and tool- and progress-bar
//==========================================


    static ToolBarData_t gToolBarData[] = {
        { "bld_exit.png"  , "Exit"            , kFALSE, M_FILE_EXIT, 0 },
        { "ed_execute.png", "Run"             , kFALSE, M_FILE_EXE , 0 },
        { "bld_open.png"  , "Load setup..."   , kFALSE, M_FILE_LOAD, 0 },
        { "bld_save.png"  , "Save setup as...", kFALSE, M_FILE_SAVE, 0 },
        { gif_rt, "Store RootFile", kFALSE, M_FILE_WROOT, 0 },
        { gif_sf, "Store shotfile", kFALSE, M_FILE_WSF  , 0 },
        { 0               , 0                 , kFALSE, 0          , 0 }
    };
    SetCleanup(kDeepCleanup);
    fToolBar  = new TGToolBar(this, 0, 0, kHorizontalFrame|kRaisedFrame);
    fStatusBar= new TGStatusBar(this, main_w, status_h, kHorizontalFrame);
    fMenuBar  = new TGMenuBar(this, 0, 0, kHorizontalFrame|kRaisedFrame);
    fMenuFile = new TGPopupMenu(gClient->GetRoot());
    fMenuPlot = new TGPopupMenu(gClient->GetRoot());
    fMenuHelp = new TGPopupMenu(gClient->GetRoot());

    fStatusBar->SetText("Ready");

    for (i = 0; gToolBarData[i].fPixmap; i++){
        j = remap[i];
        if (j == 0){
            fMenuFile->AddSeparator();
        }
        fMenuFile->AddEntry( gToolBarData[j].fTipText, gToolBarData[j].fId, 0,
                             fClient->GetPicture(gToolBarData[j].fPixmap) );

        TGPictureButton *pb = new TGPictureButton(fToolBar, 
            fClient->GetPicture(gToolBarData[i].fPixmap), gToolBarData[i].fId);
        pb->SetToolTipText(gToolBarData[i].fTipText);
        fToolBar->AddButton(this, pb, spacing);
        spacing = 0;
    }

    fMenuPlot->AddEntry(" &Plot window distr", P_WIN,   0);
    fMenuPlot->AddEntry(" &Plot time signal" , P_TIME,  0);
    fMenuPlot->AddEntry(" &Plot pulses"      , P_PULSE, 0);
    fMenuPlot->AddEntry(" &Plot PM gain"     , P_GAIN,  0);

    fMenuHelp->AddEntry(" &About...", M_HELP_ABOUT, 0, gClient->GetPicture("about.xpm"));

    fMenuBar->AddPopup(" &File", fMenuFile, new TGLayoutHints(kLHintsTop|kLHintsLeft));
    fMenuBar->AddPopup(" &Plot", fMenuPlot, new TGLayoutHints(kLHintsTop|kLHintsLeft));
    fMenuBar->AddPopup(" &Help", fMenuHelp, new TGLayoutHints(kLHintsTop|kLHintsRight));

    fMenuBar ->MoveResize(0, 0     , main_w, menu_h);
    fToolBar ->MoveResize(0, menu_h, main_w, menu_h);

    uGC = gClient->GetGC(&gval, kTRUE);

//=============
// Progress bar
//=============

    TGHorizontalFrame* prog_box = new TGHorizontalFrame(this, 248, 112, kHorizontalFrame);
    prog_lbl = new TGLabel(prog_box, " % of analysed pulses", TGLabel::GetDefaultGC()(), TGLabel::GetDefaultFontStruct(), kChildFrame);
        fProgrBar = new TGHProgressBar(prog_box,TGProgressBar::kFancy, main_w);
        fProgrBar->SetBarColor("#00cc66");
        fProgrBar->SetPosition(0.0);
        fProgrBar->ShowPosition(kTRUE,kTRUE, "");
        prog_lbl ->MoveResize(5*margin_hor, 0     , main_w-10*margin_hor, menu_h);
        fProgrBar->MoveResize(5*margin_hor, menu_h, main_w-10*margin_hor, 20);

        Int_t prog_w = main_w;
        Int_t prog_h = 2*menu_h;
        Int_t prog_y = bars_h;

//=============
// Data I/O box
//=============

    gClient->GetColorByName("#cccc33", ucolor);
    TGVerticalFrame* data_box = new TGVerticalFrame(this, 248, 112, kVerticalFrame,ucolor);
        gClient->GetColorByName("#ffff99", ucolor);
        TGLabel *data_lbl = new TGLabel(data_box, "Data I/O files", TGLabel::GetDefaultGC()(), TGLabel::GetDefaultFontStruct(), kChildFrame, ucolor);
        data_lbl->MoveResize(58, margin_hor, 120, 25);

        nline = ndline;
	clbl="Path";
        lbl_txt = new TGLabel(data_box, setup_label[clbl], TGLabel::GetDefaultGC()(), TGLabel::GetDefaultFontStruct(), kChildFrame, ucolor);
        d_txt[clbl] = new TGTextEntry(data_box, new TGTextBuffer(31), -1, uGC->GetGC(), ufont->GetFontStruct(), kSunkenFrame | kDoubleBorder | kOwnBackground);
        d_txt[clbl]->SetName(clbl);
        d_txt[clbl]->Connect("TextChanged(char*)", "dpsd_gui", this, "GetVar()");
        lbl_txt    ->MoveResize(margin_hor        , nline, lbl_wid, fld_hgt);
        d_txt[clbl]->MoveResize(margin_hor+lbl_wid, nline, txt_fld, fld_hgt);
        nline += linspace;

        const char* io_int[] = {"Acq1", "Acq2", 0};
        for (jstr = 0; io_int[jstr]; jstr++){
            clbl = io_int[jstr];
            lbl_txt = new TGLabel(data_box, setup_label[clbl], TGLabel::GetDefaultGC()(), TGLabel::GetDefaultFontStruct(), kChildFrame, ucolor);
            d_num[clbl] = new TGNumberEntry(data_box, 0, 11, -1,(TGNumberFormat::EStyle) 5);
            d_num[clbl]->GetNumberEntry()->SetName(clbl);
            d_num[clbl]->GetNumberEntry()->Connect("TextChanged(char*)", "dpsd_gui", this, "GetVar()");
            lbl_txt    ->MoveResize(margin_hor        , nline, txt_wid, fld_hgt);
            d_num[clbl]->MoveResize(margin_hor+txt_wid, nline, fld_wid, fld_hgt);
            nline += linspace;
        }

        Int_t data_w = lbl_wid+txt_fld+2*margin_hor;
        Int_t data_h = nline;
        Int_t data_y = prog_y+prog_h;

//===================
// General setup box
//===================

    gClient->GetColorByName("#66aa66", ucolor);
    TGHorizontalFrame *setup_box = new TGHorizontalFrame(this, 180, 180, kHorizontalFrame, ucolor);
        gClient->GetColorByName("#99ff99", ucolor);
        TGLabel *setup_lbl = new TGLabel(setup_box, "General Setup", TGLabel::GetDefaultGC()(), TGLabel::GetDefaultFontStruct(), kChildFrame, ucolor);
        setup_lbl->MoveResize(30, margin_hor, 88, 24);

        nline = ndline;
        const char* set1[] = {"TimeBin", "TBeg", "TEnd", "WindowLength",
                              "ToFWindowLength", "PHSxmax", 0};
        for (jstr=0; set1[jstr]; jstr++){
            clbl = set1[jstr];
            lbl_txt = new TGLabel(setup_box, setup_label[clbl], TGLabel::GetDefaultGC()(), TGLabel::GetDefaultFontStruct(), kChildFrame, ucolor);
            d_num[clbl] = new TGNumberEntry(setup_box, 0, 11, -1, (TGNumberFormat::EStyle) 5);
            d_num[clbl]->GetNumberEntry()->SetName(clbl);
            d_num[clbl]->GetNumberEntry()->Connect("TextChanged(char*)", "dpsd_gui", this, "GetVar()");
            lbl_txt    ->MoveResize(margin_hor        , nline, txt_wid, fld_hgt);
            d_num[clbl]->MoveResize(margin_hor+txt_wid, nline, fld_wid, fld_hgt);
            nline += linspace;
        }

        const char* set_boo[] = {"Slice", 0};
        for (jstr=0; set_boo[jstr]; jstr++){
            clbl = set_boo[jstr];
            lbl_txt = new TGLabel(setup_box, setup_label[clbl], TGLabel::GetDefaultGC()(), TGLabel::GetDefaultFontStruct(), kChildFrame, ucolor);
            d_ckb[clbl] = new TGCheckButton(setup_box, "");
            d_ckb[clbl]->SetBackgroundColor(ucolor);
            d_ckb[clbl]->SetName(clbl);
            d_ckb[clbl]->Connect("Toggled(Bool_t)", "dpsd_gui", this, "GetVar()");
            d_ckb[clbl]->MoveResize(margin_hor       , nline, cb_wid, fld_hgt);
            lbl_txt    ->MoveResize(margin_hor+cb_wid, nline, txt_wid+fld_wid-cb_wid-margin_hor, fld_hgt);
            nline += linspace;
        }

        Int_t setup_w = txt_wid+fld_wid+margin_hor;
        Int_t setup_h = nline;
        Int_t setup_y = margin_ver+data_y+data_h;

//================
// Separation box
//================

    gClient->GetColorByName("#99ffff", ucolor);
    TGHorizontalFrame *sep_box = new TGHorizontalFrame(this, 254, 232, kHorizontalFrame, ucolor);
        gClient->GetColorByName("#aaaaff", ucolor);
        TGLabel *sep_lbl = new TGLabel(sep_box, "Separation", TGLabel::GetDefaultGC()(), TGLabel::GetDefaultFontStruct(), kChildFrame, ucolor);
    sep_lbl->MoveResize(19, 5, 96, 24);

        nline = ndline;
        const char* sep_int[] = {"xChannels", "yChannels", "Marker", "DDlo", "DDup", "DTlo", "DTup", 0};
        for (jstr=0; sep_int[jstr]; jstr++){
            clbl = sep_int[jstr];
            lbl_txt = new TGLabel(sep_box, setup_label[clbl], TGLabel::GetDefaultGC()(), TGLabel::GetDefaultFontStruct(), kChildFrame, ucolor);
            d_num[clbl] = new TGNumberEntry(sep_box, 0, 11, -1,(TGNumberFormat::EStyle) 5);
            d_num[clbl]->GetNumberEntry()->SetName(clbl);
            d_num[clbl]->GetNumberEntry()->Connect("TextChanged(char*)", "dpsd_gui", this, "GetVar()");
            lbl_txt    ->MoveResize(margin_hor        , nline, txt_wid, fld_hgt);
            d_num[clbl]->MoveResize(margin_hor+txt_wid, nline, fld_wid, fld_hgt);
            nline += linspace;
        }

        sep_lbl = new TGLabel(sep_box, "2 sep lines", TGLabel::GetDefaultGC()(), TGLabel::GetDefaultFontStruct(), kChildFrame, ucolor);
        sep_lbl->MoveResize(165, 5, 100, 24);
        nline2 = ndline;

        const char* sep2[] = {"LineChange", "Offset", "Slope1", "Slope2", 0};
        for (jstr=0; sep2[jstr]; jstr++){
            clbl = sep2[jstr];
            lbl_txt = new TGLabel(sep_box, setup_label[clbl], TGLabel::GetDefaultGC()(), TGLabel::GetDefaultFontStruct(), kChildFrame, ucolor);
            d_num[clbl] = new TGNumberEntry(sep_box, 0, 11, -1, (TGNumberFormat::EStyle) 5);
            d_num[clbl]->GetNumberEntry()->SetName(clbl);
            d_num[clbl]->GetNumberEntry()->Connect("TextChanged(char*)", "dpsd_gui", this, "GetVar()");
            lbl_txt    ->MoveResize(x2        , nline2, txt_wid, fld_hgt);
            d_num[clbl]->MoveResize(x2+txt_wid, nline2, fld_wid, fld_hgt);
            nline2 += linspace;
        }

        gClient->GetColorByName("#99ffff", ucolor);
        TGButtonGroup *btn_group = new TGButtonGroup(sep_box, "");
        TGButton *BngSep  = new TGTextButton(btn_group, "n-gamma Sep", B_NG);
        TGButton *BRenSep = new TGTextButton(btn_group, "Separation replot", B_PHA);

        sep_box->AddFrame(btn_group);
        btn_group->SetBackgroundColor(ucolor);
        btn_group->MoveResize(x2, nline2, txt_wid+fld_wid, 2*linspace);
        BngSep   ->MoveResize(0 , 0       , txt_wid, fld_hgt);
        BRenSep  ->MoveResize(0 , linspace, txt_wid, fld_hgt);
        nline2 += 2*linspace;
        
        Int_t sep_w = 2*txt_wid+2*fld_wid+3*margin_hor;
        Int_t sep_h = TMath::Max(nline, nline2);
        Int_t sep_y = setup_y;

//=========
// LED box
//=========

    gClient->GetColorByName("#cc3333", ucolor);
    TGHorizontalFrame *led_box = new TGHorizontalFrame(this, 265, 135, kHorizontalFrame, ucolor);
        gClient->GetColorByName("#ff6666", ucolor);
        TGLabel *led_lbl = new TGLabel(led_box, "LED", TGLabel::GetDefaultGC()(), TGLabel::GetDefaultFontStruct(), kChildFrame, ucolor);
        led_lbl->MoveResize(25, 5, 96, 24);

        nline=ndline;
        const char* led_bool[] = {"LEDcorrection", 0};
        for (jstr = 0; led_bool[jstr]; jstr++){
            clbl = led_bool[jstr];
            lbl_txt = new TGLabel(led_box, setup_label[clbl], TGLabel::GetDefaultGC()(), TGLabel::GetDefaultFontStruct(), kChildFrame, ucolor);
            d_ckb[clbl] = new TGCheckButton(led_box, "");
            d_ckb[clbl]->SetBackgroundColor(ucolor);
            d_ckb[clbl]->SetName(clbl);
            d_ckb[clbl]->Connect("Toggled(Bool_t)", "dpsd_gui", this, "GetVar()");
            d_ckb[clbl]->MoveResize(margin_hor, nline, cb_wid, fld_hgt);
            lbl_txt    ->MoveResize(margin_hor+cb_wid, nline, txt_wid+fld_wid-cb_wid-margin_hor, fld_hgt);
            nline += linspace;
        }

        const char* led1[] = {"LEDdt", "LEDFront", "LEDTail", "LEDreference", "LEDxmin", "LEDxmax", "LEDymin", "LEDymax", 0};
        for (jstr = 0; led1[jstr]; jstr++){
            clbl = led1[jstr];
            lbl_txt = new TGLabel(led_box, setup_label[clbl], TGLabel::GetDefaultGC()(), TGLabel::GetDefaultFontStruct(), kChildFrame, ucolor);
            d_num[clbl] = new TGNumberEntry(led_box, 0, 11, -1, (TGNumberFormat::EStyle) 5);
            d_num[clbl]->GetNumberEntry()->SetName(clbl);
            d_num[clbl]->GetNumberEntry()->Connect("TextChanged(char*)", "dpsd_gui", this, "GetVar()");
            lbl_txt    ->MoveResize(margin_hor        , nline, txt_wid, fld_hgt);
            d_num[clbl]->MoveResize(margin_hor+txt_wid, nline, fld_wid, fld_hgt);
            nline += linspace;
        }

        Int_t led_w = txt_wid+fld_wid+margin_hor;;
        Int_t led_h = nline;
        Int_t led_y = setup_y+setup_h+margin_ver;

//==========
// Peak box
//==========

    gClient->GetColorByName("#99ff99", ucolor);
    TGHorizontalFrame *peak_box = new TGHorizontalFrame(this, 135, 135, kHorizontalFrame, ucolor);
        gClient->GetColorByName("#66aa66", ucolor);
        TGLabel *peak_lbl = new TGLabel(peak_box, "Peak detection", TGLabel::GetDefaultGC()(), TGLabel::GetDefaultFontStruct(), kChildFrame, ucolor);
        peak_lbl->MoveResize(80, margin_hor, 96, 24);

        nline = ndline;

        const char* peak_boo[] = {"SubtBaseline", 0};
        for (jstr=0; peak_boo[jstr]; jstr++){
            clbl = peak_boo[jstr];
            lbl_txt = new TGLabel(peak_box, setup_label[clbl], TGLabel::GetDefaultGC()(), TGLabel::GetDefaultFontStruct(), kChildFrame, ucolor);
            d_ckb[clbl] = new TGCheckButton(peak_box, clbl, ucolor);
            d_ckb[clbl]->SetBackgroundColor(ucolor);
            d_ckb[clbl]->SetName(clbl);
            d_ckb[clbl]->Connect("Toggled(Bool_t)", "dpsd_gui", this, "GetVar()");
            d_ckb[clbl]->MoveResize(margin_hor, nline, cb_wid, fld_hgt);
            lbl_txt    ->MoveResize(margin_hor+cb_wid, nline, txt_wid+fld_wid-cb_wid-margin_hor, fld_hgt);
            nline += linspace;
        }

        clbl = "PeakAlgorythm";
        lbl_txt = new TGLabel(peak_box, setup_label[clbl], TGLabel::GetDefaultGC()(), TGLabel::GetDefaultFontStruct(), kChildFrame, ucolor);
        d_cmb[clbl] = new TGComboBox(peak_box, -1, kHorizontalFrame | kSunkenFrame | kDoubleBorder | kOwnBackground);
        d_cmb[clbl]->AddEntry("0",    0);
        d_cmb[clbl]->AddEntry("1",    1);
        d_cmb[clbl]->AddEntry("2",    2);
        d_cmb[clbl]->AddEntry("CFD",  3);
        d_cmb[clbl]->AddEntry("ROOT", 4);
        lbl_txt    ->MoveResize(margin_hor        , nline, txt_wid, fld_hgt);
        d_cmb[clbl]->MoveResize(margin_hor+txt_wid, nline, fld_wid, fld_hgt);
        d_cmb[clbl]->SetName(clbl);
        d_cmb[clbl]->Connect("Selected(Int_t)", "dpsd_gui", this, "GetVar()");
        peak_box->AddFrame(d_cmb[clbl]);

        nline += linspace;
        const char* peak1[] = {"BaselineStart", "BaselineEnd", "Threshold", "Front", "Tail", 0};
        for (jstr=0; peak1[jstr]; jstr++){
            clbl = peak1[jstr];
            lbl_txt = new TGLabel(peak_box, setup_label[clbl], TGLabel::GetDefaultGC()(), TGLabel::GetDefaultFontStruct(), kChildFrame, ucolor);
            d_num[clbl] = new TGNumberEntry(peak_box, 0, 11, -1, (TGNumberFormat::EStyle) 5);
            d_num[clbl]->GetNumberEntry()->SetName(clbl);
            d_num[clbl]->GetNumberEntry()->Connect("TextChanged(char*)", "dpsd_gui", this, "GetVar()");
            lbl_txt    ->MoveResize(margin_hor        , nline, txt_wid, fld_hgt);
            d_num[clbl]->MoveResize(margin_hor+txt_wid, nline, fld_wid, fld_hgt);
            nline += linspace;
        }

        nline2 = ndline+linspace;
        clbl = "TotalGate";
        lbl_txt = new TGLabel(peak_box, setup_label[clbl], TGLabel::GetDefaultGC()(), TGLabel::GetDefaultFontStruct(), kChildFrame, ucolor);
        d_cmb[clbl] = new TGComboBox(peak_box, -1, kHorizontalFrame | kSunkenFrame | kDoubleBorder | kOwnBackground);
        d_cmb[clbl]->AddEntry("Full frame"      , 0);
        d_cmb[clbl]->AddEntry("Baseline cond. 1", 1);
        d_cmb[clbl]->AddEntry("Baseline cond. 2", 2);
        d_cmb[clbl]->AddEntry("Peak value"      , 3);
        lbl_txt    ->MoveResize(x2,             nline2, 0.6*txt_wid        , fld_hgt);
        d_cmb[clbl]->MoveResize(x2+0.6*txt_wid, nline2, 0.4*txt_wid+fld_wid, fld_hgt);
        d_cmb[clbl]->SetName(clbl);
        d_cmb[clbl]->Connect("Selected(Int_t)", "dpsd_gui", this, "GetVar()");
        peak_box->AddFrame(d_cmb[clbl]);

        const char* peak2[] = {"SaturationHigh", "SaturationLow", "LongGate", "ShortGate", "MaxDifference", 0};

        nline2 += linspace;
        for (jstr=0; peak2[jstr]; jstr++){
            clbl = peak2[jstr];
            lbl_txt = new TGLabel(peak_box, setup_label[clbl], TGLabel::GetDefaultGC()(), TGLabel::GetDefaultFontStruct(), kChildFrame, ucolor);
            d_num[clbl] = new TGNumberEntry(peak_box, 0, 11, -1, (TGNumberFormat::EStyle) 5);
            d_num[clbl]->GetNumberEntry()->SetName(clbl);
            d_num[clbl]->GetNumberEntry()->Connect("TextChanged(char*)", "dpsd_gui", this, "GetVar()");
            lbl_txt    ->MoveResize(x2        , nline2, txt_wid, fld_hgt);
            d_num[clbl]->MoveResize(x2+txt_wid, nline2, fld_wid, fld_hgt);
            nline2 += linspace;
        }

        Int_t peak_w = 2*txt_wid+2*fld_wid+3*margin_hor;
        Int_t peak_h = TMath::Max(nline, nline2);
        Int_t peak_y = sep_y+sep_h+margin_ver;

        Int_t status_y = TMath::Max(peak_y+peak_h, led_y+led_h);
        status_y += margin_ver;
        fStatusBar->MoveResize(0, status_y, main_w, status_h);

//========
// Actions
//========

    btn_group->Connect("Clicked(Int_t)",   "dpsd_gui", this, "HandleMenu(Int_t)");
    fMenuFile->Connect("Activated(Int_t)", "dpsd_gui", this, "HandleMenu(Int_t)");
    fMenuPlot->Connect("Activated(Int_t)", "dpsd_gui", this, "HandleMenu(Int_t)");
    fMenuHelp->Connect("Activated(Int_t)", "dpsd_gui", this, "HandleMenu(Int_t)");
    fToolBar ->Connect("Clicked(Int_t)",   "dpsd_gui", this, "HandleMenu(Int_t)");

// Set and create main frame

    SetLayoutBroken(kTRUE);
    sep_box ->SetLayoutBroken(kTRUE);
    peak_box->SetLayoutBroken(kTRUE);

    AddFrame(fMenuBar);
    AddFrame(fToolBar);
    AddFrame(prog_box);
    AddFrame(data_box,   new TGLayoutHints(kLHintsNormal));
    AddFrame(setup_box,  new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    AddFrame(sep_box,    new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
    AddFrame(led_box,    new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
    AddFrame(peak_box,   new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
    AddFrame(fStatusBar, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

    prog_box ->MoveResize(margin_hor, prog_y , prog_w , prog_h);
    data_box ->MoveResize(margin_hor, data_y , data_w , data_h);
    setup_box->MoveResize(margin_hor, setup_y, setup_w, setup_h);
    sep_box  ->MoveResize(setup_w+2*margin_hor, sep_y , sep_w , sep_h);
    led_box  ->MoveResize(margin_hor          , led_y , led_w , led_h);
    peak_box ->MoveResize(led_w+2*margin_hor  , peak_y, peak_w, peak_h);

    //    AddInput(kKeyPressMask | kKeyReleaseMask);
    Int_t main_h = status_y+status_h;
    MapSubwindows();
    SetWindowName("DPSD");
    MapWindow();
    MoveResize(600, 200, main_w, main_h);

    TString xmlfile = xml_dir + "/default.xml";
    ifstream ifile(xmlfile);
    if (ifile){
        dpsd_gui::load_xml(xmlfile);
    }
    else{
        cout << " File " << xmlfile << " not found" << endl;
    }
}

void dpsd_gui::SliceX(){

    TObject *select = gPad->GetSelected();
    if(!select) return;
    if (!select->InheritsFrom(TH2::Class())){
        gPad->SetUniqueID(0); return;
    }
    TH2 *h = (TH2*)select;
    gPad->GetCanvas()->FeedbackMode(kTRUE);

//erase old position and draw a line at current position
    int pxold   = gPad->GetUniqueID();
    int px      = gPad->GetEventX();
    float uymin = gPad->GetUymin();
    float uymax = gPad->GetUymax();
    int pymin   = gPad->YtoAbsPixel(uymin);
    int pymax   = gPad->YtoAbsPixel(uymax);
    if(pxold) gVirtualX->DrawLine(pxold, pymin, pxold, pymax);
    gVirtualX->DrawLine(px, pymin, px, pymax);
    gPad->SetUniqueID(px);
    Float_t upx = gPad->AbsPixeltoX(px);
    Float_t x   = gPad->PadtoX(upx);

//create or set the new canvas c2
    TVirtualPad *padsav = gPad;
    TCanvas *c2 = (TCanvas*)gROOT->FindObject("cproj");

    if(c2){
        delete c2->GetPrimitive("Projection");
    }
    else{
        c2 = new TCanvas("cproj", "Projection Canvas", 710, 10, 700, 500);
    }
    c2->SetGrid();
    c2->cd();

//draw slice corresponding to mouse position
    Int_t binx = h->GetXaxis()->FindBin(x);
    TH1D *hp = h->ProjectionY("proj", binx, binx);
    hp->SetLineColor(kGreen);
    hp->SetLineWidth(3);
    char title[80];
    sprintf(title, "Projection of binx=%d", binx);
    hp->SetTitle(title);
    TAxis *xhp = hp->GetXaxis();
    TAxis *yhp = hp->GetYaxis();
    hp->Draw();

// Double gaussian fit
    Double_t par[6];
    TF1 *g1    = new TF1("g1", "gaus", 400, 595);
    TF1 *g2    = new TF1("g2", "gaus", 600, 1000);
    TF1 *total = new TF1("total", "gaus(0)+gaus(3)", 0, 1023);
    total->SetLineColor(2);
    hp->Fit(g1, "R");
    hp->Fit(g2, "R+");
    g1->GetParameters(&par[0]);
    g2->GetParameters(&par[3]);
    total->SetParameters(par);
    hp->Fit(total, "R+");
    xhp->SetTitle("Short/Long");
    xhp->CenterTitle(kTRUE);
    xhp->SetTitleOffset(0.96);
    yhp->SetTitle("Counts");
    yhp->CenterTitle(kTRUE);
    c2->Update();
    padsav->cd();
}

void dpsd_gui::GetVar(){

// Updating the maps at every change in the GUI

    TGNumberEntry *entry = (TGNumberEntry*)gTQSender;
    TString clbl  = entry->GetName();
    TString stype = setup_type[clbl];
    stringstream ss;

    if (stype == "int"){
        d_int[clbl] = d_num[clbl]->GetNumber();
        ss << d_int[clbl];
        map_xml[clbl] = ss.str();
    }
    if (stype == "double"){
        d_dbl[clbl] = d_num[clbl]->GetNumber();
        ss << d_dbl[clbl];
        map_xml[clbl] = ss.str();
    }
    if (stype == "combo"){
        d_int[clbl] = d_cmb[clbl]->GetSelected();
        ss << d_int[clbl];
        map_xml[clbl] = ss.str();
    }
    if (stype == "bool"){
        d_bool[clbl] = d_ckb[clbl]->IsDown();
        if (d_bool[clbl]){
            map_xml[clbl] = "true";
        }
        else{
            map_xml[clbl] = "false";
        }
    }
    if (stype == "string"){
        d_str[clbl] = d_txt[clbl]->GetText();
        map_xml[clbl] = d_str[clbl];
    }
}

int main(int argc, char *argv[]){

    TApplication *theApp = new TApplication("dpsd_gui", &argc, argv);
    setDPSDstyle();
    main_frame = new dpsd_gui(gClient->GetRoot(), 10, 10, kMainFrame | kVerticalFrame);
    theApp->Run();
}

void plot_dpsd(){

    int i, j_ch, jt, jstr, j_pul;
    TString clbl;
    TGaxis::SetMaxDigits(3);

    tslbl.Form("#%i [%6.3f,%6.3f] s", nacq, d_dbl["TBeg"], d_dbl["TEnd"]);

    TString phs_title;
    phs_title.Form("Pulse height spectrum %s", tslbl.Data());

//-----------------------------
    PUTLOG(1, "PHS plot");
//-----------------------------

    TCanvas *cphs = (TCanvas*) gROOT->FindObject("cphs");
    if (cphs){
        delete cphs;
    }
    cphs = new TCanvas("cphs", "PHS", 1300, 0, 700, 500);

    TMultiGraph *mgphs = new TMultiGraph();
    TGraph *gphs;
    TLegend *legphs = new TLegend(0.80, 0.8, 0.95, 0.95);

    Int_t phs_chan[nxCh];
    for (j_ch=0; j_ch< nxCh; j_ch++){
        phs_chan[j_ch] = j_ch;
    }

    mgphs->SetTitle(phs_title);
    legphs->SetHeader("PHS");
    for(jstr=0; phs_label[jstr]; jstr++){
        clbl = phs_label[jstr];
        gphs = new TGraph(d_int["PHSxmax"], phs_chan, phs[clbl]);
        gphs->SetLineColor(dpsdcol[clbl]);
        mgphs->Add(gphs);
        legphs->AddEntry(gphs, leg_lbl[clbl], "l");
    }
    mgphs->Draw("AL");
    TAxis* xphs = mgphs->GetXaxis();
    TAxis* yphs = mgphs->GetYaxis();
    xphs->SetRangeUser(0, d_int["PHSxmax"]);
    xphs->SetTitle("Total Integral");
    xphs->CenterTitle(kTRUE);
    yphs->SetTitle("Counts");
    yphs->CenterTitle(kTRUE);

    legphs->Draw();

    cphs->Update();
    cphs->Modified();
    
//-----------------------------
    PUTLOG(0, "Count rates plot");
//-----------------------------

    TString cnt_title;
    TMultiGraph *mgcnt = new TMultiGraph();
    const char* cnt_label[] = {"PileUp", "LED", "Gamma", "Gamma2", "Neut", "Neut2", 0};
    TGraph *gcnt;
    TLegend *legcnt = new TLegend(0.80, 0.75, 0.95, 0.95);

    TCanvas *ccnt = (TCanvas*) gROOT->FindObject("ccnt");
    if (ccnt){
        delete ccnt;
    }
    ccnt = new TCanvas("ccnt", "Count rate", 1300, 0, 700, 500);

    cnt_title.Form("Count rates  #%i", nacq);
    mgcnt->SetTitle(cnt_title);
    legcnt->SetHeader("Count rates");

    for(jstr=0; cnt_label[jstr]; jstr++){
        clbl = cnt_label[jstr];
        gcnt = new TGraph(n_TimeBins,cnt["Time"], cnt[clbl]);
        gcnt->SetLineColor(dpsdcol[clbl]);
        mgcnt->Add(gcnt);
        legcnt->AddEntry(gcnt, leg_lbl[clbl], "l");
    }

    mgcnt->Draw("AL");
    legcnt->Draw();

    TAxis* xcnt = mgcnt->GetXaxis();
    TAxis* ycnt = mgcnt->GetYaxis();
    xcnt->SetRangeUser(cnt["Time"][0], cnt["Time"][n_TimeBins-1]);
    xcnt->SetTitle("Time [s]");
    xcnt->CenterTitle(kTRUE);
    ycnt->SetTitle("Count rate [1/s]");
    ycnt->CenterTitle(kTRUE);

    ccnt->Update();
    ccnt->Modified();
}

void plot_PMgain(){

    //-----------------------
    PUTLOG(0, "PM gain plot");
    //-----------------------

    TCanvas *cled = new TCanvas("cled", "PM gain monitor", 200, 600, 700, 500);

    TString led_title;
    led_title.Form("PM gain  %s",tslbl.Data());

    TGraph *led1 = new TGraph(n_TimeLED, cnt["TimeLED"], cnt["PMgain"]);
    led1->SetLineColor(kOrange);
    led1->Draw("AL");
    led1->SetTitle(led_title);
    TAxis* xled = led1->GetXaxis();
    TAxis* yled = led1->GetYaxis();
    xled->SetRangeUser(cnt["TimeLED"][0], cnt["TimeLED"][n_TimeLED-1]);
    xled->SetTitle("Time [s]");
    xled->CenterTitle(kTRUE);
    yled->SetTitle("LED Total Integral/LED reference");
    yled->CenterTitle(kTRUE);
    yled->SetRangeUser(0., 2.);
    cled->Update();
}

void plot_win_dist(){

    int i,hist_len[xWinLen];
    for (i=0;i<xWinLen;i++){
        hist_len[i] = 0;
    }
    //-----------------------------------------
    PUTLOG(0, "Time windows distribution plot");
    //-----------------------------------------

    TCanvas *clen = new TCanvas("clen", "Window length", 0, 600, 700, 500);

    TString wlen_title;
    wlen_title.Form("Window length distribution    %s",tslbl.Data());

    for (i=0; i<n_pulses; i++){
        if (pul_len[i] < xWinLen){
            hist_len[pul_len[i]]++;
        }
    }
    TH1D *hlen = new TH1D("hlen",wlen_title, xWinLen, 0, xWinLen);
    for (i=0; i<xWinLen; i++){
        hlen->SetBinContent(i, hist_len[i]);
    }
    hlen->SetFillStyle(4000);
    hlen->SetLineColor(kRed);
    hlen->Draw("");
    TAxis* xlen = hlen->GetXaxis();
    TAxis* ylen = hlen->GetYaxis();
    xlen->SetTitle("Length [sample]");
    xlen->CenterTitle(kTRUE);
    ylen->SetTitle("Window count");
    ylen->CenterTitle(kTRUE);

    clen->Update();
    clen->Modified();

    TObject *h_len = gDirectory->GetList()->FindObject("hlen");
    gDirectory->GetList()->Remove(h_len);
}

void plot_time(){

//-----------------------------
    PUTLOG(0, "Signal vs time");
//-----------------------------

    int i, jt, jtim, j_pul, jstart;
    double *tim_bin;
    double *tim_dat;
    double tim_base[totallength];

    jtim=0;
    for (j_pul=0; j_pul<n_pulses; j_pul++){
        for (jt=0; jt<pul_len[j_pul]; jt++){
            tim_base[jtim] = t_dpsd[j_pul]+jt*5.e-9;
            jtim++;
        }
    }

    TCanvas *ctim = new TCanvas("ctim", "Time trace", 500, 300, 700, 500);

    jstart = jtbeg;
    for (jt=0; jt<n_TimeBins; jt++){
        double t1 = t_dpsd[0]+jt*TimeBin;
        double t2 = t1+TimeBin;
        int plot_len = 0;
        for (i=jstart; tim_base[i]<t2; i++){
            plot_len++;
        }
        tim_bin = new double [plot_len];
        tim_dat = new double [plot_len];
        for (i=jstart; tim_base[i]<t2; i++){
            tim_bin[i-jstart] = tim_base[i];
            tim_dat[i-jstart] = DDATA[i];
        }
        TGraph *tim1 = new TGraph(plot_len, tim_bin, tim_dat);
        tim1->SetLineColor(kGreen);
        tim1->Draw("AL");
        TAxis* xtim = tim1->GetXaxis();
        TAxis* ytim = tim1->GetYaxis();
        xtim->SetRangeUser(t1, t2);
        ytim->SetRangeUser(-50, 2000);
        xtim->SetTitle("Time [s]");
        xtim->CenterTitle(kTRUE);
        ytim->SetTitle("Pulse");
        ytim->CenterTitle(kTRUE);
        ctim->Update();
        jstart = i+1;
    }
}


void dpsd_gui::pulse_display(Int_t mypul){

    Int_t j_pul, jt;

    TString t_str;
    TText *timt;
    const char *lbl; 
    TH1D *pul = new TH1D("Pulse_hist", "Pulse", xWinLen, 0, xWinLen-1);

    TCanvas *cpul = fEC->GetCanvas();

    pul->SetLineWidth(2);

    pulse_len = pul_len[mypul];
    t_str.Form("t = %9.4f s", t_dpsd[mypul]);
    timt = new TText(0.7*xWinLen, 0., t_str);

    Int_t isumm=0;
    timt->SetTextColor(kBlack);

    for (j_pul=0; j_pul<mypul; j_pul++){
        isumm += pul_len[j_pul];
    }

    if (flg_led[mypul]==0 && flg_npeaks[mypul]==1){
        if (flg_neut[mypul] == 1){
            cpul->cd(1);
            lbl = "Neut";
        }
        if (flg_neut[mypul] == 0){
            cpul->cd(2);
            lbl = "Gamma";
        }
    }
    if (flg_led[mypul] > 0){
        if (flg_ledpks[mypul] <= 1){
            cpul->cd(3);
            lbl = "LED";
        }
        else{
            cpul->cd(5);
            lbl = "LEDpup";
        }
    } 
    if (flg_npeaks[mypul] > 1){
        cpul->cd(4);
        lbl = "PileUp";
    }
    if (flg_npeaks[mypul] == 0){
        cpul->cd(6);
        lbl = "Uthres";
    } 

    for (jt=0; jt<pulse_len; jt++){
        pul->SetBinContent(jt, DDATA[isumm+jt]);
    }
    pul->SetTitle(leg_lbl[lbl]);
    pul->SetLineColor(dpsdcol[lbl]);
    pul->Draw("AL");
    timt->Draw("same");
    cpul->cd()->Update();
    gSystem->ProcessEvents();

}


void dpsd_gui::pulses_pause(){

    pstop=true;
    cout << "Pulse " << npul << endl;
    play_fig->SetPicture(gClient->GetPicture(graphdir + "play.gif"));
    dpsd_gui::pulse_display(npul);
}

void dpsd_gui::pulses_slider(){
    Int_t perc = hslider->GetPosition();
    npul = perc*n_pulses/100;
    dpsd_gui::pulses_pause();
}

void dpsd_gui::pulses_next(){
    npul++;
    if(npul >= n_pulses){
        npul = n_pulses-1;
    }
    pulses_pause();
}

void dpsd_gui::pulses_prev(){
    npul -= 1;
    if(npul < 0){
        npul = 0;
    }
    pulses_pause();
}

void dpsd_gui::pulses_play(){

    Int_t pos;

    pstop = false;
    play_fig->SetPicture(gClient->GetPicture(graphdir + "pause.gif"));

    while(npul < n_pulses-1 && !pstop){
        npul++;
        dpsd_gui::pulse_display(npul);
        pos = 100*npul/n_pulses;
        hslider->SetPosition(pos);
    }

}


void dpsd_gui::plot_pulses(const TGWindow *p, const TGWindow *main){

//-------------------------------------
    PUTLOG(1, "Start pulse processing");
//-------------------------------------

    TGFont *ufont;
    TGGC   *uGC;
    ULong_t ucolor; 

    static ToolBarData_t gPulBarData[] = {
        { gif_fw  , "Next pulse", kFALSE, P_FWD , 0 },
        { gif_back, "Prev pulse", kFALSE, P_BACK, 0 },
        { 0       , 0           , kFALSE, 0     , 0 }
    };

// Frame style
    
    ufont = gClient->GetFont("-*-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*");

    GCValues_t frame_sty;
    frame_sty.fMask = kGCForeground | kGCBackground | kGCFillStyle | kGCFont | kGCGraphicsExposures;
    gClient->GetColorByName("#5954d9", frame_sty.fForeground);
    gClient->GetColorByName("#c0c0c0", frame_sty.fBackground);
    frame_sty.fFillStyle = kFillSolid;
    frame_sty.fFont = ufont->GetFontHandle();
    frame_sty.fGraphicsExposures = kFALSE;

    uGC = gClient->GetGC(&frame_sty, kTRUE);

    TGMainFrame *fPulFr = new TGMainFrame(p, 1200, 600, kVerticalFrame|kRaisedFrame);

    fEC = new TRootEmbeddedCanvas("ec1", fPulFr, 1180, 550);
    TCanvas *cpul = fEC->GetCanvas();
    cpul->Divide(3, 2);
    dpsd_gui::pulse_display(0);

    hslider = new TGHSlider(fPulFr, 250, kSlider2|kScaleDownRight, SL_ID);
    hslider->SetRange(0, 100);
    hslider->SetPosition(0);
    hslider->Connect("Released()", "dpsd_gui", fPulFr, "pulses_slider()");

    TGToolBar *PulToolBar = new TGToolBar(fPulFr, 0, 0, kHorizontalFrame|kRaisedFrame);
    Int_t spacing = 0;

    play_fig = new TGPictureButton(PulToolBar, gClient->GetPicture(graphdir + "play.gif"), P_RUN);
    play_fig->SetToolTipText("Run");
    PulToolBar->AddButton(fPulFr, play_fig, spacing);

    for (Int_t i=0; gPulBarData[i].fPixmap; i++){
        TGPictureButton *pb = new TGPictureButton(PulToolBar, 
            gClient->GetPicture(gPulBarData[i].fPixmap), gPulBarData[i].fId);
        pb->SetToolTipText(gPulBarData[i].fTipText);
        PulToolBar->AddButton(fPulFr, pb, spacing);
    }

    PulToolBar ->Connect("Clicked(Int_t)", "dpsd_gui", fPulFr, "HandleMenu(Int_t)");

    fPulFr->AddFrame(fEC);
    fPulFr->AddFrame(hslider);
    fPulFr->AddFrame(PulToolBar);

    fEC       ->MoveResize(0, 0  , 1100, 630);
    hslider   ->MoveResize(0, 630, 400 , 40);
    PulToolBar->MoveResize(0, 670, 200 , 28);

    fPulFr->MapSubwindows();
    fPulFr->SetWindowName("Pulse processing");
    fPulFr->MapWindow();
    fPulFr->MoveResize(0, 0, 1100, 700);
}


/* 
Pulse shape versus pulse height for all pulses in the time interval
*/
void dpsd_gui::pha(){

    int j_col, j_pul;
    Int_t ntmp;

    PUTLOG(0, "PS vs PH histogram");
    TString pha_title;
    pha_title.Form("n-#gamma separation  #%i [%6.3f,%6.3f] s", nacq, d_dbl["TBeg"], d_dbl["TEnd"]);
    TH2I *hpha = (TH2I*) gROOT->FindObject("PS_PH");
    if (hpha){
        hpha->SetTitle(pha_title);
        hpha->Reset();
    }
    else{
        hpha = new TH2I("PS_PH", pha_title, nxCh, 0., double(nxCh-1), nyCh, 0., double(nyCh-1));
    }

    Double_t *weight = new Double_t[n_pulses];
    for (j_pul=0; j_pul<n_pulses; j_pul++){
        if ( (flg_sat[j_pul]==0) && (flg_npeaks[j_pul]==1) ){
	    weight[j_pul] = 1.;
	}
	else{
	    weight[j_pul] = 0.;
	}
    }
    hpha->FillN(n_pulses, PulseHeight, PulseShape, weight);
    Int_t n_col=10;
    Double_t contours[n_col];
    Double_t maxsm = hpha->GetBinContent(hpha->GetMaximumBin());
    Double_t colstep = log(maxsm)/double(n_col-1);

    contours[0] = .8;
    for (j_col=1; j_col<n_col ; j_col++){
        contours[j_col] = contours[j_col-1]*exp(colstep);
    }

    TGaxis::SetMaxDigits(4);

    TCanvas *cpha = (TCanvas*) gROOT->FindObject("cpha");
    if (cpha){
        cpha->cd();
    }
    else{
        cpha = new TCanvas("cpha", "Separation diagram", 0, 0, 600, 600);
        cpha->SetRightMargin(0.15);
        cpha->SetLeftMargin(0.15);
	cpha->SetLogz();
    }

    hpha->SetMarkerStyle(2);
    hpha->SetMarkerSize(5);
    hpha->GetZaxis()->SetRangeUser(0.95, maxsm); 
    hpha->SetContour(n_col, contours);
    hpha->Draw("col logz");
    if (d_bool["Slice"]){
        gPad->AddExec("dynamic", Form("((dpsd_gui*)0x%x)->SliceX()"));
    }
    TAxis *xpha = hpha->GetXaxis();
    TAxis *ypha = hpha->GetYaxis();
    xpha->SetTitle("Total Integral");
    xpha->CenterTitle(kTRUE);
    ypha->SetTitle("Short/Long");
    ypha->CenterTitle(kTRUE);
    ypha->SetTitleOffset(1.5);

    TText *ddt = new TText(1.1*d_int["DDlo"], 200, "D-D");
    TText *dtt = new TText(1.1*d_int["DTlo"], 400, "D-T");
    TLatex *gammat = new TLatex(1.1*d_int["DTlo"], 900, "#gamma");

    ddt   ->SetTextColor(kMagenta);
    dtt   ->SetTextColor(kBlack);
    gammat->SetTextColor(kGreen);

    dtt   ->Draw("same"); 
    ddt   ->Draw("same");
    gammat->Draw("same");

    dpsd_gui::sep_replot();
    
}


void dpsd_gui::load_xml(TString sname){

    TString clbl, stype;
    TString cval;
    int val;
    double dval;
    map<TString, TString>:: iterator siter;

    map_xml = xml2map(sname);

    for (siter=map_xml.begin(); siter != map_xml.end(); ++siter){
        clbl = siter->first;
        cval = siter->second;
        stype = setup_type[clbl];
        if (stype == "bool"){
            if (cval == "true"){
                d_bool[clbl] = true;
                d_ckb[clbl]->SetState(kButtonDown);
            }
            else{
                d_bool[clbl] = false;
                d_ckb[clbl]->SetState(kButtonUp);
            }
        }
        if (stype == "int"){
            val = atoi(cval);
            d_num[clbl]->SetIntNumber(val);
        }
        if (stype == "double"){
            dval = atof(cval);
            d_num[clbl]->SetNumber(dval);
        }
        if (stype == "combo"){
            val = atoi(cval);
            d_cmb[clbl]->Select(val);
        }
        if (stype == "string"){
            d_txt[clbl]->SetText(cval);
        }
    }
}

void dpsd_gui::sep_replot(){

    int jstr, j_ch;

    Double_t* Separatrix = new Double_t[nxCh];
    PUTLOG(0, "PHA plot");

    Separatrix[0] = d_dbl["Offset"];
    for (j_ch=1; j_ch<=d_int["LineChange"]; j_ch++){
        Separatrix[j_ch] = Separatrix[j_ch-1] + d_dbl["Slope1"];
    }
    for (j_ch=d_int["LineChange"]+1; j_ch<nxCh; j_ch++){
        Separatrix[j_ch] = Separatrix[j_ch-1] + d_dbl["Slope2"];
    }

    TCanvas *canvas1 = (TCanvas*) gROOT->FindObject("cpha");
    if (canvas1){
        canvas1->cd();
    }

    TH1D *pha_sep = (TH1D*) gROOT->FindObject("phasep");
    if (pha_sep){
        delete pha_sep;
    }

    pha_sep = new TH1D("phasep", "Separatrix", nxCh, 0, nxCh-1);
    for (j_ch=0; j_ch<nxCh; j_ch++){
        pha_sep->SetBinContent(j_ch, Separatrix[j_ch]);
    }
    pha_sep->SetLineColor(kRed);
    pha_sep->Draw("same");

// LED Box
    TBox *ledbox = new TBox(d_int["LEDxmin"], d_int["LEDymin"], TMath::Min(d_int["LEDxmax"], nxCh-1), d_int["LEDymax"]);
    ledbox->SetFillStyle(0);
    ledbox->SetLineColor(kBlue);
    ledbox->Draw("same");

// DD - DT

    const char* lines[] = {"DDlo", "DDup", "DTlo", "DTup", 0};
    TString clbl;
    int nX;
    for(jstr=0; lines[jstr]; jstr++){
        clbl = lines[jstr];
        TGraph *mygr = (TGraph*) gROOT->FindObject(clbl);
        if (mygr){
            delete mygr;
        }
        nX = d_int[clbl];
        double xax[2] = {double(nX), double(nX)};
        double yax[2] = {0, Separatrix[nX]};
        mygr = new TGraph(2, xax, yax);
        mygr->SetName(clbl);
        if(clbl(0, 2) == "DD"){
            mygr->SetLineColor(kMagenta);
        }
	mygr->Draw("same");
    }

    canvas1->Update();
    canvas1->Modified();
}

void dpsd_gui::loop_dpsd() {

    int j_acq;
    int acq_start = d_int["Acq1"];
    int acq_end   = d_int["Acq2"];
    TString xmlfile = xml_dir + "/tmp.xml";
    map2xml(xmlfile, map_xml);
    Bool_t wsf=false, wrt=false;
    TString s_exp = getenv("USER");

    if(acq_start > acq_end){
        nacq = acq_start;
        dpsd_run(nacq, xmlfile, wsf, wrt, s_exp);
	dpsd_gui::pha();
	plot_dpsd();
    }
    else{
        for(j_acq=acq_start; j_acq<=acq_end; j_acq++){
            if ((j_acq != 266) && (j_acq != 303)){
                nacq = j_acq;
                dpsd_run(nacq, xmlfile, wsf, wrt, s_exp);
            }
        }
    }
}


void dpsd_gui::HandleMenu(Int_t menu_id){

// Handle menu events

    TString gHelpDPSD = "Digital Pulse Discrimination Analysis\nVersion 1.0.0\nGiovanni Tardini 2011 Dec 5th\nhttp://www.ipp.mpg.de/~git/ne213/dpsd/index.php";
    TGFileInfo fi;
    const char *dpsd_types[] = {
        "xml files", "*.xml",
        "All files", "*",
         0         ,  0};

    fi.fFileTypes = dpsd_types;

    char *cxml = new char[xml_dir.Length()];
    strcpy(cxml, xml_dir);
    fi.fIniDir = cxml;

    switch (menu_id){
        case M_FILE_EXIT:
            gApplication->Terminate(0);
            break;
        case M_FILE_LOAD:
            fStatusBar->SetText("Load setup");
            new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fi);
            dpsd_gui::load_xml(fi.fFilename);
            break;
        case M_FILE_SAVE:
            fStatusBar->SetText("Save setup");
            new TGFileDialog(gClient->GetRoot(), this, kFDSave, &fi);
            map2xml(fi.fFilename, map_xml);
            break;
        case M_FILE_WSF:
            fStatusBar->SetText("Write shotfile");
            StoreSF(getenv("USER"));
            break;
        case M_FILE_WROOT:
            fStatusBar->SetText("Write RootFile");
            StoreRoot();
            break;
        case M_FILE_EXE:
            fStatusBar->SetText("Running DPSD");
            dpsd_gui::loop_dpsd();
            break;
        case M_HELP_ABOUT:
            TRootHelpDialog *hd;
            fStatusBar->SetText("DPSD help");
            hd = new TRootHelpDialog(this, "About DPSD...", 550, 250);
            hd->SetText(gHelpDPSD);
            hd->Popup();
            break;
        case B_NG:
            fStatusBar->SetText("Recomputing count rates");
            ng_sep();
            break;
        case B_PHA:
            fStatusBar->SetText("Replotting PHA with new separatrix");
            sep_replot();
            break;
        case P_GAIN:
            plot_PMgain();
            break;
        case P_TIME:
            fStatusBar->SetText("Plotting time signal");
            plot_time();
            break;
        case P_WIN:
            plot_win_dist();
            break;
        case P_PULSE:
            fStatusBar->SetText("Plotting single pulses");
            dpsd_gui::plot_pulses(gClient->GetRoot(), main_frame);
            break;
        case P_RUN:
            fStatusBar->SetText("Plotting single pulses");
            if(pstop){
                dpsd_gui::pulses_play();
            }
            else{
                dpsd_gui::pulses_pause();
            }
            break;
        case P_FWD:
            fStatusBar->SetText("Plotting single pulses");
            dpsd_gui::pulses_next();
            break;
        case P_BACK:
            fStatusBar->SetText("Plotting single pulses");
            dpsd_gui::pulses_prev();
            break;

    }
    fStatusBar->SetText("Ready");
}
