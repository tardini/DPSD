// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME dpsd_gui_Dict

/*******************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define G__DICTIONARY
#include "RConfig.h"
#include "TClass.h"
#include "TDictAttributeMap.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"
#include "TFileMergeInfo.h"
#include <algorithm>
#include "TCollectionProxyInfo.h"
/*******************************************************************/

#include "TDataMember.h"

// Since CINT ignores the std namespace, we need to do so in this file.
namespace std {} using namespace std;

// Header files passed as explicit arguments
#include "dpsd_gui.h"
#include "dpsd_run.h"
#include "xml.h"

// Header files passed via #pragma extra_include

namespace ROOT {
   static void delete_dpsd_gui(void *p);
   static void deleteArray_dpsd_gui(void *p);
   static void destruct_dpsd_gui(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::dpsd_gui*)
   {
      ::dpsd_gui *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::dpsd_gui >(0);
      static ::ROOT::TGenericClassInfo 
         instance("dpsd_gui", ::dpsd_gui::Class_Version(), "dpsd_gui.h", 6,
                  typeid(::dpsd_gui), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::dpsd_gui::Dictionary, isa_proxy, 4,
                  sizeof(::dpsd_gui) );
      instance.SetDelete(&delete_dpsd_gui);
      instance.SetDeleteArray(&deleteArray_dpsd_gui);
      instance.SetDestructor(&destruct_dpsd_gui);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::dpsd_gui*)
   {
      return GenerateInitInstanceLocal((::dpsd_gui*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::dpsd_gui*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr dpsd_gui::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *dpsd_gui::Class_Name()
{
   return "dpsd_gui";
}

//______________________________________________________________________________
const char *dpsd_gui::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::dpsd_gui*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int dpsd_gui::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::dpsd_gui*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *dpsd_gui::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::dpsd_gui*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *dpsd_gui::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::dpsd_gui*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
void dpsd_gui::Streamer(TBuffer &R__b)
{
   // Stream an object of class dpsd_gui.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(dpsd_gui::Class(),this);
   } else {
      R__b.WriteClassBuffer(dpsd_gui::Class(),this);
   }
}

namespace ROOT {
   // Wrapper around operator delete
   static void delete_dpsd_gui(void *p) {
      delete ((::dpsd_gui*)p);
   }
   static void deleteArray_dpsd_gui(void *p) {
      delete [] ((::dpsd_gui*)p);
   }
   static void destruct_dpsd_gui(void *p) {
      typedef ::dpsd_gui current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::dpsd_gui

namespace {
  void TriggerDictionaryInitialization_dpsd_gui_Dict_Impl() {
    static const char* headers[] = {
"dpsd_gui.h",
"dpsd_run.h",
"xml.h",
0
    };
    static const char* includePaths[] = {
"/afs/ipp-garching.mpg.de/home/n/nesp/myroot/include",
"/afs/ipp-garching.mpg.de/home/n/nesp/DPSD/",
0
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "dpsd_gui_Dict dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_Autoloading_Map;
class __attribute__((annotate("$clingAutoload$dpsd_gui.h")))  dpsd_gui;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "dpsd_gui_Dict dictionary payload"

#ifndef G__VECTOR_HAS_CLASS_ITERATOR
  #define G__VECTOR_HAS_CLASS_ITERATOR 1
#endif

#define _BACKWARD_BACKWARD_WARNING_H
#include "dpsd_gui.h"
#include "dpsd_run.h"
#include "xml.h"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[]={
"dpsd_gui", payloadCode, "@",
nullptr};

    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("dpsd_gui_Dict",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_dpsd_gui_Dict_Impl, {}, classesHeaders, /*has no C++ module*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_dpsd_gui_Dict_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_dpsd_gui_Dict() {
  TriggerDictionaryInitialization_dpsd_gui_Dict_Impl();
}
