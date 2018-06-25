///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///             
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestEventViewer.h
///
///             A geometry class to store detector geometry 
///
///             jul 2015:   First concept
///                 J. Galan
///		nov 2015: Generic class for event visualization
///		    JuanAn Garcia
///
///_______________________________________________________________________________


#ifndef RestCore_TRestEventViewer
#define RestCore_TRestEventViewer

#include <iostream>

#include <TObject.h>
#include <TGeoManager.h>
#include <TGLViewer.h>

#include "TRestEvent.h"

class TRestEventViewer {

    protected:
     
    TGeoManager *fGeometry;
    TRestEvent *fEvent;
        
    TString fOption;

    public:

    void SetOption( TString opt ) { fOption = opt; }

    virtual void Initialize( Int_t fW = 1024, Int_t fH = 768 );

    virtual void AddEvent( TRestEvent *ev ) = 0;
            
    virtual void DeleteCurrentEvent(  );
    //Setters
    virtual void SetGeometry( TGeoManager *geo){ fGeometry=geo;}
    void SetEvent( TRestEvent *ev){fEvent=ev;}
    //Getters
    TGeoManager *GetGeometry( ){return fGeometry;}
    TRestEvent *GetEvent( ){return fEvent;}

            
    //Construtor
    TRestEventViewer();
    //Destructor
    virtual ~ TRestEventViewer();

    ClassDef(TRestEventViewer, 1);     // REST event superclass
};
#endif

