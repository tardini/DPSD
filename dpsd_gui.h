#include "TGFrame.h"
#include "TGProgressBar.h"

extern TGHProgressBar *fProgrBar;

class dpsd_gui : public TGMainFrame
{
private:

public:
   dpsd_gui(const TGWindow *p, UInt_t w, UInt_t h, UInt_t options);
   virtual ~dpsd_gui() {}

   void load_xml(TString xml_file);
   void plot_pulses(const TGWindow *p, const TGWindow *main);
   void pulse_display(Int_t mypul);
   void pulses_slider();
   void pulses_play();
   void pulses_pause();
   void pulses_next();
   void pulses_prev();
   void HandleMenu(int);
   void SliceX();
   void GetVar();
   void loop_dpsd();
   void pha();
   void sep_replot();
   ClassDef(dpsd_gui, 0)
};
