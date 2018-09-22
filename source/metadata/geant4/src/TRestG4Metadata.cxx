/*************************************************************************
 * This file is part of the REST software framework.                     *
 *                                                                       *
 * Copyright (C) 2016 GIFNA/TREX (University of Zaragoza)                *
 * For more information see http://gifna.unizar.es/trex                  *
 *                                                                       *
 * REST is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * REST is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have a copy of the GNU General Public License along with   *
 * REST in $REST_PATH/LICENSE.                                           *
 * If not, see http://www.gnu.org/licenses/.                             *
 * For the list of contributors see $REST_PATH/CREDITS.                  *
 *************************************************************************/


//////////////////////////////////////////////////////////////////////////
/// TRestG4Metadata is the main class used to interface with *restG4* (a REST based
/// Geant4 code distributed with REST) used to launch Geant4 based simulations, and
/// store later on the simulation conditions as metadata information inside the 
/// output generated file. The simulations produced by *restG4* will write to disk 
/// the event data generated as a TRestG4Event type. The tutorials page includes a
/// tutorial describing on detail how to launch a restG4 simulation and 
/// retrieve basic simulation results.
///
/// There are few helper classes that aid to organize and access the information that 
/// TRestG4Metadata contains, TRestG4PrimaryGenerator, TRestBiasingVolume, 
/// TRestParticleCollection, TRestParticle, TRestParticleSource, and TRestPhysicsLists.
/// 
/// The full RML description to be provided to *restG4* to launch a simulation 
/// requires in addition a TRestPhysicsLists definition, providing the physics 
/// processes, EM, hadronic, etc, that will be active in our Geant4 simulation.
/// 
/// In general terms, an RML file to be used with restG4 must define the following 
/// sections, and structures.
///
/// \code
///
/// // We must, as usual, define the location where the REST output files will be stored
/// <globals>
///    <parameter name="mainDataPath" value="${REST_DATAPATH}" />
/// </globals>
///
/// // A TRestRun section to define few run parameters with a general run description.
/// <TRestRun>
///     ...
/// </TRestRun>
/// 
/// //A TRestG4Metadata section definning few parameters, generator, and storage.
/// <TRestG4Metadata>
///     ...
/// </TRestG4Metadata>
///
/// //A TRestPhysicsLists section definning the physics processes active.
/// <TRestPhysicsLists>
///     ...
/// </TRestPhysicsLists>
///
/// \endcode
///
/// \note Wherever 3 dots (`...`) are provided means a redundant code format, or
/// that additional fields might be required. 
///
/// Few basic working examples can be found at REST_PATH/config/template/restG4.rml
///
/// \note The runTag inside the TRestRun class will be overwritten by the name of
/// TRestG4Metadata section defined in the RML.
/// 
/// This page describes in detail the different parameters, particle generator
/// types, storage, and other features implemented in restG4, that can be defined
/// inside the section TRestG4Metadata. The description of other required sections,
/// as TRestRun or TRestPhysicsLists, will be found in their respective class
/// documentation.
/// 
/// We can sub-divide the information provided through TRestG4Metadata in 
/// different parts,
///
/// 1. the main parameters related to the simulation conditions, such as number
/// of events to be simulated, maximum step size, or GDML geometry definitions file, 
///
/// 2. the definition of the primary particle generator, using the `<generator>` 
/// section, to define which particles will be generated, from where they will be
/// generated, and which energies and direction distributions they will follow,
///
/// 3. the definition of what event hits will be written to disk, using the 
/// `<storage>` section,
///
/// 4. and the (optional) definition of biasing volumes to simulate particle
/// transmission through extended detector shieldings, using the `<biasing>`
/// section.
///
/// ## 1. Basic simulation parameters
///
/// The following list presents the common parameters that must be defined inside
/// TRestG4Metadata section to define few simulation conditions in *restG4*, as
/// important as the detector geometry, or the number of events to be simulated.
/// 
/// * **Nevents**: The number of primary particles to be generated. The number of
/// registered events might differ from *Nevents* due to storage definitions. The
/// number of events registered could be even larger than the number of primaries,
/// as for example when launching full decay chain simulations, where different 
/// isotope decays are stored in different events.
///
/// * **geometryPath**: The local path where the geometry files are stored.
///
/// [GDML]: https://gdml.web.cern.ch/GDML/
/// * **gdml_file**: The name of the main GDML file. In principle, the user has
/// full freedom to create any detector setup geometry using a [GDML][GDML] 
/// description.
/// \warning The only requirement is that the gas logical volume (implemented
/// in a single physical volume on the geometry) must be named `gasVolume`.
///
/// * **maxTargetStepSize**: This is the maximum integration step size allowed to
/// Geant4 when aproximating the interaction of particles in a medium. Lower
/// values will provide a more accurate description of the particle tracks, but
/// will also require an additional computation time.
/// \note The parameter **maxTargetStepSize** has only effect on the target
/// volume, that is the physical volume corresponding to the logical volume named
/// `gasVolume` in the geometry.
///
/// * **subEventTimeDelay**: This parameter defines the event time window. If a
/// Geant4 energy deposit has place after this time, those hits will be registered
/// as a fully independent event. Therefore, the total number of registered 
/// events could be higher than the initial number of events.
///
/// The following example ilustrates the definition of the common simulation
/// parameters.
///
/// \code
///    <parameter name="Nevents" value="100" />
///    <parameter name="geometryPath" value="${REST_PATH}/config/template/geometry/" />
///    <parameter name="gdml_file" value="mySetupTemplate.gdml"/>
///    <parameter name="maxTargetStepSize" value="200" units="um" />
///    <parameter name="subEventTimeDelay" value="100" units="us" />
/// \endcode
///
/// ## 2. The primary particle generator section
///
/// The generator section describes from where we will launch our primary
/// particles, which particles we will launch, the energy of these particles,
/// their angular distribution and other required functions needed to cover
/// different physics cases.
///
/// The complete structure of the primary particle generator follows this 
/// scheme.
///
/// \code
/// <generator type="generatorType" ... >
///
///     <source particle="particleName" ... >
///         <angularDist type="angularDistribution" />
///         <energyDist type="energyDistribution" energy="energyValues" units="MeV" />
///     </source>
///
///     <source particle="particleName" ... >
///         ...
///     </source>
///
///     // Add any number of sources.
///
/// </generator>
/// \endcode
/// 
/// ### The generator type definition
///
/// The different generator types determine the spatial origin of the primaries that
/// will be generated.
///
/// We can define any number of primary particles (sources) that will be used
/// to construct an initial event. All primary particles that build an
/// event will have a common position. The spatial origin of these sources, or
/// primaries, is specified in the definition of the generator, through the
/// generator *type* field.
///
/// We must define the *generatorType* inside the generator tag description as
/// follows.
/// \code
/// <generator type="generatorType" ... />
/// \endcode
///
/// Depending on the generator type we use, we will need to define additional
/// values to the generator definition.
///
/// The different types we can choose to use in *restG4*, are
/// detailed in the following list:
///
/// * **volume**: This generator will launch the events from random positions inside
/// a volume defined in the geometry. It requires to define the parameter
/// `from="physVolume"`, where `physVolume` is a physical volume name defined in our
/// GDML geometry description. This generator will launch homogeneously events from
/// the internal volume of `physVolume`. If there are empty holes in this physical
/// volumes we should take care to substract the different internal empty volumes in
/// the geometry definition, for this generator to work properly.
/// \code
///     // We launch particles from random positions inside the vessel
///     // volume defined in our GDML geometry
///     <generator type="volume" from="vessel" >
/// \endcode
///
/// * **surface**: This generator will launch homogeneously events from the surface
/// of a volume defined in the geometry. It also requires to define the additional
/// parameter `from="physVolume"`, using a physical volume name defined in our
/// GDML geometry description.
/// \code
///     // We launch particles from random positions at the surface of the cathode
///     // volume defined in our GDML geometry
///     <generator type="surface" from="cathode" >
/// \endcode
///
/// * **point**: All the particles will be launched from the same position. It
/// requires to define the additional parameter `position="(X,Y,Z)"` together with the
/// unit values.
/// \code
///     // We launch particles at a fix position at the XY origing and z=-15cm
///     <generator type="point" position="(0,0,-150)" units="mm" >
/// \endcode
///
/// * **virtualSphere**: It generates the events from the surface of a virtual (not
/// defined in the GDML geometry) sphere. It requires to define the parameters to place
/// and dimension the sphere, `position="(X,Y,Z)"` and `radius="R"`. Where `X,Y,Z` are
/// the coordinates of the center of the sphere and `R` is the radius of the sphere. If
/// the angular distribution is defined as `isotropic` the events will be launched only
/// towards the inside.
/// NB: the generic word "size" can replace "radius".
/// \code
///     // We launch particles from the surface of a virtual sphere,
///     // with center at (0,0,-100)mm, and radius 10cm.
///     <generator type="virtualSphere" position="(0,0,-100)" units="mm" radius="100" units="mm" >
/// \endcode
///
/// * **virtualWall**: It generates the events from a finite rectangular plane of a given length and width.
/// It requires to define the additional parameters `position="(X,Y,Z)"`,
/// `lenX="lX"`,`lenY="lY"` and `rotation="(rX,rY,rZ)"`. Where `X,Y,Z` is the center position of
/// the wall, `lX` and `lY` are respectively the size of the wall along th x-axis and the y-axis, and `rX`, `rY`, `rZ` are the rotation
/// angles applied to the wall. The wall is originally generated on the `XY` plane and
/// then rotated by the axis angles `rX`, `rY`, `rZ`, following that order.
/// NB: the generic word "size" an be used instead of "lenX" and "lenY", producing a square of the specified size.
/// \code
///     // We launch particles from the surface of a virtual wall,
///     // with center at (0,0,-100)mm, size of 10cm by 15cm, and rotate 45 degrees along x-axis.
///    <generator type="virtualWall" position="(0,0,-100)" units="mm" lenX="100" units="mm" lenY="150" units="mm" rotation="(45,0,0)" >
/// \endcode
///
/// * **virtualCylinder**: It generates the events from the surface of a virtual (not
/// defined in the GDML geometry) cylinder. It requires to define the additional
/// parameters `position="(X,Y,Z)"`, `radius="R"`, `length="L"` and
/// `rotation="(rX,rY,rZ)"`, where `X`, `Y`, `Z` is the center position of the
/// cylinder, `R` is the radius of the cylinder, `L` is the length of the cylinder,
/// and `rX`, `rY`, `rZ` are the rotation angles applied to the cylinder. The cylinder
/// is originally generated with its axis along the z-axis. In the cylinder case,
/// isotropic generator will launch events both sides, towards the inside and the
/// outside.
/// NB: the generic word "size" can replace "radius".
/// \code
///     // We launch particles from the surface of a virtual cylinder,
///     // with center at (0,0,-100)mm, radius 10cm, length 100cm and rotated 90 degrees along y-axis.
///     <generator type="virtualCylinder" position="(0,0,-100)" units="mm" radius="100" units="mm" length="100" units="cm" rotation="(0,90,0)" >
/// \endcode
///
/// * **virtualBox**: It generates the events from a surface of a virtual (not defined
/// in the GDML geometry) box, a cube of size L. It requires to define the additional parameters
/// `position="(X,Y,Z)"`, `size="L"`, where `X`, `Y`, `Z` is the center of the box, and
/// `L` is the length of the box side. The virtualBox does not implement rotation yet.
/// And if isotropic angular distribution is used events will be launched towards
/// the inside.
/// \code
///     // We launch particles from the surface of a virtual box,
///     // with center at (0,0,-100)mm, and size 10cm.
///     <generator type="virtualBox" position="(0,0,-100)" units="mm" size="100" units="mm" >
/// \endcode
///
/// ### The source definition
/// 
/// We can define any number of sources inside our generator. A source is defined 
/// using the following structure
///
/// \code
///     <source particle="particleName" ... >
///       <energyDist type="energyDistType" ... />
///       <angularDist type="angularDistType" ... />
///     </source>
/// \endcode
///
/// [leptons]: http://geant4.cern.ch/G4UsersDocuments/UsersGuides/ForApplicationDeveloper/html/Appendix/particleList.src/leptons/index.html
/// [ions]: http://geant4.cern.ch/G4UsersDocuments/UsersGuides/ForApplicationDeveloper/html/Appendix/particleList.src/ions/index.html
///
/// In the particle field we can use any pre-defined particle name existing in Geant4,
/// as [leptons][leptons] or [ions][ions]. Additionally, we can define any radioactive
/// isotope by using the element name symbol and the number of nucleons 
///(i.e. Rn222, Co60, U238 ). For the radiactive decays we can also define an additional
/// field `fullchain="on/off"`. This parameter specifies if we want to simulate the full
/// radiative chain or a single decay to the next de-excited isotope. Or, we can also 
/// use an external generator using the `fromFile=""` parameter.
///
/// In summary we can use the following options inside the source definition.
///
/// * **particle="G4_Particle"**: A particle name understood by Geant4 (i.e. e-, e+,
/// gamma, etc).
/// \code
///     // We launch gammas
///     <source particle="gamma">
/// \endcode
///
/// * **particle="Xn" fullchain="off"**: A radiactive isotope. Where X is the element
/// name and n the number of nucleons (I.e. Rn222, Xe137, Co60, etc).
/// \code
///     // We launch a single chain Rn222 decay
///     <source particle="Rn222" fullChain="off" >
/// \endcode
///
/// * **fromFile="Xe136bb0n.dat"**: The particles, energies, and direction are 
/// fully described by an input file. `<energyDistribution>` and 
/// `<angularDistribution>` source specifiers have no effect here. Everything will
/// be defined through the generator file. The file `Xe136bb0n.dat` is a pre-generated
/// `Decay0` file that can be found at `REST_PATH/inputData/generator`, together with
/// other generator examples.
/// \code
///     // We launch pre-generated 136Xe NLDBD events
///     <source fromFile="Xe136bb0n.dat">
///         // The energyDist and angularDist 
///         // definitions here will be just
///         // ignored with this type of source
///     </source>
/// \endcode
///
/// #### The source energy distribution
///
/// A source, or particle, associated kinetic energy is defined by using.
/// \code
/// <energyDist type="energyDistType" ... />
/// \endcode
///
/// There are different energy distribution types we can use to define the energy of
/// the primary particles. Depending on the type of energy distribution we will be
/// required to add additional parameters. The types we can use in REST are the
/// following types.
///
/// * **mono**: All the particles from this source will be launched with the same 
/// energy for each simulated event. It requires to define the parameter 
/// `energy="E"`, where `E` is the kinetic energy of the particle in keV.
/// \code
///     // A mono energetic particle with 10keV.
///     <energyDist type="mono" energy="10" units="keV" >
/// \endcode
///
/// * **flat**: All the particles from this source will be launched in a specified
/// energy range using a uniform energy distribution. It requires to define the 
/// parameter `range="(Ei,Ef)"`, where `Ei` is the minimum energy and `Ef` is the
/// maximum energy (in keV).
/// \code
///     // A random uniform generation between 1 and 10 keV
///     <energyDist type="mono" range="(1,10)" units="keV" >
/// \endcode
///
/// * **TH1D**: It will use a TH1D histogram from a ROOT file with a user defined
/// spectrum. It requires to define the parameters `file="mySpectrum.root"`
/// `spctName="histName"` and `range="(Ei,Ef)"`. The ROOT file should contain a
/// TH1D histogram with name `histName`. Only the region of the spectrum inside
/// the range `Ei-Ef` will be considered. The energy units of the spectum must be 
/// specified in the x-axis label of the TH1D object. If no units are given
/// in the x-label, the default units considered will be keV.
/// \code
///     // A TH1D input spectrum to produce underground muons in the range between 150 and 400 GeV
///     <energyDist type="TH1D" file="Muons.root" spctName="LSCMuon" range="(150,400)" units="GeV" >
/// \endcode
///
/// #### The source angular distribution
///       
/// The momentum direction of a particle is specified by using.
/// \code
/// <angularDist type="angularDistType" ... />
/// \endcode
///
/// We can use different types of angular distributions to define the 
/// initial direction of the particle. We can choose between the following
/// types inside REST.
/// 
/// * **flux**: It defines a fixed direction for the particle momentum. We
/// must specify the additional parameter `direction="(pX,pY,pZ)"` as the
/// cartesian components of the momentum direction (not required to be
/// normalized).
/// \code
///     // Particles will be launched in the positive y-axis direction.
///     <angularDist type="flux" direction="(0,1,0)" >
/// \endcode
///       
/// * **isotropic**: The momentum direction of each particle will be
/// generated homogeneously. In case of *virtualSphere* and *virtualBox*
/// only the particles travelling towards the inside of the virtual
/// volume will be considered.
/// \code
///     // Particles will be launched without prefered direction.
///     <angularDist type="isotropic" >
/// \endcode
/// 
/// * **backtoback**: The source momentum direction will be oposite to
/// the direction of the previous source. If it is the first source the
/// angular distribution type will be re-defined to isotropic.
/// \code
///     // Particles will be launched without prefered direction.
///     <angularDist type="backtoback" >
/// \endcode
/// 
/// * **TH1D** : It will use a TH1D histogram from a ROOT file with a
/// user defined angular distribution. It requires to define the 
/// additional parameters as `file="mySpectrum.root"` and 
/// `spctName="histName"`. The file we give should be stored in 
/// `"inputData/distributions/"` and contain a TH1D histogram with 
/// name "histName".
/// \code
///     // A TH1D input angular distribution used for cosmic rays
///     <angularDist type="TH1D" file="CosmicAngles.root" spctName="Theta2">
/// \endcode
/// 
/// ## 3. The storage section definition
///
/// The information we store in the ROOT file can be defined using the
/// storage section. The storage section is defined as follows
///
///  \code
///  <storage sensitiveVolume="gas">
///
///     <parameter name="energyRange" value="(0,5)" units="MeV" />
///
///      <activeVolume name="gas" chance="1" />
///      // Add as many active volumes as needed
///
///  </storage>
///  \endcode
///
/// The storage section defines the `sensitiveVolume`, and the active
/// volumes where data will be stored.
///  \code
///  <storage sensitiveVolume="gas">
///  \endcode
///
/// The sensitive volume can be any physical volume defined on the GDML geometry.
/// If an event did not produce an energy deposit in the sensitiveVolume,
/// the event will not be stored at all. Therefore, the sensitive volume
/// will serve as a trigger volume to decide when an event should be 
/// stored.
///
/// We can define the energy range we are interested in by defining the 
/// parameter energyRange. The event will be written to disk only if the
/// total energy deposited in all the active volumes of the detector is
/// in the Ei-Ef range.
///
/// \code
/// <parameter name="energyRange" value="(Ei,Ef)" />
/// \endcode
///
/// This energy range implies that only events that produced an energy
/// deposit at the sensitive volume, and produced a total energy
/// deposit between Ei and Ef, integrated to all the active volumes, will
/// be stored. 
///
/// We should define inside the `<storage>` definition all the physical
/// volumes where we want hits to be stored using `<activeVolume>`
/// definition.
/// 
/// \code
/// <activeVolume name="gas" chance="1" />
/// \endcode
/// 
/// In general, we will always want to store all the hits in the gas volume
/// of the TPC. But the gas being the sensitive volume does not mean that
/// hits on the gas volume will be stored. This volume should be activated
/// for storage even if we defined it as the sensitive volume. The user
/// may decide to do not store the hits in the gas volume. For example, we
/// may want to track events that deposit something on the gas volume, 
/// through the hits in another volume, but not be interested to study the
/// hits inside the gas for a particular case.
/// 
/// Each active volume defines a chance parameter. This parameter gives the
/// probability to store the hits of an event in a particular volume. For
/// example,
/// 
/// \code
///      <activeVolume name="gas" chance="1" />
///      <activeVolume name="vessel" chance="0.1" />
/// \endcode
///
/// will store all the hits produced in the gas, and 10% of the events will
/// include information of the hits in the vessel. This may be used as a
/// control volume to allow further study of events depositing energy in the
/// vessel, but saving some space in disk in case we do not need to use all 
/// the event population.
/// 
///
/// ## 4. The biasing volumes section (optional)
///
/// The REST Geant4 toolkit (*restG4*) implements a particular biasing 
/// technique to simulate external radiation contributions in extense 
/// shieldings. The technique consists in spatially placing biasing volumes
/// between the initial particle generator and the detector (or sensitive
/// volume).
///
/// The largest biasing volume must be fully contained inside the original 
/// event generator. And the smaller biasing volume should fully contain
/// all the volumes where hits will be stored. The different biasing 
/// volumes must be fully containned one in each other (in other words, 
/// the biasing volumes should not overlap).
///
/// In practice, *restG4* will run a total of N+1 *Geant4* simulations,
/// being N the number of biasing volumes defined. The first simulation
/// will just count the number of particles of the specified type traversing
/// the biasing volume, registering also their energy and angular
/// distribution. In the second simulation, the number of particles that
/// traversed the biasing volume will be multiplied by a factor, that is
/// defined independently at each biasing volume, using the recorded energy
/// spectrum from the previous simulation. This will continue until the
/// last smaller biasing volume is reached. Only in the last simulation,
/// when events are launched from the smaller biasing volume, hits will be
/// stored.
///
/// This biasing technique has been already tested using a virtualBox 
/// generator and virtualBox biasing volumes, leading to compatible results
/// if compared to non biased simulations. When using this technique it is
/// recommended that you first do a small scale version of your simulation
/// to compare biased and non-biased results. If both results (from biased
/// and non-biased) agree then you will be more confident you will not 
/// obtain misleading results.
///
/// In order to obtain the best results using this technique you should try
/// to keep the same number of events reaching each biasing volume. First, 
/// some optimization is required. You will need to empirically test the
/// number of events reaching each biasing volume, and adjust the factors
/// according to these results. In order to have reasonable good results
/// you should assure to have at least 10,000 events reaching each of the
/// biasing volumes. If some energies are very unlikely, and not enough
/// statistics exist at the biasing volume the error will be much higher.
/// This should be solved in a future implementation by using weighting
/// factors.
///
/// The biasing technique needs to be activated using the following scheme
///
/// \code
/// <biasing value="on" type="virtualBox,virtualSphere" >
///
///     <biasingVolume particle="particleName" size="L" position="(X,Y,Z)" factor="F" energyRange="(Ei,Ef)" />
///     // Add as many biasing volumes as desired
///
/// </biasing>
/// \endcode
///
/// \note The biasing is optional, and only required in exceptional cirumstances, 
/// most of the times will be disabled by using `value="off"`. 
///
/// We can use two spatial generators to define the biasing volumes (virtualBox
/// and virtualSphere). It is recommended to use the same type as the original
/// generator. But it is left to the user to make his choice.
/// 
/// \warning The volumes should be placed in increasing order. From smaller to 
/// larger volume.
///
///  A biasing volume is defined as follows
///
/// \code
/// <biasingVolume particle="particleName" size="L" position="(X,Y,Z)" factor="F" energyRange="(Ei,Ef)" />
/// \endcode
///
/// The particle name may be any particle name defined in Geant4. The biasing
/// particle we define is the only particle that will be transferred to the next
/// biasing volume. Therefore, this biasing method is used to study the 
/// transmission of a given particle gamma, neutron, etc through a shielding).
/// The size defines the side of the virtualBox or the radius of the virtualSphere,
/// its position `X`, `Y`, `Z` and a factor `F` that is the multiplication factor
/// to be used in the next run (In other words, the number of particles launched
/// from a biasing volume will be the number of particles that reached the volume
/// times `F`). Only the particles in the specified energyRange `(Ei,Ef)` will be
/// considered in the transmission to the next biasing volume.
///
///
///--------------------------------------------------------------------------
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2015-july: First concept and implementation of TRestG4 classes.
///            Javier Galán
///
///
/// \class      TRestG4Metadata
/// \author     Javier Galan
///
/// <hr>
///
#include "TRestG4Metadata.h"
using namespace std;

ClassImp(TRestG4Metadata)
	///////////////////////////////////////////////
	/// \brief Default constructor
	///
TRestG4Metadata::TRestG4Metadata() : TRestMetadata()
{
	Initialize();
}

///////////////////////////////////////////////
/// \brief Constructor loading data from a config file
/// 
/// If no configuration path is defined using TRestMetadata::SetConfigFilePath
/// the path to the config file must be specified using full path, absolute or relative.
///
/// The default behaviour is that the config file must be specified with 
/// full path, absolute or relative.
///
/// \param cfgFileName A const char* giving the path to an RML file.
/// \param name The name of the specific metadata. It will be used to find the correspondig TRestG4Metadata section inside the RML.
///
TRestG4Metadata::TRestG4Metadata( char *cfgFileName, string name ) : TRestMetadata (cfgFileName)
{

	Initialize();

	LoadConfigFromFile( fConfigFileName, name );

	PrintMetadata();
}

///////////////////////////////////////////////
/// \brief Default destructor 
/// 
TRestG4Metadata::~TRestG4Metadata()
{
}

///////////////////////////////////////////////
/// \brief Initialization of TRestG4Metadata members
/// 
void TRestG4Metadata::Initialize()
{
	SetSectionName( this->ClassName() );

	fChance.clear();
	fActiveVolumes.clear();
	fBiasingVolumes.clear();

	fNBiasingVolumes = 0;

	fGeneratorFile = "";

	fPrimaryGenerator.Reset();

	fSensitiveVolume = "gas";

	fEnergyRangeStored.Set( 0, 1.E20 );
}

///////////////////////////////////////////////
/// \brief Initialization of TRestG4Metadata members through a RML file
/// 
void TRestG4Metadata::InitFromConfigFile()
{
	this->Initialize();

	// Initialize the metadata members from a configfile
	fGDML_Filename = GetParameter( "gdml_file" );

	fGeometryPath = GetParameter( "geometryPath" );

	Double_t defaultStep = 100 * REST_Units::um;
	fMaxTargetStepSize = GetDblParameterWithUnits( "maxTargetStepSize", defaultStep );

	Double_t defaultTime = 1. / REST_Units::s;
	fSubEventTimeDelay = GetDblParameterWithUnits( "subEventTimeDelay", defaultTime );

	fNEvents = StringToInteger( GetParameter( "Nevents" ) );
	//fActivity = StringToDouble( GetParameter( "activity" ) );
	//fMass = StringToDouble( GetParameter( "mass" ) );

	ReadGenerator();

	ReadStorage();

	ReadBiasing();

	// should return sucess or fail

}

///////////////////////////////////////////////
/// \brief Reads the biasing section defined inside TRestG4Metadata.
/// 
/// This section allows to define the size and properties of any number of biasing 
/// volumes. Biasing volume properties include the multiplicity factor and the 
/// range of energies that particles will be propogated to the next biasing volume.
/// 
/// Check for more details in the general description of this class.
/// 
void TRestG4Metadata::ReadBiasing()
{
	string biasingString = GetKEYStructure( "biasing" );

	if ( biasingString == "NotFound" )
	{
		fNBiasingVolumes = 0;
		return;
	}

	string biasingDefinition = GetKEYDefinition( "biasing", biasingString );

	TString biasEnabled = GetFieldValue( "value", biasingDefinition );
	TString biasType =  GetFieldValue( "type", biasingDefinition );

	cout << "Bias : " << biasEnabled << endl;

	if( biasEnabled == "on" || biasEnabled == "ON" || biasEnabled == "On" || biasEnabled == "oN" )
	{
		cout << "Biasing is enabled" << endl;

		size_t position = 0;
		string biasVolumeDefinition;

		Int_t n = 0;
		while( ( biasVolumeDefinition = GetKEYDefinition( "biasingVolume", position, biasingString ) ) != "" )
		{
			TRestBiasingVolume biasVolume;

			cout << "Def : " << biasVolumeDefinition << endl;

			biasVolume.SetBiasingVolumePosition( Get3DVectorFieldValueWithUnits( "position", biasVolumeDefinition ) );
			biasVolume.SetBiasingFactor( StringToDouble( GetFieldValue( "factor", biasVolumeDefinition ) ) );
			biasVolume.SetBiasingVolumeSize( GetDblFieldValueWithUnits( "size", biasVolumeDefinition ) );
			biasVolume.SetEnergyRange( Get2DVectorFieldValueWithUnits( "energyRange", biasVolumeDefinition ) );
			biasVolume.SetBiasingVolumeType( biasType ); // For the moment all the volumes should be same type

			/* TODO check that values are right if not printBiasingVolume with getchar()
			   biasVolume.PrintBiasingVolume();
			   getchar();
			   */

			fBiasingVolumes.push_back( biasVolume );

			n++;
		}
		fNBiasingVolumes = n;
	}


}

///////////////////////////////////////////////
/// \brief Reads the generator section defined inside TRestG4Metadata.
/// 
/// This section allows to define the primary particles to be simulated.
/// The generator will allow us to define from where we will launch the 
/// primary particles, their energy distribution, and their angular 
/// momentum.
/// 
void TRestG4Metadata::ReadGenerator()
{

	// TODO if some fields are defined in the generator but not finally used
	// i.e. <generator type="volume" from="gasTarget" position="(100,0,-100)
	// here position is irrelevant since the event will be generated from the volume defined in the geometry
	// we should take care of these values so they are not stored in metadata (to avoid future confusion)
	// In the particular case of position, the value is overwritten in DetectorConstruction by the center of the volume (i.e. gasTarget)
	// but if i.e rotation or side are defined and not relevant we should set it to -1

	string generatorString = GetKEYStructure( "generator" );

	string generatorDefinition = GetKEYDefinition( "generator", generatorString );

	fGenType = GetFieldValue( "type", generatorDefinition );
	fGenFrom = GetFieldValue( "from", generatorDefinition );

	string dimension1 [3]{"size","lenX","radius"};
	for (int i = 0; i < 3; i++)
	{
		fGenDimension1 = GetDblFieldValueWithUnits( dimension1[i], generatorDefinition );
		if ( fGenDimension1 != PARAMETER_NOT_FOUND_DBL )
		{
			if ( dimension1[i] == "size" )
				fGenDimension2 = fGenDimension1;
			break;
		}
	}

	// TODO : If not defined (and required to be) it just returns (0,0,0) we should make a WARNING. Inside StringToVector probably
	fGenPosition = Get3DVectorFieldValueWithUnits( "position", generatorDefinition );

	fGenRotation = StringTo3DVector ( GetFieldValue( "rotation", generatorDefinition ) );

	string dimension2 [2]{"length","lenY"};
	for (int i = 0; i < 2; i++)
	{
		Double_t tmpDim2 = GetDblFieldValueWithUnits( dimension2[i], generatorDefinition );
		if (tmpDim2 != PARAMETER_NOT_FOUND_DBL )
		{
			fGenDimension2 = tmpDim2;
			break;
		}
	}

	size_t position = 0;
	string sourceString;

	Int_t n = 0;
	while( ( sourceString = GetKEYStructure( "source", position, generatorString ) ) != "" )
	{
		TRestParticleSource source;

		// Source parameters
		string sourceDefinition = GetKEYDefinition( "source", sourceString );

		fGeneratorFile = GetFieldValue( "fromFile", sourceDefinition );

		if( fGeneratorFile != "Not defined" )
		{
			cout << "Reading sources from generator file : " << fGeneratorFile << endl;
			ReadGeneratorFile( fGeneratorFile );

			break;
		}

		source.SetParticleName( GetFieldValue( "particle", sourceDefinition ) );

		if( GetVerboseLevel() >= REST_Debug )
			cout << "Particle name : " << source.GetParticle() << endl;

		source.SetExcitationLevel( StringToDouble(GetFieldValue( "excitedLevel", sourceDefinition  )) );

		TString fullChain = GetFieldValue( "fullChain", sourceDefinition );

		if( fullChain == "on" || fullChain == "ON" || fullChain == "On" || fullChain == "oN" )
		{
			SetFullChain( true );
		}
		else
		{
			SetFullChain( false );
		}

		// Angular distribution parameters
		string angularDefinition = GetKEYDefinition( "angularDist", sourceString );

		source.SetAngularDistType( GetFieldValue( "type", angularDefinition ) );

		if( source.GetAngularDistType() == "TH1D" )
		{

			source.SetAngularFilename( GetFieldValue( "file", angularDefinition ) );
			source.SetAngularName( GetFieldValue( "spctName", angularDefinition ) );
		}

		if( n == 0 && source.GetAngularDistType() == "backtoback" )
		{
			cout << "WARNING: First source cannot be backtoback. Setting it to isotropic" << endl;
			source.SetAngularDistType( "isotropic");

		}

		source.SetDirection( StringTo3DVector( GetFieldValue( "direction", angularDefinition ) ) );

		// Energy distribution parameters
		string energyDefinition = GetKEYDefinition( "energyDist", sourceString );

		source.SetEnergyDistType( GetFieldValue( "type", energyDefinition ) );

		if( source.GetEnergyDistType() == "TH1D" )
		{
			source.SetSpectrumFilename ( GetFieldValue( "file", energyDefinition ) );
			source.SetSpectrumName ( GetFieldValue( "spctName", energyDefinition ) );
		}

		source.SetEnergyRange( Get2DVectorFieldValueWithUnits ( "range", energyDefinition ) );

		if( source.GetEnergyDistType() == "mono" )
		{
			Double_t en;
			en = GetDblFieldValueWithUnits( "energy", energyDefinition );
			source.SetEnergyRange ( TVector2 (en,en) );
			source.SetEnergy( en );
		}

		fPrimaryGenerator.AddSource( source );

		n++;
	}

}

///////////////////////////////////////////////
/// \brief Reads the storage section defined inside TRestG4Metadata.
/// 
/// This section allows to define which hits will be stored to disk.
/// Different volumes in the geometry can be tagged for hit storage.
/// 
void TRestG4Metadata::ReadStorage( )
{

	string storageString = GetKEYStructure( "storage" );
	fSensitiveVolume = GetFieldValue( "sensitiveVolume", storageString  );
	cout << "Sensitive volume : " << fSensitiveVolume << endl;
	if( fSensitiveVolume == "Not defined" )
	{
		cout << "REST WARNNING : Sensitive volume not defined. Setting it to gas!!!!" << endl;
		fSensitiveVolume = "gas";
	}

	cout << fSensitiveVolume << endl;

	size_t pos = 0;
	fEnergyRangeStored = Get2DVectorParameterWithUnits( "energyRange", pos, storageString  );

	pos = 0;
	string volumeDefinition;
	while( (volumeDefinition = GetKEYDefinition( "activeVolume", pos, storageString )) != "" )
	{
		Double_t chance = StringToDouble( GetFieldValue( "chance", volumeDefinition ) );
		TString name = GetFieldValue( "name", volumeDefinition );
		SetActiveVolume( name, chance );
		cout << "Adding active volume" << endl;
	}

}

///////////////////////////////////////////////
/// \brief Prints on screen the details about the Geant4 simulation
/// conditions, stored in TRestG4Metadata.  
///
void TRestG4Metadata::PrintMetadata( )
{

	cout << endl;
	cout << "+++++++++++++++++++++++++++++++++++++++++++++" << endl;
	cout << "TRestG4Metadata content" << endl;
	cout << "+++++++++++++++++++++++++++++++++++++++++++++" << endl;
	cout << "Config file : " << fConfigFileName << endl;
	cout << "Section name : " << fSectionName << endl;        // section name given in the constructor of TRestSpecificMetadata
	cout << "---------------------------------------" << endl;
	cout << "Name : " << GetName() << endl;
	cout << "Title : " << GetTitle() << endl;
	cout << "---------------------------------------" << endl;
	cout << "Geometry File : " << Get_GDML_Filename() << endl;
	cout << "Geometry Path : " << GetGeometryPath() << endl;
	cout << "Max. Step size : " << GetMaxTargetStepSize() << " mm" << endl;
	cout << "Sub-event time delay : " << GetSubEventTimeDelay() << " us" << endl;
	cout << "---------------------------------------" << endl;
	TString generatorType = GetGeneratorType();
    cout << "Number of generated events : " << GetNumberOfEvents() << endl;
	cout << "Generator type : " << generatorType << endl;
	cout << "Generated from : " << GetGeneratedFrom() << endl;
	TVector3 a = GetGeneratorPosition();
	cout << "Generator center : (" << a.X() << "," << a.Y() << "," << a.Z() << ") mm" << endl;
	TVector3 b = GetGeneratorRotation();
	cout << "Generator rotation : (" << b.X() << "," << b.Y() << "," << b.Z() << ") mm" << endl;
	if ( generatorType == "virtualSphere" )
		cout << "Generator radius : " << GetGeneratorRadius() << " mm" << endl;
	else if ( generatorType ==  "virtualWall" )
	{
		cout << "Generator lenX : " << GetGeneratorLenX() << " mm" << endl;
		cout << "Generator lenY : " << GetGeneratorLenY() << " mm" << endl;
	}
	else if ( generatorType ==  "virtualCylinder" )
	{
		cout << "Generator radius : " << GetGeneratorRadius() << " mm" << endl;
		cout << "Generator length : " << GetGeneratorLength() << " mm" << endl;
	}
	else if ( generatorType ==  "virtualBox" )
		cout << "Generator size : " << GetGeneratorSize() << " mm" << endl;
	cout << "---------------------------------------" << endl;
	cout << "Number of primary particles : " << GetNumberOfPrimaries() << endl;
	cout << "Generator file : " << GetGeneratorFile() << endl;
	cout << "***************************************" << endl;

	for( int n = 0; n < GetNumberOfPrimaries(); n++ )
	{
		TRestParticleSource src = GetParticleSource(n);
		src.PrintParticleSource();
	}

	cout << "Storage." << endl;
	cout << "***************************************" << endl;
	cout << "Energy range : Emin = " << GetMinimumEnergyStored() << " Emax : " << GetMaximumEnergyStored() << endl;
	cout << "Sensitive volume : " << GetSensitiveVolume() << endl;
	cout << "Active volumes : " << GetNumberOfActiveVolumes() << endl;
	cout << "---------------------------------------" << endl;
	for( int n = 0; n < GetNumberOfActiveVolumes(); n++ )
	{
		cout << "Name : " << GetActiveVolumeName(n) << " ID : " << GetActiveVolumeID( GetActiveVolumeName(n) ) << " chance : " << GetStorageChance( GetActiveVolumeName(n) ) << endl;

	}
	cout << "+++++++++++++++++++++++++++++++++++++++++++++" << endl;
	cout << "List of biasing volumes" << endl;
	cout << "+++++++++++++++++++++++++++++++++++++++++++++" << endl;
	for( int n = 0; n < GetNumberOfBiasingVolumes(); n++ )
	{
		GetBiasingVolume( n ).PrintBiasingVolume();
	}

}

///////////////////////////////////////////////
/// \brief Reads an input file produced by Decay0.
///
/// The input file should contain the description of several 
/// pre-generated events, providing the names (or ids) of 
/// particles to be produced, their energy, and momentum.
/// The particles and their properties are stored in a
/// TRestParticleCollection which will be randomly accessed
/// by the restG4 package.
///
/// \param fName The Decay0 filename located at 
/// REST_PATH/inputData/generator/
///
void TRestG4Metadata::ReadGeneratorFile( TString fName )
{
	TString fullFilename = (TString) getenv("REST_PATH") +  "/inputData/generator/" + fName;

	char cadena[256];
	int tmpInt;

	FILE *fFileIn;

	if ( (fFileIn = fopen(fullFilename.Data(),"rt") ) == NULL ) {
		printf("Error en el archivo %s\n", fullFilename.Data());
		return;
	}

	// First lines are discarded.
	for( int i = 0 ; i < 20 ; i++)
	{
		if( fscanf( fFileIn, "%[^\n]\n", cadena) <= 0 )
		{
			cout << "REST Error!!. TRestG4Metadata::ReadGeneratorFile. Contact rest-dev@cern.ch" << endl;
			exit(-1);
		}
		if( strcmp( cadena, "First event and full number of events:" ) == 0 ) break;
	}

	Int_t fGeneratorEvents;
	if( fscanf( fFileIn, "%d %d\n", &tmpInt, &fGeneratorEvents) <= 0 ) // The number of events.
	{
		cout << "REST Error!!. TRestG4Metadata::ReadGeneratorFile. Contact rest-dev@cern.ch" << endl;
		exit(-1);
	}

	// cout << "i : " << tmpInt << " fN : " << fGeneratorEvents << endl;

	TRestParticleCollection particleCollection;

	TRestParticle particle;


	cout << "Reading generator file : " << fName << endl;
	cout << "Total number of events : " << fGeneratorEvents << endl;

	for( int n = 0; n < fGeneratorEvents; n++ )
	{
		particleCollection.RemoveParticles();

		Int_t nParticles;
		Int_t evID;
		Double_t time;


		if( fscanf(fFileIn,"%d %lf %d\n", &evID, &time, &nParticles) <= 0 )
		{
			cout << "REST Error!!. TRestG4Metadata::ReadGeneratorFile. Contact rest-dev@cern.ch" << endl;
			exit(-1);
		}

		for( int i = 0; i < nParticles; i++)
		{
			Int_t pID;
			Double_t momx, momy, momz, mass;
			Double_t energy = -1, momentum2;

			if ( fscanf( fFileIn, "%d %lf %lf %lf %lf\n", &pID, &momx, &momy, &momz, &time) <= 0 )
			{
				cout << "REST Error!!. TRestG4Metadata::ReadGeneratorFile. Contact rest-dev@cern.ch" << endl;
				exit(-1);
			}

			if( pID == 3)
			{
				momentum2 = (momx * momx) + (momy * momy) + (momz * momz);
				mass = 0.511;

				energy = TMath::Sqrt( momentum2 + mass*mass ) - mass;
				particle.SetParticleName( "e-" );

			}
			else
			{
				cout << "Particle id " << pID << " not recognized" << endl;

			}

			TVector3 momDirection( momx, momy, momz );
			momDirection = momDirection.Unit();

			particle.SetEnergy( 1000. * energy );
			particle.SetDirection( momDirection );

			particleCollection.AddParticle( particle );
		}

		fPrimaryGenerator.AddParticleCollection( particleCollection );
	}

	fPrimaryGenerator.SetSourcesFromParticleCollection( 0 );

}

///////////////////////////////////////////////
/// \brief Returns the id of an active volume giving as parameter its name.
Int_t TRestG4Metadata::GetActiveVolumeID( TString name )
{
	Int_t id;
	for( id = 0; id < (Int_t) fActiveVolumes.size(); id++ )
	{
		if( fActiveVolumes[id] == name ) return id;
	}
	return -1;
}

///////////////////////////////////////////////
/// \brief Adds a geometry volume to the list of active volumes.
///
/// \param name The name of the volume to be added to the active volumes list for storage.
///             Using GDML naming convention.
/// \param chance Probability that for a particular event the hits are stored in that volume.
///               The aim of this parameter is to define control volumes.
///               Usually the volume of interest will be always registered (chance=1).
///
void TRestG4Metadata::SetActiveVolume( TString name, Double_t chance )
{
	fActiveVolumes.push_back( name );
	fChance.push_back( chance );
}

///////////////////////////////////////////////
/// \brief Returns true if the volume named *volName* has been registered for data storage.
///
Bool_t TRestG4Metadata::isVolumeStored( TString volName )
{
	for( int n = 0; n < GetNumberOfActiveVolumes(); n++ )
		if( GetActiveVolumeName( n ) == volName )
			return true;

	return false;
}

///////////////////////////////////////////////
/// \brief Returns the probability of an active volume being stored
///
Double_t TRestG4Metadata::GetStorageChance( TString vol )
{
	Int_t id;
	for( id = 0; id < (Int_t) fActiveVolumes.size(); id++ )
	{
		if( fActiveVolumes[id] == vol ) return fChance[id];
	}
	cout << "STORAGE VOLUME NOT FOUND" << endl;

	return 0;
}
