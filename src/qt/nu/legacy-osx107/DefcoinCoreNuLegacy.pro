QT += widgets network

CONFIG += c++11 release
TEMPLATE = app
TARGET = DefcoinCoreNuLegacy

DEFINES += DEFCOIN_NU_VERSION=\\\"26.3.1-lion\\\"

SOURCES += main.cpp

macx {
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
    ICON = ../assets/brand/DefcoinCoreNuNuIcon.icns
    QMAKE_INFO_PLIST = Info.plist
}
