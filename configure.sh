#!/bin/bash
#
# Configuration script for creating a CMake build directory.
#
# Takes one parameter, which can be:
#
# Debug -- make a debug build (default)
# Release -- make a release build
# install -- installation build (install directory is derived from version)
#
# The optional second parameter determines the configuration script to use
# chuk / ussd / ussd-services / auto
#
# A third parameter can be used to set an installation path.
#
# Here are some examples:
#
# Make Debug version, autodetect config,
# install to /illumina/development/haplocompare/haplocompare-master-debug
#
# configure.sh Debug
#
# Make Release version, autodetect config,
# install to $HOME/haplotypes_testing
#
# configure.sh Release auto $HOME/haplotypes_testing
#
# Make Release version, San Diego production config
# install to $HOME/haplotypes_testing
#
# configure.sh Release ussd $HOME/haplotypes_testing


DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

unset SPECIALCONFIG

if [[ $2 == "auto" ]]; then
    unset CONFIGTYPE
else
    CONFIGTYPE="$2"
fi

if [[ -z $CONFIGTYPE ]]; then

if [[ "$(hostname)" == *chuk.illumina.com ]]; then
    echo "Using Illumina-UK configuration."
    SPECIALCONFIG="${DIR}/src/sh/chuk-setup.sh"
    . ${DIR}/src/sh/chuk-setup.sh
elif [[ "$(hostname)" == ukch-prd-lndt* ]] || [[ "$(hostname)" == ukch-dev-lnt* ]]; then
    echo "Using Illumina-UK EL6 configuration."
    SPECIALCONFIG="${DIR}/src/sh/chuk-el6-setup.sh"
    . ${DIR}/src/sh/chuk-el6-setup.sh
elif [[ "$(hostname)" == ussd-prd-lndt-b* ]]; then
    echo "using illumina-us configuration."
    SPECIALCONFIG="${DIR}/src/sh/ussd-setup.sh"
    . ${DIR}/src/sh/ussd-setup.sh
fi

else
    echo "using $CONFIGTYPE configuration."
    SPECIALCONFIG="${DIR}/src/sh/$CONFIGTYPE-setup.sh"
    . ${DIR}/src/sh/$CONFIGTYPE-setup.sh
fi

# DIR gets overwritten above
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [[ -z $1 ]]; then
    BT=Debug
else
    BT=$1
fi

shift 1

if [[ -z $1 ]]; then
    echo "Configuration detected automatically"
else
    shift 1
fi

if [[ "$(echo ${BT} | awk '{print tolower($0)}')" == "install" ]]; then
    BV=$(cd ${DIR} ; git describe)
    BT=Release
    PREFIX=/illumina/development/haplocompare/haplocompare-${BV}
else
    TARGET=$(echo ${BT} | awk '{print tolower($0)}')
    PREFIX=/illumina/development/haplocompare/haplocompare-master-${TARGET}
fi

if [[ ! -z $1 ]]; then
    PREFIX=$1
    shift 1
fi

if [[ -z ${CC} ]]; then
    CC=`which gcc`
fi

if [[ -z ${CXX} ]]; then
    CXX=`which g++`
fi

echo "Build from ${DIR}, type is $BT / installation directory: $PREFIX"

if [[ -z ${BOOST_ROOT} ]]; then
    echo "Using system Boost"
    cmake  ${DIR} -DCMAKE_BUILD_TYPE=$BT \
          -DCMAKE_INSTALL_PREFIX=${PREFIX} \
          -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX \
          $EXTRA_CMAKE_OPTS $@
else
    echo "Using Boost: $BOOST_ROOT"
    cmake  ${DIR} -DCMAKE_BUILD_TYPE=$BT \
          -DCMAKE_INSTALL_PREFIX=${PREFIX} \
          -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX \
          -DBOOST_ROOT=$BOOST_ROOT \
          $EXTRA_CMAKE_OPTS $@
fi


if [[ ! -z $SPECIALCONFIG ]]; then
    echo ""
    echo "*********************** NOTICE ****************************"
    echo ""
    echo "You need to use the correct version of binutils, this may  "
    echo "not get set up by cmake."
    echo ""
    echo "The easiest way to do this is by doing "
    echo ""
    echo "  . ${SPECIALCONFIG} "
    echo ""
    echo "before compiling."
    echo ""
    echo "***********************************************************"
fi
