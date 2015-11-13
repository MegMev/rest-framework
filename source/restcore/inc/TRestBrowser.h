///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///             
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestBrowser.h : public TRestRun
///
///             G4 class description
///
///             sept 2015:   First concept
///                 Created as part of the conceptualization of existing REST 
///                 software.
///                 JuanAn Garcia
///_______________________________________________________________________________


#ifndef RestCore_TRestBrowser
#define RestCore_TRestBrowser

#include "TObject.h"
#include "TGFrame.h"
#include "TGButton.h"
#include "TGNumberEntry.h"
#include "TGTextEntry.h"
#include "TGFileDialog.h"
#include "TApplication.h"

#include "TRestRun.h"
#include "TRestEventViewer.h"

#include <iostream>
using namespace std;

class TRestBrowser: public TRestRun {
    
    protected:
	
    #ifndef __CINT__
        TGMainFrame* frmMain;

        //Frames and buttons
        TGVerticalFrame *fVFrame;   	//! < Vertical frame.
        TGNumberEntry   *fNEvent;  	//! Event number.
        TGTextButton	*fLoadEvent;	//! Load Event button
        TGTextButton	*fExit;		//! Load Event button
        TGPictureButton	*fButNext;  	//! Next number.
        TGPictureButton	*fButPrev;  	//! Previous event.
        TGPictureButton	*fMenuOpen;	//! Open file
    #endif     


private:                	
    #ifndef __CINT__
	Bool_t isFile;
	
	TRestEventViewer *fEventViewer;
   #endif
	
    public:
        //Constructors
        TRestBrowser();

        //Destructor
        ~TRestBrowser();

        void Initialize();                
	void InitFromConfigFile();
	
	void SetViewer(TRestEventViewer *eV){fEventViewer=eV;}
	
        void setWindowName(TString wName ){frmMain->SetWindowName(wName.Data());}

        void setButtons( );    
	
        void LoadEventAction();
        void LoadNextEventAction();
        void LoadPrevEventAction();
        void LoadFileAction( );
        void ExitAction( );

        Bool_t OpenFile( TString fName );
	Bool_t LoadEvent( Int_t n);
	
        ClassDef(TRestBrowser, 1);     //
};
#endif
