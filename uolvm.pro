#-------------------------------------------------
#
# Project created by QtCreator 2016-07-13T10:53:40
#
#------------------------------------------------

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += __UNIX__

SOURCES +=\
    vm.c \
    vmSys.c \
    nativetable.c \
    main.c

HEADERS  += \
    errcode.h \
    instruction.h \
    vm.h
