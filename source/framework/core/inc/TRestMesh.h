///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestMesh.h
///
///             Event class to help for using mesh nodes
///
///             feb 2016:
///                 Javier Galan
///_______________________________________________________________________________

#ifndef RestCore_TRestMesh
#define RestCore_TRestMesh

#include <iostream>

#include <TObject.h>
#include <TVector3.h>
#include "TRestHits.h"

const int NODE_NOT_SET = -1;
const int GROUP_NOT_FOUND = -1;
const int NODE_NOT_FOUND = -1;

class TRestMesh : public TObject {
   protected:
    TVector3 fNetOrigin;

    Double_t fNetSizeX;
    Double_t fNetSizeY;
    Double_t fNetSizeZ;

    Int_t fNodesX;
    Int_t fNodesY;
    Int_t fNodesZ;

    Int_t fNumberOfNodes;
    Int_t fNumberOfGroups;

    std::vector<Int_t> nodeGroupID;
    std::vector<Int_t> nodeX;
    std::vector<Int_t> nodeY;
    std::vector<Int_t> nodeZ;

    Bool_t fIsCylindrical = false;

   public:
    // Getters
    Int_t GetNumberOfNodes() { return fNumberOfNodes; }
    Int_t GetNumberOfGroups() { return fNumberOfGroups; }

    Double_t GetX(Int_t nX);
    Double_t GetY(Int_t nY);
    Double_t GetZ(Int_t nZ);

    Int_t GetNodeX(Double_t x, Bool_t relative = false);
    Int_t GetNodeY(Double_t y, Bool_t relative = false);
    Int_t GetNodeZ(Double_t z, Bool_t relative = false);

    Bool_t IsCylindrical() { return fIsCylindrical; }

    TVector3 GetNodeByIndex(Int_t index) {
        TVector3 node(nodeX[index], nodeY[index], nodeZ[index]);
        return node;
    }

    void SetNodesFromHits(TRestHits* hits);
    void Regrouping();

    Int_t GetNodeIndex(Int_t nx, Int_t ny, Int_t nz);

    Int_t GetGroupId(Double_t x, Double_t y, Double_t z);
    Int_t GetGroupId(Int_t index);
    //       Int_t GetGroupIdByIndex( Int_t index );

    Int_t FindNeighbourGroup(Int_t nx, Int_t ny, Int_t nz);
    Int_t FindForeignNeighbour(Int_t nx, Int_t ny, Int_t nz);

    // Setters
    void SetOrigin(Double_t oX, Double_t oY, Double_t oZ);
    void SetOrigin(TVector3 pos);

    void SetSize(Double_t sX, Double_t sY, Double_t sZ);

    void SetNodes(Int_t nX, Int_t nY, Int_t nZ);

    void SetCylindrical(Bool_t v) { fIsCylindrical = v; }

    Int_t GetNodesX() { return fNodesX; }
    Int_t GetNodesY() { return fNodesY; }
    Int_t GetNodesZ() { return fNodesZ; }

    Double_t GetNetSizeX() { return fNetSizeX; }
    Double_t GetNetSizeY() { return fNetSizeY; }
    Double_t GetNetSizeZ() { return fNetSizeZ; }

    TVector3 GetOrigin() { return fNetOrigin; }
    TVector3 GetNetSize() { return TVector3(fNetSizeX, fNetSizeY, fNetSizeZ); }
    TVector3 GetNetCenter();
    TVector3 GetVertex(Int_t id);

    /// It returns the bottom boundary vertex
    TVector3 GetBottomVertex() { return GetVertex(0); }
    /// It returns the top boundary vertex
    TVector3 GetTopVertex() { return GetVertex(1); }

    void AddNode(Double_t x, Double_t y, Double_t z);

    void RemoveNodes();

    Bool_t IsInside(TVector3 pos);
    Bool_t IsInsideBoundingBox(TVector3 pos);

    std::vector<TVector3> GetTrackBoundaries(TVector3 pos, TVector3 dir, Bool_t particle = true);
    std::vector<TVector3> GetTrackBoundariesCylinder(TVector3 pos, TVector3 dir, Bool_t particle = true);

    void Print();

    // Construtor
    TRestMesh();

    TRestMesh(Double_t size, Int_t nodes);
    TRestMesh(TVector3 size, TVector3 position, Int_t nx, Int_t ny, Int_t nz);
    // Destructor
    ~TRestMesh();

    ClassDef(TRestMesh, 2);
};
#endif
