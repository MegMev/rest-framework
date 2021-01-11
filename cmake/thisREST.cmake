# Write thisREST.[c]sh to INSTALL directory

# install thisREST script, sh VERSION
install( CODE
"

file( WRITE \${CMAKE_INSTALL_PREFIX}/thisREST.sh

\"\#!/bin/bash

\# check active shell by checking for existence of _VERSION variable
if [[ -n \\\"\\\${BASH_VERSION}\\\" ]]; then
    thisdir=\\\$(cd \\\$(dirname \\\${BASH_ARGV[0]}); pwd)
elif [[ -n \\\"\\\${ZSH_VERSION}\\\" ]]; then
    thisdir=\\\$(cd \\\$(dirname \\\$0); pwd)
else
    echo \\\"Invalid shell! Either source with bash or zsh!\\\"
    return 1
fi

if [ \\\$REST_PATH ] ; then
echo switching to REST installed in \\\${thisdir}
_PATH=`echo \\\$PATH | sed -e \\\"s\#\\\${REST_PATH}/bin:\#\#g\\\"`
_LD_LIBRARY_PATH=`echo \\\$LD_LIBRARY_PATH | sed -e \\\"s\#\\\${REST_PATH}/lib:\#\#g\\\"`
else
_PATH=\\\$PATH
_LD_LIBRARY_PATH=\\\$LD_LIBRARY_PATH
fi

export REST_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}
export REST_PATH=\\\${thisdir}
export ROOT_INCLUDE_PATH=\\\${thisdir}/include
export REST_INPUTDATA=\\\$REST_PATH/data
export REST_GARFIELD_INCLUDE=${Garfield_INCLUDE_DIRS}
export REST_GARFIELD_LIB=${Garfield_LIBRARIES}
export PATH=\\\$REST_PATH/bin:\\\$_PATH
export LD_LIBRARY_PATH=\\\$REST_PATH/lib:\\\$_LD_LIBRARY_PATH
export LIBRARY_PATH=\\\$LIBRARY_PATH:\\\$REST_PATH/lib

\#alias restRoot=\\\"root -l \\\$REST_PATH/scripts/LoadRESTScripts.C\\\"

if [ \\\$(rest-config --flags | grep \\\"REST_WELCOME=ON\\\") ];then
rest-config --welcome
fi
\"
)
        "
)


foreach(mac ${rest_macros})

string(REPLACE " " "" mac ${mac})
string(REPLACE "rest" "" m ${mac})

install( CODE
"
file( APPEND \${CMAKE_INSTALL_PREFIX}/thisREST.sh 

\"
alias ${mac}=\\\"restManager ${m}\\\"
\"
)
        "
)

endforeach(mac ${rest_macros})

# install thisREST script, csh VERSION
install( CODE
"
file( WRITE \${CMAKE_INSTALL_PREFIX}/thisREST.csh 

\"\#!/bin/csh

setenv REST_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}
setenv REST_PATH ${CMAKE_INSTALL_PREFIX}
setenv REST_INPUTDATA \\\$REST_PATH/data
setenv ROOT_INCLUDE_DIR \\\$REST_PATH/include
if ( \\\$?ROOT_INCLUDE_PATH == 0 ) setenv ROOT_INCLUDE_PATH
setenv ROOT_INCLUDE_PATH \\\${ROOT_INCLUDE_PATH}:\\\$REST_PATH/include
setenv REST_GARFIELD_INCLUDE ${Garfield_INCLUDE_DIRS}
setenv REST_GARFIELD_LIB ${Garfield_LIBRARIES}
setenv PATH \\\$REST_PATH/bin:\\\$PATH
setenv LD_LIBRARY_PATH \\\$REST_PATH/lib:\\\$LD_LIBRARY_PATH
if ( \\\$?LIBRARY_PATH == 0 ) setenv LIBRARY_PATH
setenv LIBRARY_PATH \\\${LIBRARY_PATH}:\\\$REST_PATH/lib

if ( `rest-config --flags | grep -c \\\"REST_WELCOME=ON\\\"` ) then
rest-config --welcome
endif
\"
)
        "
)


foreach(mac ${rest_macros})

string(REPLACE " " "" mac ${mac})
string(REPLACE "rest" "" m ${mac})

install( CODE
"
file( APPEND \${CMAKE_INSTALL_PREFIX}/thisREST.csh 

\"
alias ${mac} \\\"restManager ${m}\\\"
\"
)
        "
)

endforeach(mac ${rest_macros})


#install rest-config
install( CODE
"
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CollectGitInfo.cmake)

message(STATUS \"Installing: \${CMAKE_INSTALL_PREFIX}/bin/rest-config\")


file( WRITE \${CMAKE_INSTALL_PREFIX}/bin/rest-config

\"

if [ $# -ne 1 ] ; then 

echo \\\"  Use restRoot command to load REST libraries and scripts inside ROOT          \\\"
echo \\\"  Use restManager command to manage the configurations and start REST          \\\"
echo \\\"  Type \\\\\\\"rest-config --help\\\\\\\" for more info                        \\\"    

else

option=$1

if [ $option = \\\"--incdir\\\" ] ; then
echo ${CMAKE_INSTALL_PREFIX}/include

fi

if [ $option = \\\"--libdir\\\" ] ; then
echo ${CMAKE_INSTALL_PREFIX}/lib

fi

if [ $option = \\\"--libs\\\" ] ; then
echo ${rest_libraries_regular}

fi

if [ $option = \\\"--exes\\\" ] ; then
echo ${rest_exes} 
echo -------------------------------------------
echo ${rest_macros_str}

fi

if [ $option = \\\"--version\\\" ] ; then
echo \${GIT_TAG}

fi

if [ $option = \\\"--commit\\\" ] ; then
echo \${GIT_COMMIT}

fi

if [ $option = \\\"--clean\\\" ] ; then
echo \${GIT_CLEANSTATE}

fi

if [ $option = \\\"--release\\\" ] ; then
echo \${REST_OFFICIAL_RELEASE}

fi

if [ $option = \\\"--flags\\\" ] ; then
echo REST_WELCOME=${REST_WELCOME}
echo REST_GARFIELD=${REST_GARFIELD}
echo REST_G4=${REST_G4}
echo REST_DATABASE=${REST_DATABASE}
echo CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
echo CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}

fi

if [ $option = \\\"--welcome\\\" ] ; then

if [ -t 0 ] ; then

echo \\\"  *****************************************************************************\\\"
echo \\\"  W E L C O M E   to  R E S T  \\\"
echo \\\"  \\\"
echo \\\"  Commit  : \${GIT_COMMIT} (\${GIT_DATE})  \\\"
echo \\\"  Branch/Version : \${GIT_BRANCH}/\${GIT_TAG}  \\\"
echo \\\"  Compilation date : ${date}  \\\"
echo \\\"  Clean state : \${GIT_CLEANSTATE} \\\"
echo \\\"  Official release : \${REST_OFFICIAL_RELEASE} \\\"
echo \\\"  \\\"
echo \\\"  Installed at : $REST_PATH  \\\"
echo \\\"  \\\"
echo \\\"  REST forum site : ezpc10.unizar.es  \\\"
echo \\\"  \\\"
echo \\\"  Remember that REST is made by physicists for physicists, \\\"
echo \\\"  who are supposed to toil and suffer till they become experts. \\\"
echo \\\"  *****************************************************************************\\\"
echo \\\"  \\\" 

fi

fi

if [ $option = \\\"--help\\\" ] ; then
echo \\\"  Usage :                                                                      \\\"
echo \\\"  rest-config [--incdir]  : Shows the directory of headers                      \\\"
echo \\\"  rest-config [--libdir]  : Shows the directory of library                      \\\"
echo \\\"  rest-config [--libs]    : Prints regular REST libraries                       \\\"
echo \\\"  rest-config [--exes]    : Prints a list of REST executables with alias        \\\"
echo \\\"  rest-config [--version] : Prints the version of REST                          \\\"
echo \\\"  rest-config [--welcome] : Prints the welcome message                          \\\"
echo \\\"  rest-config [--flags]   : Prints cmake flags defined when installing          \\\"
echo \\\"  rest-config [--release] : Prints 'Yes' if the compilation corresponds with an official git tag.      \\\"
echo \\\"  rest-config [--clean] : Prints 'Yes' if no local modifications were found during compilation   \\\"

fi


fi





\"
)
        "
)
