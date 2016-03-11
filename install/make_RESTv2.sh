#!/bin/bash
set -e

source $(cd $(dirname $0); pwd)/REST_ENV.sh
echo $REST_PATH

cd $REST_PATH/source
    #make clean
    make -j4
    cd $WP

cd $REST_PATH/packages/restG4-build
    cmake -DGeant4_DIR=$(geant4-config --prefix)/lib/Geant4-10.2.0 $REST_PATH/packages/restG4
    #make clean
    make -j4
    cd $WP

