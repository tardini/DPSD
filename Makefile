CC	= g++
OFLAGS	= -O2
LDFLAGS	= $(shell $(ROOT_CERN_HOME)/bin/root-config --cflags --glibs)
DIC_GUI = dpsd_gui_Dict.cxx
DIC_BG  = dpsd_bg_Dict.cxx
EXE_GUI = DPSD_GUI
EXE_BG  = DPSD_BG
LIBWW = /afs/ipp-garching.mpg.de/aug/ads/lib64/amd64_sles11/
INCWW = /afs/ipp-garching.mpg.de/aug/ads/common/include

$(EXE_GUI): dpsd_gui.cpp dpsd_run.cpp xml.cpp $(DIC_GUI)
	$(CC) $(LDFLAGS) $(OFLAGS) -I$(INCWW) -L$(LIBWW) -lXMLIO -lddww8 -lsfh8 -lSpectrum -o $(EXE_GUI) dpsd_gui.cpp dpsd_run.cpp xml.cpp $(DIC_GUI) -DGUI

$(DIC_GUI): dpsd_gui.h dpsd_run.h xml.h LinkDef2.h
	rootcint -f $(DIC_GUI) -c dpsd_gui.h dpsd_run.h xml.h LinkDef2.h

$(EXE_BG): dpsd_bg.cpp dpsd_run.cpp xml.cpp $(DIC_BG)
	$(CC) $(LDFLAGS) $(OFLAGS) -I$(INCWW) -L$(LIBWW) -lXMLIO -lddww8 -lsfh8 -o $(EXE_BG) dpsd_bg.cpp dpsd_run.cpp xml.cpp $(DIC_BG)

$(DIC_BG): dpsd_run.h xml.h LinkDef.h
	rootcint -f $(DIC_BG) -c dpsd_run.h xml.h LinkDef.h

clean:
	$(RM) -f $(EXE_BG) $(EXE_GUI) *.o *.d *Dict.*
