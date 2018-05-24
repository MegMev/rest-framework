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
///
/// One of the core classes of REST. Abstract class
/// from which all REST "metadata classes" must derive.
/// A metadata class in REST is any holder of data other than event data
/// that is relevant to understand the origin and history of 
/// transformations that a given set of event data has gone through. 
/// For example the geometry of a simulation,
/// the parameters of a process, the properties of a gas, 
/// the readout pattern used to "pixelize" data, etc... are examples 
/// of metadata.
/// All metadata classes can be "initialized" via
/// configuration (.rml) files that the user can write. Alternatively
/// they can be read from root files. TRestMetadata contains 
/// the common functionality that allows metadata to be read from .rml
/// files or previously `stored` TRestMetadata structures stored in a 
///  ROOT file.
///
/// ### RML file structure 
///
/// A class deriving from TRestMetadata can retrieve information from a plain text
/// configuration file (or RML file). The syntaxis in an RML file is imposed by
/// TRestMetadata. The metadata information provided through an RML file to a class
/// deriving from TRestMetadata corresponds to a *section* structure inside the file.
/// The following piece of code shows the common structure of the metadata description 
/// corresponding to a specific metadata class.
///
/// \code
///
/// <section sectionName name="userGivenName" title="User given title" >
///
///     <keyStructure field1="value1" field2="value2" ... >
///
///         <keyDefinition field1="value1" field2="value2">  
///         
///          ...
///
///     </keyStructure>
///
/// </section>
///
/// \endcode
/// 
/// The section can also be defined skipping the *section* keyword,
/// using the following convention.
///
/// \code
///
/// <sectionName name="userGivenName" title="User given title" >
///
///     <keyStructure field1="value1" field2="value2" ... >
///
///     ...
///
/// </sectionName>
///
/// \endcode
///
/// The derived class from TRestMetadata is resposible to define the name of the
/// section (*sectionName*) used to extract the corresponding metadata section 
/// and store it in TRestMetadata::configBuffer. The default behaviour in REST is
/// that *sectionName* will be the name of the specific metadata class.
///
/// This must be implemented at each specific metadata class, at the construction 
/// time, by implementing the TRestMetadata::Initialize method, as follows:
///
/// \code
/// void TRestSpecificMetadata::Initialize( )
/// {
///
///     SetSectionName( this->ClassName() );
///
///     ....
///
/// }
/// \endcode
///
/// The methods defined inside TRestMetadata class allow to extract different metadata 
/// structures with openning-closing tags (as the *keyStructure* shown in the previous 
/// code), and definitions (as in the definition *keyDefinition* shown in the previous 
/// code).
///
/// Each specific metadata class is responsible to extract the information found in its
/// section. The initialization of a specific metadata class through a RML file should
/// be implemented in TRestMetadata::InitFromConfigFile( ).
///
/// The derived metadata class can access to the different structures using the 
/// different methods provided, as TRestMetadata::GetKEYStructure, 
/// TRestMetadata::GetKEYDefinition, etc. If no string *buffer* is given as argument, 
/// the specific *keyStructure* or *keyDefinition* we are looking for will be searched 
/// in the entire metadata section found in TRestMetadata::configBuffer. The search will
/// start from the beginning of the section, except that a position *fromPostion* is 
/// specified by argument. The first match will be returned in a string.
///
/// In order to read several key definitions or structures with the same name we must
/// to provide as argument a position (size_t &fromPosition) that it is updated with 
/// the position where the end of the structure or definition read by the method is 
/// found. This position value can be given to the next method call to read the next
/// definition. You might find useful examples of use in the implementations of
/// complex metadata structures as in TRestG4Metadata::InitFromConfigFile and 
/// TRestReadout::InitFromConfigFile.
///
/// ### Using system environment variables in RML files
/// 
/// RML files allow to retrieve environment variables defined in our system. This feature
/// may result specially useful to generate configuration templates that can be used for
/// different purposes. The environment variables must be introduced by using curly 
/// brackets (i.e. {USER}).
///
/// The RML file will be parsed and the words contained inside {} will be replaced by
/// their corresponding system variable. 
///
/// We can define externally different variables in our environment (i.e. in bash we
/// could use *export RUN_NUMBER="101"*). Then, define in our RML the field value 
/// **runNumber** from TRestRun, using that value:
///
/// \code
///
/// <parameter name="runNumber" value="{RUN_NUMBER}" />
///
/// \endcode
/// 
/// ### Defining local environment variables in RML files
///
/// If we write an RML file relying on many environmental variables, and some of those
/// variables have not been defined in the system environment, REST will complain during
/// exexution time of the undefined variable. A solution is to define inside the RML
/// file default values for those variables by using the *environment* section.
///
/// \code
///   <environment>
///       <variable name="ISOTOPE" value="Rn222" overwrite="false" />
///       <variable name="FULLCHAIN" value="on" overwrite="true" />
///   </environment>
///
/// \endcode
/// 
/// This section allows to define those variables, that at the same time could allow
/// us to identify the most important definitions in our RML file. Additionally, we 
/// can protect the local variable we have defined from the system definition. Therefore,
/// if the **overwrite** parameter is set as **false**, the external definition of the
/// environment variable will not have effect on the RML, and the local definition will
/// not be overwritten.
///
/// ### Including external RML files in a main RML file
///
/// We have the possibility to add several section definitions in an external file, and then include,
/// or link, them in a main indexing file. We should define the name of the section we want to include,
/// and the file where we can find the real complete metadata description.
///
/// For example, the following code lines would implement the readout and gas definitions found in 
/// external files.
///
/// \code
///
/// <section TRestReadout nameref="Readout-PANDA_3MM_Single" file="{REST_PATH}/inputData/definitions/readouts.rml"> </section>
///
/// <section TRestGas nameref="Xenon-TMA 3Pct 10-10E3V/cm" file="{REST_PATH}/inputData/definitions/gases.rml"> </section>
///
/// \endcode
///
/// This link is made by definning the *nameref* and *file* fields in the section definition. It is
/// still important to close the section definition using </section>.
///
/// ### Including an external text file inside an RML section
///
/// We can also include an external file content **inside** a section. The contents of the external
/// file will be dumped inside the section and replace the <include statement used to include the file.
///
/// As in the following example:
///
/// \code
/// <section TRestXX .... >
///
/// ...
///
/// <include file="/full/path/file.xml" />
///
/// ...
///
/// </section>
///
/// \endcode
/// 
/// ### The globals section
///
/// The *globals* section allows to specify few common definitions used in the REST 
/// framework. As the output data path, the gas data path, verbose level, etc. An example
/// of this section definition follows.
///
/// \code
/// <globals>
///    <parameter name="mainDataPath" value="{REST_DATAPATH}" />
///    <parameter name="gasDataPath" value="{GAS_PATH}" /> // allows to modify the default gas path
///    <parameter name="verboseLevel" value="debug" /> // options are : silent, warning, info, debug 
/// </globals>
/// \endcode
///
/// The global section will have effect on *all the metadata structures* (or sections) that are
/// defined in a same RML file. It does not affect to other possible linked sections defined by 
/// reference using for example nameref.
///
/// ### Definning the output level
///
/// We can change the amount of output REST will print on screen by definning different verbose
/// levels. The *verboseLevel* defined inside the *globals* section will be the default output
/// level for all the metadata sections defined in that RML file. 
///
/// However, we can define the verbose level for any particular metadata structure (including 
/// processes, since they derive from TRestMetadata ). In order to change the output level for
/// a particular section we must add the *verboseLevel* field in its definition.
///
/// The following line would print on screen any debug message implemented in 
/// TRestSignalAnalysisProcess.
///
/// \code
/// <section TRestSignalAnalysisProcess name="sgnlAna" title="Data analysis" verboseLevel="debug" >
/// \endcode
///
///
/// ### Using physical units in fields definitions 
///
/// Some physical parameters need to specify the unit so that the provided value makes 
/// sense. For example, when defining the electric field we must provide its units.
///
/// \code
/// <parameter name="electricField" value="1000" units="V/cm" />
/// \endcode
/// 
/// The implementation inside TRestMetadata::InitFromConfigFile requires we 
/// specify that we will read the field value with units, by using the method(s)
/// TRestMetadata::GetDblParameterWithUnits. If we use this method, and no units
/// are provided in the RML file REST will complain of the inexistence of the
/// units field. We can also use fields with units in complex key multi-field 
/// definitions by using the method(s) TRestMetadata::GetDblFieldValueWithUnits.
///
/// The physical field values read in this way will be converted to the standard unit 
/// system used by REST. REST_Units namespace provides details on the different existing 
/// units, unit conversion and unit definition. 
///
/// When we retrieve any value from a REST member in a TRestMetadata class the value 
/// will be returned in the default REST units (mm, keV, V/cm, us). We may convert
/// this member to the desired units (as defined in REST_Units), by doing:
///
/// \code
/// Double_t valueInMeV = value * REST_Units::MeV;
/// \endcode
///
/// ### Mathematical expression evaluation
///
/// Any field value found inside the RML will be previously evaluated by the ROOT
/// class TFormula. In case the field is a valid regular expression, the mathematical
/// formula found in the field value will be substituted by the value returned by
/// the TFormula::Eval method.
/// 
/// The evaluation of complex mathematical expression is done after the replacement
/// of environment variables in the configuration buffer. Therefore, the use of 
/// environment variables inside the field computation is allowed. As for example,
///
/// \code
/// <parameter name="circleArea" value="pi * {RADIUS} * {RADIUS}" />
/// \endcode
///
/// where RADIUS would be an environment variable previously defined. Any 
/// mathematical function allowed by TRestFormula (as sqrt, log) should be allowed 
/// to be used here.
/// 
/// ### Defining internal parameters
///
/// Another option is to define an internal parameter by using the special key definition
/// *myParameter*.
///
/// \code
/// <myParameter name="pixelsPerDetector" value="100" />
/// <myParameter name="detectors" value="5" />
/// \endcode
///
/// The value of a parameter defined this way can be retrieved
/// at any time, by using TRestMetadata::GetMyParameter method.
/// This type of parameter will be pre-processed by the RML interpreter
/// and it can be used later in the same way as environment variables are used.
/// However, in the case of *myParameter* definition, no braces are necessary inside
/// other field definitions, as for example:
///
/// \code
/// <parameter name="totalChannels" value="pixelsPerDetector * detectors" />
/// \endcode
///
/// ### FOR loops definition
///
/// The definition of FOR loops is implemented in RML in order to allow extense
/// definitions, where many elements may need to be added to an existing array in
/// our metadata structure. The use of FOR loops allows to introduce more
/// versatil and extense definitions. Its implementation was fundamentally triggered 
/// by its use in the construction of complex, multi-channel generic readouts by
/// TRestReadout.
/// 
/// The start of the *for* loop definition is as follows
///
/// \code
/// <for variable="n" from="1" to="5" step="1" > 
/// \endcode
///
/// where we define the name of the *variable* that will be iterated ( *variable="n"* ),
/// the initial value of the variable *n* ( *from="1"* ), the final value of *n*
/// ( *to="5"* ), and the step value the variable will be increased in 
/// each iteration ( *step="1"* ). The for loop continues its executing until the end
/// condition defined by the *to="X"* statement is not valid anymore. Therefore, in the
/// previous example the variable *n* will take values 1,2,3,4,5.
///
///
/// All the key structures and parameter definitions found inside the for definition
/// <code> <for ... >  </for> </code> will be replicated until the *for* loop conditions
/// is not valid any more. The *variable name* defined can be used inside the field values 
/// inside the *for* loop definition using square brackets []. Any other named variable
/// (enviromental variable or internal parameter) will be evaluated as usual.
///
/// Nested loops are also possible, as it is shown in the following example
///
/// \code
/// <for variable="nChX" from="1" to="nChannels" step="1" />
///      <for variable="nChY" from="1" to="nChannels" step="1" />
///           <readoutChannel id="([nChX]-1)+nChannels*([nChY]-1)" >
///               <addPixel id="0" origin="(([nChX]-1)*pitch,([nChY]-1)*pitch)" size="(pixelSize,pixelSize)" rotation="0" />
///           </readoutChannel>
///      </for>
///  </for>
/// \endcode
///
/// where *pitch* and *nChannels* are previously defined internal parameters, and *nChX* 
/// and *nChY* are the *for* loop iteration variables.
///
/// ### Comments support
/// 
/// Any not recognized statement written inside a RML file will be just ignored. However, 
/// any information written in the RML will be stored inside the configuration buffer
/// in TRestMetadata::configBuffer, anytime we save to disk a TRestMetadata structure
/// this config buffer is stored and the "original" RML file (after loop expansion, and
/// variable replacement) can be recovered.
///
/// Although we can just write text outside key definition without impact on the read
/// of the RML key definition statements, we can use the XML-style comments to avoid
/// some sentences (or sensitive data) to be stored in the config buffer. Therefore, 
/// any text containned between <code> \<!-- --> </code> will be fully ignored.
///
///--------------------------------------------------------------------------
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2014-june: First concept. As part of conceptualization of previous REST
///            code (REST v2)
///            Igor G. Irastorza
///
/// 2015-jul:  Re-implementation to read .rml files with xml-inspired 
///            syntax
///            Javier Galán
///
/// \class      TRestMetadata
/// \author     Igor G. Irastorza
/// \author     Javier Galan
///
/// <hr>
///

#include <iomanip>

#include <TMath.h>
#include <TSystem.h>
#include "TRestMetadata.h"
using namespace std;
using namespace REST_Units;

int debug = 0;

const int NAME_NOT_FOUND = -2;
const int NOT_FOUND = -1;
const int ERROR = -1;
const int OK = 0;

ClassImp(TRestMetadata)

///////////////////////////////////////////////
/// \brief TRestMetadata default constructor
///
TRestMetadata::TRestMetadata()
{
    fStore = true;

    fGasDataPath = (TString) getenv("REST_PATH") + (TString) "/inputData/gasFiles/";

    fVerboseLevel = REST_Warning;
}

///////////////////////////////////////////////
/// \brief TRestMetadata constructor loading data from a config file
/// 
/// If no configuration path is defined using TRestMetadata::SetConfigFilePath
/// the path to the config file must be specified using full path, absolute or relative.
///
/// By default the config file must be specified with full path, absolute or relative.
///
/// \param cfgFileName A const char* giving the path to an RML file.
///
TRestMetadata::TRestMetadata( const char *cfgFileName)
{
    SetConfigFile( cfgFileName );

    SetTitle("Config");
    SetSectionName("TRestMetadata");

    CheckConfigFile( );

    fStore = true;

    fGasDataPath = (TString) getenv("REST_PATH") + (TString) "/inputData/gasFiles/";

    fVerboseLevel = REST_Warning;
}

///////////////////////////////////////////////
/// \brief TRestMetadata default destructor
///
TRestMetadata::~TRestMetadata()
{
}

///////////////////////////////////////////////
/// \brief Sets the configfile path to be used.
///
/// If the path to configuration file is not specified the location of the configuration file is relative to the directory where we are launching REST.
///
void TRestMetadata::SetConfigFilePath(const char *configFilePath)
{
    fConfigFilePath = string(configFilePath);
}

///////////////////////////////////////////////
/// \brief Returns the input string removing any starting and/or ending white spaces.
///
string TRestMetadata::trim(string str)
{
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');

    if( first == std::string::npos )
        return "";

    return str.substr(first, (last-first+1));
}

///////////////////////////////////////////////
/// \brief Returns 1 only if valid mathematical expression keywords (or numbers) are found in the string **in**. If not it returns 0.
///
Int_t TRestMetadata::isAExpression( string in )
{
    size_t pos = 0;
    string st1 = "sqrt";
    string st2 = "0";
    in = Replace( in, st1, st2, pos, 0 );

    pos = 0;
    st1 = "log";
    st2 = "0";
    in = Replace( in, st1, st2, pos, 0 );

    if ( in == "e-" || in == "e+" ) return 0;

    return (in.find_first_not_of("-0123456789e+*/.,)( ") == std::string::npos && in.length() != 0);
}

///////////////////////////////////////////////
/// \brief Returns 1 only if a valid number is found in the string **in**. If not it returns 0.
///
Int_t TRestMetadata::isANumber( string in )
{
    return (in.find_first_not_of("-+0123456789.e") == std::string::npos && in.length() != 0);
}

///////////////////////////////////////////////
/// \brief Returns the input string without comments. Comments are enclosed between \<!-- \-->.
///
string TRestMetadata::RemoveComments( string in )
{
    string out = in;
    size_t pos = 0;
    while( (pos = out.find("<!--", pos)) != string::npos )
    {
        int length = out.find("-->", pos) - pos;
        out.erase( pos, length+3 ); 
    }
    return out;
}

string TRestMetadata::SectionsToXMLType( string in )
{
    string out = in;
    size_t pos = 0;
    while( (pos = out.find("<section ", pos)) != string::npos )
    {
        // Removing <section keyword
        out.replace( pos, 9, "<" ); 

        size_t pos2 = out.find( " ", pos );

        string sectionName = trim( out.substr( pos+1, pos2-pos ) );

        out = this->Replace( out, (string) "</section>", (string) ("</" + (TString) sectionName + ">"), pos2, 1);
    }
    return out;
}

///////////////////////////////////////////////
/// \brief Returns the input string removing white spaces.
///
string TRestMetadata::RemoveWhiteSpaces( string in )
{
    string out = in;
    size_t pos = 0;
    while( (pos = out.find(" ", pos)) != string::npos )
    {
        out.erase( pos, 1 ); 
        pos = pos + 1;
    }

    return out;
}

///////////////////////////////////////////////
/// \brief Counts the number of occurences of **substring** inside the input string **in**. 
///
Int_t TRestMetadata::Count( string in, string substring )
{
    int count = 0;
    size_t nPos = in.find(substring, 0); // First occurrence
    while( nPos != string::npos )
    {
        count++;
        nPos = in.find( substring, nPos+1);
    }

    return count;
}

///////////////////////////////////////////////
/// \brief Replace the first **N** occurences of **thisSring** by **byThisString** inside string **in**. 
/// If **N=0** (which is the default) all occurences will be replaced.
///
string TRestMetadata::Replace( string in, string thisString, string byThisString, size_t fromPosition, Int_t N )
{
    string out = in;
    size_t pos = fromPosition;
    Int_t cont = 0;
    while( (pos = out.find( thisString , pos)) != string::npos )
    {
        if( debug ) cout << "replacing (" << thisString << ") by (" << byThisString << ")" << endl;
        out.replace( pos, thisString.length(), byThisString ); 
        pos = pos + 1;
        cont++;

        if( N > 0 && cont == N ) return out;
    }

    return out;
}

///////////////////////////////////////////////
/// \brief Gets a double from a string.
///
Double_t TRestMetadata::StringToDouble( string in )
{
    if( isANumber ( in ) )
    {
        return stod ( in );
    }
    else
    {
        return -1;
    }
}

/**/	//AJOUT TRIANGLE
///////////////////////////////////////////////
/// \brief Gets a boolean from a string.
///
Bool_t TRestMetadata::StringToBool( string in )
{
    return ( in == "true" || in == "True" || in == "TRUE" );
}
//*/

///////////////////////////////////////////////
/// \brief Gets an integer from a string.
///
Int_t TRestMetadata::StringToInteger( string in )
{
    return (Int_t) StringToDouble( in );
}


///////////////////////////////////////////////
/// \brief Gets a 3D-vector from a string. Format should be : (X,Y,Z).
///
/// TODO : Implement a warning when the format is not correct.
///
TVector3 TRestMetadata::StringTo3DVector( string in )
{
    TVector3 a;

    size_t startVector = in.find_first_of("(");
    if( startVector == string::npos ) return a;

    size_t endVector = in.find_first_of(")");
    if( endVector == string::npos ) return a;

    size_t n = count(in.begin(), in.end(), ',');
    if( n != 2 ) return a;

    size_t firstComma = in.find_first_of(",");
    size_t secondComma = in.find(",", firstComma+1);

    if( firstComma >= endVector || firstComma <= startVector ) return a;
    if( secondComma >= endVector || secondComma <= startVector ) return a;

    string X = in.substr( startVector+1, firstComma-startVector-1 );
    string Y = in.substr( firstComma+1, secondComma-firstComma-1 );
    string Z = in.substr( secondComma+1, endVector-secondComma-1 );

    X = EvaluateExpression( X );
    Y = EvaluateExpression( Y );
    Z = EvaluateExpression( Z );

    a.SetXYZ( StringToDouble(X), StringToDouble(Y), StringToDouble(Z) );

    return a;
}


///////////////////////////////////////////////
/// \brief Gets a 2D-vector from a string.
///
/// TODO : Implement a warning when the format is not correct.
///
TVector2 TRestMetadata::StringTo2DVector( string in )
{
    TVector2 a(-1,-1);

    size_t startVector = in.find_first_of("(");
    if( startVector == string::npos ) return a;

    size_t endVector = in.find_first_of(")");
    if( endVector == string::npos ) return a;

    size_t n = count(in.begin(), in.end(), ',');
    if( n != 1 ) return a;

    size_t firstComma = in.find_first_of(",");

    if( firstComma >= endVector || firstComma <= startVector ) return a;

    string X = in.substr( startVector+1, firstComma-startVector-1 );
    string Y = in.substr( firstComma+1, endVector-firstComma-1 );

    X = EvaluateExpression( X );
    Y = EvaluateExpression( Y );

    a.Set( StringToDouble(X), StringToDouble(Y)  );

    return a;
}


///////////////////////////////////////////////
/// \brief Returns true if the filename exists.
///
bool TRestMetadata::fileExists(const std::string& filename)
{
    struct stat buf;
    if (stat(filename.c_str(), &buf) != -1)
    {
        return true;
    }
    return false;
}

///////////////////////////////////////////////
/// \brief Returns true if the **filename** has *.root* extension.
///
bool TRestMetadata::isRootFile( const std::string& filename )
{
    if ( filename.find( ".root" ) == string::npos ) return false; 

    return true;
}

///////////////////////////////////////////////
/// \brief Returns true if the **path** given by argument is writable 
///
bool TRestMetadata::isPathWritable( const std::string& path )
{
    int result = access(path.c_str(), W_OK);
    if (result == 0) return true;
    else return false;
}

///////////////////////////////////////////////
/// \brief Returns the position of the **nth** occurence of the string **strToFind** inside the string **in**.
///
size_t TRestMetadata::FindNthStringPosition( const string& in, size_t pos, const string& strToFind, size_t nth)
{
    size_t found_pos = in.find( strToFind, pos);
    if( nth == 0 || string::npos == found_pos ) return found_pos;
    return FindNthStringPosition( in, found_pos+1, strToFind, nth-1);
}

///////////////////////////////////////////////
/// \brief Sets the default configuration path. The default is empty. 
///
/// If the default configuration path is set (by calling this method) the config file must 
/// be provided with full path, or with path relative to the directory where REST is being executed.
///
/// This is the default behaviour in REST.
/// 
void TRestMetadata::SetDefaultConfigFilePath( )
{
    SetConfigFilePath( "" );
}

///////////////////////////////////////////////
/// \brief Assigns a config filename to the metadata object. When this is done the default empty path is defined.
///
void TRestMetadata::SetConfigFile( string cfgFileName )
{
    fConfigFileName = cfgFileName;
    SetDefaultConfigFilePath( );
}

///////////////////////////////////////////////
/// \brief Loads a **section** with a given **name** from a **cfgFileName** into the buffer.
///
/// This method loads a **section** with a given **name** into the buffer. 
/// Only the section corresponding to the specific metadata (which defines the **section** keyword) is assigned to the TRestMetadata::configBuffer.
/// If the name is not provided the first section will be extracted.
///
/// \param section The name that defines the section type (i.e. restG4, run, etc). This name is implemented in the derived class.
/// \param cfgFileName The name of the config file from where the section will be extracted.
/// \param name The specific name of the section to be loaded. Given by the user at the RML file.
///
Int_t TRestMetadata::LoadSectionMetadata( string section, string cfgFileName, string name )
{
    fSectionName = section;

    SetConfigFile( cfgFileName );
    string fileName = fConfigFileName;

    ifstream file(fileName);

    // We load all the config file in a temporal buffer
    string temporalBuffer;
    string line;
    while(getline(file, line))
        temporalBuffer += line;

    temporalBuffer = RemoveComments( temporalBuffer );
    temporalBuffer = SectionsToXMLType( temporalBuffer );

    // We temporally associate the environment to the configBuffer
    // We define environment variables that have validity only during execution
    size_t p = 0;
    configBuffer = GetKEYStructure( "environment", p, temporalBuffer );
         
    if( configBuffer != "" )
    {
        configBuffer = ReplaceIncludeDefinitions( configBuffer );
        p = 0;
        while( p != string::npos ) SetEnvVariable( p );
    }

    temporalBuffer = ReplaceEnvironmentalVariables( temporalBuffer );

    // We temporally associate the globals to the configBuffer
    size_t pos = 0;
    configBuffer = GetKEYStructure( "globals", pos, temporalBuffer );
    if( configBuffer != "" )
    {

        configBuffer = ReplaceIncludeDefinitions( configBuffer );
        // We extract the values from globals. 
        // Globals will not be stored but they will be used by the REST framework during execution

        pos = 0;
        fDataPath = GetParameter( "mainDataPath", pos, configBuffer );
        pos = 0;
        fGasDataPath = GetParameter( "gasDataPath", pos, configBuffer );
        pos = 0;
        string vLevelString  = GetParameter( "verboseLevel", pos, configBuffer );

        if( vLevelString == "silent" )
        {
            fVerboseLevel = REST_Silent;
            cout << "Setting verbose level to silent : " << fVerboseLevel << endl;
        }
        else if ( vLevelString == "warning" )
        {
            fVerboseLevel = REST_Warning;
            cout << "Setting verbose level to warning : " << fVerboseLevel <<  endl;

        }
        else if ( vLevelString == "info" )
        {
            fVerboseLevel = REST_Info;
            cout << "Setting verbose level to info : " << fVerboseLevel << endl;

        }
        else if ( vLevelString == "debug" )
        {
            fVerboseLevel = REST_Debug;
            cout << "Setting verbose level to debug : " << fVerboseLevel << endl;
        }
        else if ( vLevelString == "extreme" )
        {
            fVerboseLevel = REST_Extreme;
            cout << "Setting verbose level to extreme : " << fVerboseLevel << endl;
        }
    }

    // We just extract the corresponding section name as defined in the derived class (fSectionName)
    pos = 0;
    while( ( configBuffer = GetKEYStructure( fSectionName, pos, temporalBuffer ) ) != "" )
    {
        this->SetName( (TString) GetFieldValue( "name", configBuffer ) );
        this->SetTitle( (TString) GetFieldValue( "title", configBuffer ) );

        if( (TString) this->GetName() == "Not defined" ) 
        {
            string sectionDefinition = GetKEYDefinition( fSectionName, configBuffer );

            string nameref = GetFieldValue( "nameref", sectionDefinition );
            string fileref = GetFieldValue( "file", sectionDefinition );

            if( nameref != "Not defined" && fileref != "Not defined" )
            {
                configBuffer = GetSectionByNameFromFile( nameref, fileref );
                if( configBuffer == "" ) 
                { 
                    cout << "REST error : Could not find section " << fSectionName <<
                        " with name : " << nameref <<
                        " inside " << ReplaceEnvironmentalVariables( fileref ) << endl; 
                    exit(1); 
                    return -1; 
                }
            }
        }

        if( this->GetName() == name || name == "" ) break;
    }

    if( configBuffer == "" )
    {
        cout << "REST ERROR : Section " << fSectionName << " with name : " << name << " not found" << endl;
        exit(1);
    }

    configBuffer = ReplaceIncludeDefinitions( configBuffer );

    string sectionDefinition = GetKEYDefinition( fSectionName, configBuffer );

    string debugStr = GetFieldValue( "verboseLevel", sectionDefinition );
    if ( debugStr == "silent" )
       fVerboseLevel = REST_Silent;
    if ( debugStr == "info" )
       fVerboseLevel = REST_Info;
    if ( debugStr == "warning" )
       fVerboseLevel = REST_Warning;
    if ( debugStr == "debug" )
       fVerboseLevel = REST_Debug;
    if ( debugStr == "extreme" )
       fVerboseLevel = REST_Extreme;

    if( configBuffer == "" )
    {
        cout << "REST error reading section : " << section << " ( " << GetName() << " )." << endl;
        cout << "Config buffer is EMPTY" << endl;
        exit(1);
        return -1;
    }

    configBuffer = ReplaceEnvironmentalVariables( configBuffer );

    size_t position = 0;
    string value, myParam;
    while( position != string::npos )
    {
        myParam = GetMyParameter( value, position );

        // We replace only the pure parameters that we find between quotes (not inside mathematical expressions)
        /*
        myParam  = "\"" + myParam + "\"";
        value = "\"" + value + "\"";
        */
        if( myParam != "\"\"" )
        {
            if( debug ) cout << myParam << " = " << value << endl;
            configBuffer = Replace( configBuffer, myParam, value, position, 0 );
        }
    }

    configBuffer = ReplaceMathematicalExpressions( configBuffer );

    while( Count ( configBuffer, "<for" ) > 0 )
        configBuffer = ExpandForLoops( configBuffer );

    configBuffer = ReplaceMathematicalExpressions( configBuffer );

    position = 0;
    while( position != string::npos )
    {
        myParam = GetMyParameter( value, position );
        if( myParam != "\"\"" )
        {
            if( debug ) cout << myParam << " = " << value << endl;
            configBuffer = Replace( configBuffer, myParam, value, position, 0 );
        }
    }

    /*
    cout << "===================================" << endl;
    cout << configBuffer << endl;
    cout << "===================================" << endl;
    getchar();
    */

    if( debug > 0 )
    {
        cout << "=====config buffer===(before mathematical replacement)======" << endl;
        cout << configBuffer << endl;
        cout << endl << "======================" << endl;
        getchar();
    }

    configBuffer = ReplaceMathematicalExpressions( configBuffer );

    /*
    cout << "===================================" << endl;
    cout << configBuffer << endl;
    cout << "===================================" << endl;
    getchar();
    */

    if( debug > 0 )
    {
        cout << "=====config buffer=================" << endl;
        cout << configBuffer << endl;
        cout << endl << "======================" << endl;
        getchar();
    }

    if( file ) file.close();
    return 0;
}

///////////////////////////////////////////////
/// \brief Loads the corresponding metadata section named by the user as **name**, and found inside the file **cfgFileName** into TRestMetadata::configBuffer.
///
/// In case of sucess TRestMetadata::InitFromConfigFile( ) is invoked, 
/// and the specific members of the specific metadata structure are initialized using the values found in the RML file.
///
/// \param cfgFileName The RML filename where the section can be found with the given **name**.
/// \param name The user defined name of the section.
///
Int_t TRestMetadata::LoadConfigFromFile( string cfgFileName, string name )
{
    std::string section = GetSectionName();

    Int_t result = LoadSectionMetadata( section, cfgFileName, name );
    if( result == 0 ) InitFromConfigFile();
    return result;
}

///////////////////////////////////////////////
/// \brief Loads the corresponding metadata section found inside the file **cfgFileName** into TRestMetadata::configBuffer.
///
/// In case of sucess TRestMetadata::InitFromConfigFile( ) is invoked, 
/// and the specific members of the specific metadata structure are initialized using the values found in the RML file.
///
Int_t TRestMetadata::LoadConfigFromFile( string cfgFileName )
{
    std::string section = GetSectionName();

    Int_t result = LoadSectionMetadata( section, cfgFileName );
    if( result == 0 ) InitFromConfigFile();
    return result;
}

///////////////////////////////////////////////
/// \brief Extracts the inner FOR structure in nested FOR loops.
///
string TRestMetadata::ExtractLoopStructure( string in, size_t pos )
{
    // This might be an improved version of GetKEYStructure()
    string startKey = "<for";
    string endKey = "/for";

    string output = "";

    int forDepth = 0;
    size_t startPos = in.find( startKey );
    if( pos == string::npos ) return output;

    pos = startPos + 4;
    forDepth++;

    while( pos != string::npos && forDepth > 0 )
    {
        size_t nextForStart = in.find( startKey, pos );
        size_t nextForEnd = in.find( endKey, pos );
        if( nextForEnd == string::npos ) break;

        if( nextForStart < nextForEnd ) 
        {
            forDepth++;
            pos = nextForStart+4;
        }
        else
        {
            forDepth--;
            pos = nextForEnd+4;
        }
    }

    return in.substr( startPos, pos-startPos+1 );

}

///////////////////////////////////////////////
/// \brief Expands the loop structures found in **buffer** by substituting the running indexes by their values.
///
string TRestMetadata::ExpandForLoops( const string buffer )
{
    string outputBuffer = buffer;

    // Searching the most internal for
    if( debug > 2 ) 
    {
        cout << "------input for ExtractLoopStructure-------" << endl;
        cout << outputBuffer << endl;
    }
    size_t pos = 0;
    string forLoop = ExtractLoopStructure( outputBuffer, pos );

    if( debug > 0 )
    {
        cout << " For loop to expand " << endl;
        cout << " ----------- " << endl;
        cout << forLoop << endl;
        cout << " ----------- " << endl;
        getchar();
    }

    // We replace the place loop content in output buffer by a TAG for later replacement
    outputBuffer = Replace( outputBuffer, forLoop, "PLACE FOR LOOP EXPANSION" );

    if( debug > 0 )
    {
        cout << "Input buffer modified" << endl;
        cout << " ----------- " << endl;
        cout << outputBuffer << endl;
        cout << " ----------- " << endl;
        getchar();
    }


    // We obtain the for loop parameters
    string forDefinition = GetKEYDefinition( "for", forLoop );

    forDefinition = ReplaceMathematicalExpressions( forDefinition );

    string variable = GetFieldValue( "variable", forDefinition );
    string varStr = "[" + variable + "]";
    Double_t from = StringToDouble( GetFieldValue( "from", forDefinition ) );
    Double_t to = StringToDouble ( GetFieldValue( "to", forDefinition ) );
    Double_t step = StringToDouble ( GetFieldValue( "step", forDefinition ) );

    // We obtain the for loop content to be repeated
    string forContent = forLoop.substr( forDefinition.length(), forLoop.find_last_of( "</for" ) - forDefinition.length()-4 );
    forContent = forContent.substr( forContent.find("<"), forContent.length()-forContent.find("<") );


    if( debug > 1 )
    {
        cout << " For content " << endl;
        cout << " ----------- " << endl;
        cout << forContent << endl;
    }

    // We replace the variable by its loop value and to forReplacement
    string forReplacement = "";
    for( double n = from; n <= to; n = n + step )
    {
        ostringstream ss;
        ss << n;
        string nStr = ss.str();

        forReplacement += Replace( forContent, varStr, nStr );
    }

    if( debug > 1 )
    {
        cout << " For replacement " << endl;
        cout << "+++++++++++++++++" << endl;
        cout << forReplacement << endl;
        cout << "+++++++++++++++++" << endl;
    }

    // We replace the resulting repeated sentences in the previously defined TAG 
    outputBuffer = Replace( outputBuffer, "PLACE FOR LOOP EXPANSION", forReplacement );

    if( debug > 1 )
    {
        cout << " Final result " << endl;
        cout << "+++++++++++++++++" << endl;
        cout << outputBuffer << endl;
        cout << "+++++++++++++++++" << endl;
    }

    return outputBuffer;
}

///////////////////////////////////////////////
/// \brief Evaluates a complex numerical expression and returns the resulting value using TFormula.
///
string TRestMetadata::EvaluateExpression( string exp )
{
    if( !isAExpression( exp ) ) { return exp; }

    TFormula formula("tmp", exp.c_str());

    ostringstream sss;
    Double_t number = formula.EvalPar(0);
    if( number > 0 && number < 1.e-300 ) 
        { cout << "REST Warning! Expression not recognized --> " << exp << endl;  return exp; }

    sss << number;
    string out = sss.str();

    return out;
}

///////////////////////////////////////////////
/// \brief Identifies include definitions inside the RML, and replaces it by the content in the referenced file
///
/// RML definition : <include file="includeFile.xml" />
///
string TRestMetadata::ReplaceIncludeDefinitions( const string buffer )
{
    string outputBuffer = buffer;

    size_t pos = 0;
    string includeString;
    do
    {
        includeString = GetKEYDefinition( "include", pos, outputBuffer );

        if( includeString.length() == 0 ) break;

        if( includeString.length() > 0 )
            includeString += ">";

        string fileName = GetFieldValue( "file", includeString );
        fileName = ReplaceEnvironmentalVariables( fileName );

        if( fileName != "Not defined" )
        {
            if( !fileExists( fileName ) )
            {
                cout << "REST WARNING. TRestMetadata::ReplaceIncludeDefinitions." << endl;
                cout << "File : " << fileName << " not found!" << endl;
            }
            else
            {
                string temporalBuffer;
                string line;
                ifstream file(fileName);
                while(getline(file, line)) temporalBuffer += line;

                string outputNow;
                size_t pos2 = 0;
                outputNow = Replace( outputBuffer, includeString, temporalBuffer, pos2, 0 ); 
                outputBuffer = outputNow;
            }
        }

    }
    while( includeString.length() > 0 );

    return outputBuffer;
}

///////////////////////////////////////////////
/// \brief Identifies enviromental variable definitions inside the RML and substitutes them by their value.
///
/// Enviromental variables inside RML can be used by placing the variable name between brackets with the following nomenclature ${VARIABLE_NAME}
///
string TRestMetadata::ReplaceEnvironmentalVariables( const string buffer )
{
    string outputBuffer = buffer;

    int startPosition = 0;
    int endPosition = 0;

    while ( ( startPosition = outputBuffer.find( "${", endPosition ) ) != (int) string::npos )
    {
        char envValue[256];
        endPosition = outputBuffer.find( "}", startPosition+1 );
        if( endPosition == (int) string::npos ) break;

        string expression = outputBuffer.substr( startPosition+2, endPosition-startPosition-2 );

        if( getenv( expression.c_str() ) )
        {
            sprintf( envValue, "%s", getenv( expression.c_str() ) );

            outputBuffer.replace( startPosition, endPosition-startPosition+1,  envValue );

            endPosition -= ( endPosition - startPosition + 1 );
        }
        else
        {
            sprintf( envValue, " " );
            cout << "REST ERROR :: In config file " << fConfigFilePath << fConfigFileName << endl;
            cout << "Environmental variable " << expression << " is not defined" << endl; 
            exit(1);
        }
    }

    startPosition = 0;
    endPosition = 0;

    while ( ( startPosition = outputBuffer.find( "{", endPosition ) ) != (int) string::npos )
    {
        endPosition = outputBuffer.find( "}", startPosition+1 );
        if( endPosition == (int) string::npos ) break;

        string expression = outputBuffer.substr( startPosition+1, endPosition-startPosition-1 );

        cout << "------------------------------------------------------------------------------" << endl;
        cout << "REST Warning!!" << " Section name : " << fSectionName << endl;
        cout << "Environment variables should be defined now using the following format ${VAR}" << endl;
        cout << "Please, if the definition {" << expression << "} inside the RML, is an environment" << endl;
        cout << "variable, replace it by ${" << expression << "}" << endl;
        cout << "------------------------------------------------------------------------------" << endl;

        if( GetVerboseLevel() >= REST_Extreme )
        {
            cout << "To avoid this issue requesting a key stroke you must define the verboseLevel below extreme." << endl;
            GetChar();
        }
    }

    return outputBuffer;
}

///////////////////////////////////////////////
/// \brief Evaluates and replaces valid mathematical expressions found in the input string **buffer**.
///
string TRestMetadata::ReplaceMathematicalExpressions( const string buffer )
{
    string outputBuffer = buffer;


    int startPosition = 0;
    int endPosition = 0;

    // searching any fields within quotes ""
    while ( ( startPosition = outputBuffer.find( "\"", endPosition ) ) != (int) string::npos )
    {
        endPosition = outputBuffer.find( "\"", startPosition+1 );
        if( endPosition == (int) string::npos ) break;

        string expression = outputBuffer.substr( startPosition+1, endPosition-startPosition-1 );
        endPosition++;

        string replacement = "";

        if( expression[0] == '(' && expression[expression.length()-1] == ')' && expression.find(",") != string::npos )
        {
            replacement += "(";
            string firstComponent = expression.substr( 1, expression.find(",")-1 );

            replacement += EvaluateExpression( firstComponent );
            replacement += ",";

            if ( Count( expression, "," ) == 2 )
            {
                string secondComponent = expression.substr( expression.find(",")+1, expression.find_last_of(",")-expression.find(",")-1 );
                replacement += EvaluateExpression( secondComponent );
                replacement += ",";

            }

            string lastComponent = expression.substr( expression.find_last_of(",")+1, expression.find_last_of(")")-expression.find_last_of(",")-1 );
            replacement += EvaluateExpression( lastComponent );
            replacement += ")";
        }
        else
        {
            replacement += EvaluateExpression( expression );
        }

        outputBuffer.replace( startPosition+1, endPosition-startPosition-2, replacement );


        if( debug )
        {
        cout << "Expression : " << expression << " replacement : " << replacement << endl;
        cout << "-----------------" << endl;
        cout << "Remainning buffer" << endl;
        cout << "-----------------" << endl;
        }
        endPosition = endPosition + replacement.length() - expression.length();
        if( debug ) 
            cout << outputBuffer.substr( endPosition ) << endl;

    }

    return outputBuffer;
}

///////////////////////////////////////////////
/// \brief Checks if the config file can be openned. It returns OK in case of success, ERROR otherwise.
///
Int_t TRestMetadata::CheckConfigFile( )
{
    string fileName = fConfigFilePath + fConfigFileName;

    ifstream ifs;
    ifs.open ( fileName, std::ifstream::in);

    if( !ifs ) 
    {
        cout << "Config filename : " << fileName << endl;
        cout << "REST WARNING. TRestMetadata. Config file could not be opened. Right path/filename?" << endl; 
        exit(1);
    }
    else ifs.close();

    return OK;
}

/////////////////////////////////////////////////////////////
/// \brief Method not implemented!!
///
/// TODO : Check if the section defined in TRestSpecificMetadata is in the config file
///
void TRestMetadata::CheckSection( )
{
}

///////////////////////////////////////////////
/// \brief Gets the position for the first occurence of the keyword </section> inside **TRestMetadata::configBuffer** starting from **initPos**.
///
Int_t TRestMetadata::FindEndSection( Int_t initPos )
{
    Int_t endSectionPos = configBuffer.find("</section>", initPos );

    if( (size_t) endSectionPos == string::npos ) return NOT_FOUND;
    else return endSectionPos;
}

///////////////////////////////////////////////
/// \brief Finds next *myParameter* definition found in **TRestMetadata::configBuffer** starting from **pos**.
///
/// This special parameter is intended to be used for personal and very particular cases.
///
/// \param value The value found inside myParameter definition is returned here.
/// \param pos The position where we start to search inside TRestMetadata::configBuffer.
/// \return It returns the name of the defined parameter. In case no myParameter definition is found an empty string is returned.
/// 
string TRestMetadata::GetMyParameter( string &value, size_t &pos )
{
    string parameterString = GetKEYDefinition( "myParameter", pos );

    if( parameterString.find( "name" ) != string::npos && parameterString.find( "value" ) != string::npos )
    {
        value = GetFieldValue( "value", parameterString );
        return GetFieldValue( "name", parameterString );
    }

    return "";
}

///////////////////////////////////////////////
/// \brief Returns the value inside *myParameter* definition matching the name *parname*.
///
/// \param parname The name of the parameter inside myParameter definition.
/// \return It returns the value found in the myParameter definition with name *parname*.
///
string TRestMetadata::GetMyParameterValue( string parname  )
{
    size_t pos = 0;

    string value;
    do
    {
        if( GetMyParameter( value, pos ) == parname )
            return value;
    }
    while( pos != string::npos );

        /*
    if( parameterString.find( "name" ) != string::npos && parameterString.find( "value" ) != string::npos )
    {
        value = GetFieldValue( "value", parameterString );
        return GetFieldValue( "name", parameterString );
    }
    */

    return "";
}

///////////////////////////////////////////////
/// \brief Finds an environment variable definition inside the buffer and sets it.
///
/// The environment variables defined inside the buffer have validity in the context of a REST program execution.
/// After the execution of a REST program the environment variables defined this way have not impact on the 
/// system (as it is imposed by UNIX shell).
/// 
/// In any case, if the environment variable exists already, its value can be overriden here. In this case we define 
/// *overwrite="true"*
/// 
/// Example of environmental variable definition : \code <variable name="TEST" value="VALUE" overwrite="true" > \endcode
///
void TRestMetadata::SetEnvVariable( size_t &pos )
{
    string envString = GetKEYDefinition( "variable", pos );

    if( envString.find( "name" ) != string::npos && envString.find( "value" ) != string::npos )
    {
        string oWrite = GetFieldValue( "overwrite", envString );
        if( oWrite == "Not defined" )
            oWrite = "false";

        Int_t oWriteInt = 0;

        if( oWrite == "true" ) oWriteInt = 1;

        setenv( GetFieldValue( "name", envString).c_str() , ReplaceEnvironmentalVariables( GetFieldValue( "value", envString ) ).c_str(), oWriteInt );
    }
}

///////////////////////////////////////////////
/// \brief Returns the value for the parameter name **parName** found in **inputString**. 
/// 
/// The methods starts searching in **inputString** after a given position **pos**.
///
string TRestMetadata::GetParameter( string parName, size_t &pos, string inputString )
{
    // TODO : this can be probably removed since now we store only the section on configBuffer
    // TODO : It can be useful a GetParameter( string parName, string sectionBuffer )

    /* TODO
     *
     *  Implement method FindAnySection 
     *  if AnySection position is less than EndSection >> Then </section> has been forgotten.
     *  Make WARNING
     *
     * */

    /*
     *  TODO To impose in this code that parameter must be preceded by parameter KEY word
     *
     *  This will not be a problem if the parameter name is not found anywhere else in the section.
     *  But if the parameter name is written somewhere else it may cause problems.
     *  We must find first the parameter KEY and then seach the name in the parameter substring.
     *
     * */

    string parameterString;
    do
    {
        parameterString = GetKEYDefinition( "parameter", pos, inputString );

        if( GetFieldValue( "name", parameterString ) == parName )
            return GetFieldValue( "value", parameterString );
    }
    while( parameterString.length() > 0 );

    if( this->GetVerboseLevel() >= REST_Warning )
        cout << "Section " << fSectionName << ". Parameter (" << parName << ") NOT found" << endl;
    return "";
}

///////////////////////////////////////////////
/// \brief Gets the double value of the parameter name **parName**, found in **inputString**, after applying unit conversion.
///
/// The parameter must defined providing the additional field units just behind the parameter value. As in the following example :
///
/// \code <parameter name="electricField" value="1" units="kVm" > \endcode
///
/// \param parName The name of the parameter from which we want to obtain the value.
/// \param pos Defines the position inside **inputString** where to start searching the definition of **parName**.
///
/// \return A double value in the default correspoding REST units (keV, us, mm, Vcm).
///
Double_t TRestMetadata::GetDblParameterWithUnits( std::string parName, size_t &pos, std::string inputString )
{
    while( 1 )
    {
        string parameterString = GetKEYDefinition( "parameter", pos, inputString );

        if( parameterString.find( parName ) != string::npos )
        {
            return GetDblFieldValueWithUnits( "value", parameterString );
        }
        else
        {
            if( this->GetVerboseLevel() >= REST_Warning )
                cout << "Section " << fSectionName << ". Parameter (" << parName << ") NOT found" << endl;
            return PARAMETER_NOT_FOUND_DBL;
        }
    }

    if( this->GetVerboseLevel() >= REST_Warning )
        cout << "Section " << fSectionName << ". Parameter (" << parName << ") NOT found" << endl;
    return PARAMETER_NOT_FOUND_DBL;
}

///////////////////////////////////////////////
/// \brief Returns a 2D vector value of the parameter name **parName**, found in **inputString**, after applying unit conversion.
///
/// The parameter must defined providing the additional field units just behind the parameter value. As in the following example :
///
/// \code <parameter name="position" value="(10,0)" units="mm" > \endcode
///
/// \param parName The name of the parameter from which we want to obtain the value.
/// \param pos Defines the position inside **inputString** where to start searching the definition of **parName**.
///
/// \return A 2D vector value in the default correspoding REST units (keV, us, mm, Vcm).
///
TVector2 TRestMetadata::Get2DVectorParameterWithUnits( std::string parName, size_t &pos, std::string inputString )
{
    while( 1 )
    {
        string parameterString = GetKEYDefinition( "parameter", pos, inputString );

        if( parameterString.find( parName ) != string::npos )
        {
            return Get2DVectorFieldValueWithUnits( "value", parameterString );
        }
        else
        {
            if( this->GetVerboseLevel() >= REST_Warning )
                cout << "Parameter (" << parName << ") NOT found" << endl;
            return TVector2(-1,-1);
        }
    }

    if( this->GetVerboseLevel() >= REST_Warning )
        cout << "Parameter (" << parName << ") NOT found" << endl;

    return TVector2(-1,-1);
}

///////////////////////////////////////////////
/// \brief Returns a 3D vector value of the parameter name **parName**, found in **inputString**, after applying unit conversion.
///
/// The parameter must defined providing the additional field units just behind the parameter value. As in the following example :
///
/// \code <parameter name="position" value="(10,0,-10)" units="mm" > \endcode
///
/// \param parName The name of the parameter from which we want to obtain the value.
/// \param pos Defines the position inside **inputString** where to start searching the definition of **parName**.
///
/// \return A 3D vector value in the default correspoding REST units (keV, us, mm, Vcm).
///
TVector3 TRestMetadata::Get3DVectorParameterWithUnits( std::string parName, size_t &pos, std::string inputString )
{
    while( 1 )
    {
        string parameterString = GetKEYDefinition( "parameter", pos, inputString );

        if( parameterString.find( parName ) != string::npos )
        {
            return Get3DVectorFieldValueWithUnits( "value", parameterString );
        }
        else
        {
            if( this->GetVerboseLevel() >= REST_Warning )
                cout << "Section " << fSectionName << ". Parameter (" << parName << ") NOT found" << endl;
            return TVector3( -1, -1, -1 );
        }
    }

    if( this->GetVerboseLevel() >= REST_Warning )
        cout << "Section " << fSectionName << ". Parameter (" << parName << ") NOT found" << endl;
    return TVector3( -1, -1, -1 );;
}

///////////////////////////////////////////////
/// \brief Returns a string with the value of the parameter name **parName**, found in TRestMetadata::configBuffer.
///
/// The same parameter name should not be used in a given section. Only the first occurence of **parName** is given.
///
/// \param parName The name of the parameter from which we want to obtain the value.
/// \param defaultValue An optional value that will be given in case the parameter is not found.
///
/// \return A string with the value of the parameter **parName**.
///
string TRestMetadata::GetParameter( string parName, TString defaultValue )
{
    // TODO : this can be probably removed since now we store only the section on configBuffer
    // TODO : It can be useful a GetParameter( string parName, string sectionBuffer )
    size_t position = 0;

    /* TODO
     *
     *  Implement method FindAnySection 
     *  if AnySection position is less than EndSection >> Then </section> has been forgotten.
     *  Make WARNING
     *
     * */

    /*
     *  TODO To impose in this code that parameter must be preceded by parameter KEY word
     *
     *  This will not be a problem if the parameter name is not found anywhere else in the section.
     *  But if the parameter name is written somewhere else it may cause problems.
     *  We must find first the parameter KEY and then seach the name in the parameter substring.
     *
     * */

    string parameterString;
    while( position != string::npos )
    {
        parameterString = GetKEYDefinition( "parameter", position );
        if( debug > 1 ) cout << "Parameter string : " << parameterString << endl;

        if( GetFieldValue( "name", parameterString ) == parName )
        {
            string value = GetFieldValue( "value", parameterString );
            if( value == "" ) return defaultValue.Data();
            else return value;
        }
        else
        {
            if( debug > 1 ) cout << " I did not found" << endl;
        }
    }
    debug = 0;

    if( this->GetVerboseLevel() >= REST_Warning )
    {
        cout << "Section " << fSectionName << ". Parameter (" << parName << ") NOT found" << endl;
        cout << "Returning default value (" << defaultValue << ")" << endl;
    }
    return defaultValue.Data();
}

///////////////////////////////////////////////
/// \brief Returns a list of observable names found inside TRestMetadata::configBuffer.
///
/// An observable can be defined as follows inside an RML file
/// \code <observable name="OBS_NAME" value="ON" /> \endcode
/// 
/// The observable will be added to the list only in the case the value of the observable is ON.
///
vector <string> TRestMetadata::GetObservablesList( )
{
    size_t position = 0;

    vector <string> output;
    output.clear();

    string observableString;
    while( position != string::npos )
    {
        observableString = GetKEYDefinition( "observable", position );
        if( debug > 1 ) cout << "Parameter string : " << observableString << endl;

            string value = GetFieldValue( "value", observableString );
            if( value == "ON" || value == "on" )
            {
                string observableName = GetFieldValue( "name", observableString );
                output.push_back( observableName );
            }
    }

    return output;
}

///////////////////////////////////////////////
/// \brief Returns a list of observables description correspoding to the observable list retrieved using TRestMetadata::GetObservablesList.
///
/// Optionally we can add a description to the observable definition as follows inside an RML file
/// \code <observable name="OBS_NAME" value="ON" description="A text description" /> \endcode
///
vector <string> TRestMetadata::GetObservableDescriptionsList( )
{
    size_t position = 0;

    vector <string> output;
    output.clear();

    string observableString;
    while( position != string::npos )
    {
        observableString = GetKEYDefinition( "observable", position );
        if( debug > 1 ) cout << "Parameter string : " << observableString << endl;

            string value = GetFieldValue( "value", observableString );
            if( value == "ON" || value == "on" )
            {
                string observableDescription = GetFieldValue( "description", observableString );
                cout << "Observable description : " << observableDescription << endl;
                output.push_back( observableDescription );
            }
    }

    return output;
}

///////////////////////////////////////////////
/// \brief Gets the double value of the parameter name **parName**, defined inside TRestMetadata::configBuffer, after applying unit conversion.
///
/// The parameter must defined providing the additional field units just behind the parameter value. As in the following example :
///
/// \code <parameter name="electricField" value="1" units="kVm" > \endcode
///
/// \param parName The name of the parameter from which we want to obtain the value.
/// \param defaultValue The value that will be returned in case the parameter is not found.
///
/// \return A double value in the default correspoding REST units (keV, us, mm, Vcm).
///
Double_t TRestMetadata::GetDblParameterWithUnits( string parName, Double_t defaultValue )
{
    size_t position = 0;

    string parameterString;
    while( position != string::npos )
    {
        parameterString = GetKEYDefinition( "parameter", position );
        if( debug > 1 ) cout << "Parameter string : " << parameterString << endl;

        if( parameterString.find( parName ) != string::npos )
        {
            if( GetFieldValue( "value", parameterString ) == "" )
                return defaultValue;

            Double_t value = GetDblFieldValueWithUnits( "value", parameterString );
            if( value == PARAMETER_NOT_FOUND_DBL ) return defaultValue;
            else return value;
        }
        else
        {
            if( debug > 1 ) cout << " I did not found" << endl;
        }
    }

    if( this->GetVerboseLevel() >= REST_Warning )
    {
        cout << "Section " << fSectionName << ". Parameter (" << parName << ") NOT found" << endl;
        cout << "Returning default value (" << defaultValue << ")" << endl;
    }
    return defaultValue;
}

///////////////////////////////////////////////
/// \brief Returns a 2D vector value of the parameter name **parName**, found in TRestMetadata::configBuffer, after applying unit conversion.
///
/// The parameter must defined providing the additional field units just behind the parameter value. As in the following example :
///
/// \code <parameter name="position" value="(10,0)" units="mm" > \endcode
///
/// \param parName The name of the parameter from which we want to obtain the value.
/// \param defaultValue The value that will be returned in case the parameter is not found.
///
/// \return A 2D vector value in the default correspoding REST units (keV, us, mm, Vcm).
///
TVector2 TRestMetadata::Get2DVectorParameterWithUnits( string parName, TVector2 defaultValue )
{
    size_t position = 0;

    string parameterString;
    while( position != string::npos )
    {
        parameterString = GetKEYDefinition( "parameter", position );
        if( debug > 1 ) cout << "Parameter string : " << parameterString << endl;

        if( parameterString.find( parName ) != string::npos )
        {
            if( GetFieldValue( "value", parameterString ) == "" )
                return defaultValue;

            TVector2 value = Get2DVectorFieldValueWithUnits( "value", parameterString );

            if( value.X() == -1 && value.Y() == -1 ) return defaultValue;
            else return value;
        }
        else
        {
            if( debug > 1 ) cout << " I did not found" << endl;
        }
    }

    if( this->GetVerboseLevel() >= REST_Warning )
    {
        cout << "Section " << fSectionName << ". Parameter (" << parName << ") NOT found" << endl;
        cout << "Returning default value (" << defaultValue.X() << " , " << defaultValue.Y() << ")" << endl;
    }
    return defaultValue;
}

///////////////////////////////////////////////
/// \brief Returns a 3D vector value of the parameter name **parName**, found in TRestMetadata::configBuffer, after applying unit conversion.
///
/// The parameter must defined providing the additional field units just behind the parameter value. As in the following example :
///
/// \code <parameter name="position" value="(10,0)" units="mm" > \endcode
///
/// \param parName The name of the parameter from which we want to obtain the value.
/// \param defaultValue The value that will be returned in case the parameter is not found.
///
/// \return A 3D vector value in the default correspoding REST units (keV, us, mm, Vcm).
///
TVector3 TRestMetadata::Get3DVectorParameterWithUnits( string parName, TVector3 defaultValue )
{
    size_t position = 0;

    string parameterString;
    while( position != string::npos )
    {
        parameterString = GetKEYDefinition( "parameter", position );
        if( debug > 1 ) cout << "Parameter string : " << parameterString << endl;

        if( parameterString.find( parName ) != string::npos )
        {
            if( GetFieldValue( "value", parameterString ) == "" )
                return defaultValue;

            TVector3 value = Get3DVectorFieldValueWithUnits( "value", parameterString );

            if( value.X() == -1 && value.Y() == -1 && value.Z() == -1 ) return defaultValue;
            else return value;
        }
        else
        {
            if( debug > 1 ) cout << " I did not found" << endl;
        }
    }

    if( this->GetVerboseLevel() >= REST_Warning )
    {
        cout << "Section " << fSectionName << ". Parameter (" << parName << ") NOT found" << endl;
        cout << "Returning default value (" << defaultValue.X() << " , " << defaultValue.Y() << " , " << defaultValue.Z() << ")" << endl;
    }
    return defaultValue;
}

///////////////////////////////////////////////
/// \brief Returns a string with the value of a field named **parName** found inside a definition tag **key**.
///
/// The basic structure of a **key** definition is as follows:
/// \code <key parName="value" /> \endcode
///
/// \param parName The name of the field from which we want to get the value
/// \param key The tag used in the definition where we want to look for **parName**.
string TRestMetadata::GetFieldFromKEY( string parName, string key )
{
    size_t position = 0;

    string parameterString;
    while( position != string::npos )
    {
        parameterString = GetKEYDefinition( key, position );
        if( debug ) cout << key << " string : " << parameterString << endl;
        if( parameterString.find( parName ) != string::npos )
            return GetFieldValue( parName, parameterString );
    }

    if( this->GetVerboseLevel() >= REST_Warning )
        cout << "Section " << fSectionName << ". Parameter (" << parName << ") NOT found" << endl;

    return "";
}

///////////////////////////////////////////////
/// \brief Returns a string with the value of a field named **fieldName** found inside the string **definition**.
///
/// \param fieldName The name of the field from which we want to get the value
/// \param definition The string that contains the field name and value in the format field="value".
/// \param fromPosition The position inside **definition** from where we start searching for the **fieldName**.
///
string TRestMetadata::GetFieldValue( string fieldName, string definition, size_t fromPosition )
{
    string fldName = fieldName + "=\"";

    size_t pos, pos2;
    pos = definition.find( fldName, fromPosition ); 

    if( (pos = definition.find( fldName, fromPosition )) == string::npos ) {  return "Not defined"; }
    else
    {
        pos = definition.find( "\"", pos );
        pos++;
        pos2 = definition.find( "\"", pos );
        return definition.substr( pos, pos2-pos );
    }
}

///////////////////////////////////////////////
/// \brief Returns a string with the unit name provided inside **definition**.
///
/// The first occurence of units="" is given.
/// 
/// \param definition The string where we search for the units definition.
/// \param fromPosition The position inside the string **definition** where we start looking for the units definition.
///
string TRestMetadata::GetUnits( string definition, size_t fromPosition )
{
    string fldName = "units=\"";

    size_t pos, pos2;
    pos = definition.find( fldName, fromPosition ); 

    if( pos - fromPosition > 8 ) return "Not defined";


    if( (pos = definition.find( fldName, fromPosition )) == string::npos ) {  return "Not defined"; }
    else
    {
        pos = definition.find( "\"", pos );
        pos++;
        pos2 = definition.find( "\"", pos );
        return definition.substr( pos, pos2-pos );
    }
}

///////////////////////////////////////////////
/// \brief Returns a double value of a field named **fieldName** found inside the string **definition**, after applying unit conversion.
///
/// \param fieldName The name of the field from which we want to get the value
/// \param definition The string that contains the field name and value in the format field="value".
/// \param fromPosition The position inside **definition** from where we start searching for the **fieldName**.
///
Double_t TRestMetadata::GetDblFieldValueWithUnits( string fieldName, string definition, size_t fromPosition )
{
    string fldName = fieldName + "=\"";

    size_t pos, pos2;
    pos = definition.find( fldName ); 

    if( (pos = definition.find( fldName, fromPosition )) == string::npos ) {  return PARAMETER_NOT_FOUND_DBL; }
    else
    {
        pos = definition.find( "\"", pos );
        pos++;
        pos2 = definition.find( "\"", pos );

        TString unitsStr = GetUnits( definition, pos2 );

        Double_t value = StringToDouble(  definition.substr( pos, pos2-pos ) );

        value = REST_Units::GetValueInRESTUnits( value, unitsStr );

        if( TMath::IsNaN( value ) )
        {
            cout << "REST ERROR : Check parameter \"" << fieldName << "\" units" << endl;
            cout << "Inside definition : " << definition << endl;
            getchar();
        }


        return value;
    }
}

///////////////////////////////////////////////
/// \brief Returns a 2D vector with the value of a field named **fieldName** found inside the string **definition**, after applying unit conversion.
///
/// \param fieldName The name of the field from which we want to get the value
/// \param definition The string that contains the field name and value in the format fieldName="(value,value)".
/// \param fromPosition The position inside **definition** from where we start searching for the **fieldName**.
///
TVector2 TRestMetadata::Get2DVectorFieldValueWithUnits( string fieldName, string definition, size_t fromPosition )
{
    string fldName = fieldName + "=\"";

    size_t pos, pos2;
    pos = definition.find( fldName ); 

    if( (pos = definition.find( fldName, fromPosition )) == string::npos ) {  return TVector2(-1, -1); }
    else
    {
        pos = definition.find( "\"", pos );
        pos++;
        pos2 = definition.find( "\"", pos );

        TString unitsStr = GetUnits( definition, pos2 );

        TVector2 value = StringTo2DVector( definition.substr( pos, pos2-pos ) );

        Double_t valueX = REST_Units::GetValueInRESTUnits( value.X(), unitsStr );
        Double_t valueY = REST_Units::GetValueInRESTUnits( value.Y(), unitsStr );

        if( TMath::IsNaN( valueX ) || TMath::IsNaN( valueY ) )
        {
            cout << "REST ERROR : Check parameter \"" << fieldName << "\" units" << endl;
            cout << "Inside definition : " << definition << endl;
            getchar();
        }

        return TVector2( valueX, valueY );
    }
}

///////////////////////////////////////////////
/// \brief Returns a 3D vector with the value of a field named **fieldName** found inside the string **definition**, after applying unit conversion.
///
/// \param fieldName The name of the field from which we want to get the value
/// \param definition The string that contains the field name and value in the format fieldName="(value,value,value)".
/// \param fromPosition The position inside **definition** from where we start searching for the **fieldName**.
///
TVector3 TRestMetadata::Get3DVectorFieldValueWithUnits( string fieldName, string definition, size_t fromPosition )
{
    string fldName = fieldName + "=\"";

    size_t pos, pos2;
    pos = definition.find( fldName ); 

    if( (pos = definition.find( fldName, fromPosition )) == string::npos ) 
    {
        return TVector3(-1, -1, -1); 
    }
    else
    {
        pos = definition.find( "\"", pos );
        pos++;
        pos2 = definition.find( "\"", pos );

        TString unitsStr = GetUnits( definition, pos2 );

        TVector3 value = StringTo3DVector( definition.substr( pos, pos2-pos ) );

        Double_t valueX = REST_Units::GetValueInRESTUnits( value.X(), unitsStr );
        Double_t valueY = REST_Units::GetValueInRESTUnits( value.Y(), unitsStr );
        Double_t valueZ = REST_Units::GetValueInRESTUnits( value.Z(), unitsStr );

        if( TMath::IsNaN( valueX ) || TMath::IsNaN( valueY ) || TMath::IsNaN( valueZ ) )
        {
            cout << "REST ERROR : Check parameter \"" << fieldName << "\" units" << endl;
            cout << "Inside definition : " << definition << endl;
            getchar();
        }

        return TVector3( valueX, valueY, valueZ );
    }
}

///////////////////////////////////////////////
/// \brief Returns a string with the value of a field named **fieldName** found inside TRestMetadata::configBuffer.
///
/// \param fieldName The name of the field from which we want to get the value
/// \param fromPosition The position inside TRestMetadata::configBuffer from where we start searching for the **fieldName**.
///
string TRestMetadata::GetFieldValue( string fieldName, size_t fromPosition )
{
    string fldName = fieldName + "=\"";

    size_t pos = 0,pos2;
    size_t endDefinition = configBuffer.find(">", fromPosition);
    string definition = configBuffer.substr( fromPosition, endDefinition-fromPosition);
    pos = definition.find( fldName ); 

    if( (pos = definition.find( fldName )) == string::npos ) {  return "Not defined"; }
    else
    {
        pos = definition.find( "\"", pos );
        pos++;
        pos2 = definition.find( "\"", pos );
        return definition.substr( pos, pos2-pos );
    }
}

///////////////////////////////////////////////
/// \brief Gets the first key definition for **keyName** found inside TRestMetadata::configBuffer
///
/// A key definition is written as follows:
/// \code <keyName field1="value1" field2="value2" > \endcode
///
string TRestMetadata::GetKEYDefinition( string keyName )
{
    Int_t fromPosition = 0;
    string key = "<" + keyName;
    size_t startPos = configBuffer.find( key, fromPosition );
    size_t endPos = configBuffer.find( ">", startPos );

    fromPosition = endPos;

    if( startPos == string::npos ) return "";
    else return configBuffer.substr( startPos, endPos-startPos );

}

///////////////////////////////////////////////
/// \brief Gets the first key definition for **keyName** found inside TRestMetadata::configBuffer starting at **fromPosition**.
///
/// A key definition is written as follows:
/// \code <keyName field1="value1" field2="value2" > \endcode
///
string TRestMetadata::GetKEYDefinition( string keyName, size_t &fromPosition )
{
    string key = "<" + keyName;
    size_t startPos = configBuffer.find( key, fromPosition );
    size_t endPos = configBuffer.find( ">", startPos );

    fromPosition = endPos;

    if( startPos == string::npos ) return "";
    else
    { 
	    Int_t notDefinitionEnd = 1;

	    while( notDefinitionEnd )
	    {
		    // We might find a problem when we insert > symbol inside a field value.
		    // As for example: condition=">100" This patch checks if the definition 
		    // finishes in "= If it is the case it searches the next > symbol ending 
		    // the definition.

		    string def = RemoveWhiteSpaces ( configBuffer.substr( startPos, endPos-startPos ) );

		    if( (TString) def[def.length()-1] == "\"" && (TString) def[def.length()-2] == "=" ) 
			    endPos = configBuffer.find( ">", endPos+1 );
		    else
			    notDefinitionEnd = 0;
	    }

	    return configBuffer.substr( startPos, endPos-startPos );
    }

}

///////////////////////////////////////////////
/// \brief Gets the first key definition for **keyName** found inside **buffer**.
///
/// A key definition is written as follows:
/// \code <keyName field1="value1" field2="value2" > \endcode
///
string TRestMetadata::GetKEYDefinition( string keyName, string buffer )
{
    if( buffer == "" ) return "";

    string key = "<" + keyName;

    size_t startPos = buffer.find( key, 0 );
    size_t endPos = buffer.find( ">", startPos );

    return buffer.substr( startPos, endPos-startPos );

}

///////////////////////////////////////////////
/// \brief Gets the first key definition for **keyName** found inside **buffer** starting at **fromPosition**.
///
/// A key definition is written as follows:
/// \code <keyName field1="value1" field2="value2" > \endcode
///
string TRestMetadata::GetKEYDefinition( string keyName, size_t &fromPosition, string buffer )
{
    string key = "<" + keyName;

    size_t startPos = buffer.find( key, fromPosition );
    if ( startPos == string::npos ) return "";
    size_t endPos = buffer.find( ">", startPos );
    if ( endPos == string::npos ) return "";

    fromPosition = endPos;

    Int_t notDefinitionEnd = 1;

    while( notDefinitionEnd )
    {
        // We might find a problem when we insert > symbol inside a field value.
        // As for example: condition=">100" This patch checks if the definition 
        // finishes in "= If it is the case it searches the next > symbol ending 
        // the definition.

        string def = RemoveWhiteSpaces ( buffer.substr( startPos, endPos-startPos ) );

        if( (TString) def[def.length()-1] == "\"" && (TString) def[def.length()-2] == "=" ) 
            endPos = configBuffer.find( ">", endPos+1 );
        else
            notDefinitionEnd = 0;
    }

    return buffer.substr( startPos, endPos-startPos );

}

///////////////////////////////////////////////
/// \brief Gets the first key structure for **keyName** found inside TRestMetadata::configBuffer.
///
/// A key definition is written as follows:
/// \code <keyName field1="value1" field2="value2" > 
///
///     ....
///
///  </keyName>
/// \endcode
///
string TRestMetadata::GetKEYStructure( string keyName )
{
    string strNotFound = "NotFound";

    size_t position = 0;

    string startKEY = "<" + keyName;
    string endKEY = "/" + keyName;

    size_t initPos = configBuffer.find( startKEY, position );

    if( initPos == string::npos ) { cout << "KEY (" << keyName << ") >> not found!!" << endl; return strNotFound; }

    size_t endPos = configBuffer.find( endKEY, position );

    if( endPos == string::npos ) { cout << "KEY (" << keyName << " << not found!!" << endl; return strNotFound; }

    return configBuffer.substr( initPos, endPos-initPos + endKEY.length()+1 );

}

///////////////////////////////////////////////
/// \brief Gets the first key structure for **keyName** found inside **buffer**.
///
/// A key definition is written as follows:
/// \code <keyName field1="value1" field2="value2" > 
///
///     ....
///
///  </keyName>
/// \endcode
///
string TRestMetadata::GetKEYStructure( string keyName, string buffer )
{

    size_t position = 0;

    string startKEY = "<" + keyName;
    string endKEY = "/" + keyName;

    size_t initPos = buffer.find( startKEY, position );

    if( initPos == string::npos ) { if( debug ) cout << "KEY not found!!" << endl; return "NotFound"; }

    size_t endPos = buffer.find( endKEY, position );

    if( endPos == string::npos ) { if( debug ) cout << "KEY not found!!" << endl; return "NotFound"; }


    return buffer.substr( initPos, endPos-initPos + endKEY.length()+1 );

}

///////////////////////////////////////////////
/// \brief Gets the first key structure for **keyName** found inside TRestMetadata::configBuffer after **fromPosition**.
///
/// A key definition is written as follows:
/// \code <keyName field1="value1" field2="value2" > 
///
///     ....
///
///  </keyName>
/// \endcode
///
string TRestMetadata::GetKEYStructure( string keyName, size_t &fromPosition )
{
    size_t position = fromPosition;

    string startKEY = "<" + keyName;
    string endKEY = "/" + keyName;

    size_t initPos = configBuffer.find( startKEY, position );

    if( initPos == string::npos ) { if( debug ) cout << "KEY not found!!" << endl; return "NotFound"; }

    size_t endPos = configBuffer.find( endKEY, initPos );

    if( endPos == string::npos  ) { if( debug ) cout << "END KEY not found!!" << endl; return "NotFound"; }

    fromPosition = endPos+1;

    return configBuffer.substr( initPos, endPos-initPos + endKEY.length()+1 );
}

///////////////////////////////////////////////
/// \brief Gets the first key structure for **keyName** found inside **buffer** after **fromPosition**.
///
/// A key definition is written as follows:
/// \code <keyName field1="value1" field2="value2" > 
///
///     ....
///
///  </keyName>
/// \endcode
///
string TRestMetadata::GetKEYStructure( string keyName, size_t &fromPosition, string buffer )
{
    size_t position = fromPosition;

    if( debug > 1 ) cout << "Buffer : " << buffer << endl;
    if( debug > 1 ) cout << "Start position : " << position << endl;

    string startKEY = "<" + keyName;
    string endKEY = "/" + keyName;

    if( debug > 1 ) cout << "Reduced buffer : " << buffer.substr( position ) << endl;

    size_t initPos = buffer.find( startKEY, position );
    if( debug > 1 ) cout << "initPos : " << initPos << endl;

    if( initPos == string::npos ) { if( debug > 1 ) cout << "KEY not found!!" << endl; return ""; }

    size_t endPos = buffer.find( endKEY, initPos );

    if( debug > 1 ) cout << "End position : " << endPos << endl;

    //TODO Check if a new section starts. If not it might get two complex strings if the KEY_Structure was not closed using /KEY

    if( endPos == string::npos  ) { if( debug > 1 )  cout << "END KEY not found!!" << endl; return ""; }

    fromPosition = endPos;

    return buffer.substr( initPos, endPos-initPos + endKEY.length()+1 );
}

///////////////////////////////////////////////
/// \brief Gets a string containning the section structure from a file **fref** by using the name **nref** as the user defined section name.
///
/// The section format searched inside **fref** is as follows :
/// \code 
///
/// <section metadataName name="nref">
///
/// \endcode
/// 
string TRestMetadata::GetSectionByNameFromFile( string nref, string fref )
{
    string fileName = ReplaceEnvironmentalVariables( fref );

    ifstream file(fileName);

    if( !file ) { cout << "REST Error : I could not open file : " << fileName << endl; exit(1); return ""; }

    string temporalBuffer;
    string line;
    while(getline(file, line)) temporalBuffer += line;

    size_t position = 0;
    string sectionString;
    while( ( sectionString = GetKEYStructure( fSectionName, position, temporalBuffer ) ) != "" )
        if ( GetFieldValue( "name", sectionString ) == nref ) {  return sectionString; }

    return "";
}

///////////////////////////////////////////////
/// \brief Prints a UNIX timestamp in human readable format
///
void TRestMetadata::PrintTimeStamp( Double_t timeStamp )
{
       cout.precision(10);

       time_t tt = (time_t) timeStamp;
       struct tm *tm = localtime( &tt);

       char date[20];
       strftime(date, sizeof(date), "%Y-%m-%d", tm);
       cout << "Date : " << date << endl;

       char time[20];
       strftime(time, sizeof(time), "%H:%M:%S", tm);
       cout << "Time : " << time << endl;
       cout << "++++++++++++++++++++++++" << endl;
}

///////////////////////////////////////////////
/// \brief Prints TRestMetadata::configBuffer in screen.
///
void TRestMetadata::PrintConfigBuffer( ) { cout << configBuffer << endl; }

///////////////////////////////////////////////
/// \brief Prints metadata content on screen. Usually overloaded by the derived metadata class.
///
void TRestMetadata::PrintMetadata()
{
        cout << "TRestMetadata content" << endl;
        cout << "-----------------------" << endl;
        cout << "Config file : " << fConfigFileName << endl;
        cout << "Section name : " << fSectionName << endl;        // section name given in the constructor of TRestSpecificMetadata
}

TString TRestMetadata::GetVerboseLevelString( )
{
    TString level = "unknown";
    if( this->GetVerboseLevel() == REST_Debug ) level = "debug";
    if( this->GetVerboseLevel() == REST_Info ) level = "info";
    if( this->GetVerboseLevel() == REST_Warning ) level = "warning";
    if( this->GetVerboseLevel() == REST_Silent ) level = "silent";

    return level;
}

