#-------------------------------------------------
#
# Project created by QtCreator 2016-11-21T09:27:45
#
#-------------------------------------------------

QT       += core gui
QT       += multimedia
QT       += multimediawidgets
QT       += charts
QT       += concurrent
QT       += sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

LIBS += -lpthread
TARGET = VajeganGUI
TEMPLATE = app

SOURCES += main.cpp\
        vajegangui.cpp \
    MFCC_ETSI.cpp \
    KWS.cpp \
    infra_vv_funcs.cpp \
    infra_vs_funcs.cpp \
    infra_vm_funcs.cpp \
    infra_ss_funcs.cpp \
    infra_sm_funcs.cpp \
    infra_mm_funcs.cpp \
    infra_exception.cpp \
    infra_binary.cpp \
    HtkFile.cpp \
    htk2txt.cpp \
    htk2db.cpp \
    htk_stats.cpp \
    htk_split.cpp \
    htk_shuffle.cpp \
    htk_ceps_dist.cpp \
    Dataset_KWS.cpp \
    cmd_line.cpp \
    Classifier_KWS.cpp \
    xyseriesiodevice.cpp \
    playercontrols.cpp \
    qaudiolevel.cpp \
    audiorecorder.cpp \
    wavfilewriter.cpp \
    audiodecoder.cpp \
    playlistmodel.cpp \
    login.cpp \
    qdatejalali.cpp

HEADERS  += vajegangui.h \
    MFCC_ETSI.h \
    KWS.h \
    kernels.imp \
    kernels.h \
    infra.h \
    infra_vv_funcs.h \
    infra_vs_funcs.h \
    infra_vm_funcs.h \
    infra_vector.imp \
    infra_vector.h \
    infra_svector.imp \
    infra_svector.h \
    infra_ss_funcs.h \
    infra_sm_funcs.h \
    infra_refcount_darray.imp \
    infra_refcount_darray.h \
    infra_mm_funcs.h \
    infra_matrix.imp \
    infra_matrix.h \
    infra_exception.h \
    infra_binary.h \
    HtkFile.h \
    GmmLikelihoodCalculator.h \
    Dataset_KWS.h \
    cmd_option.h \
    cmd_line.h \
    Classifier_KWS.h \
    array4dim.h \
    array3dim.h \
    active_set.imp \
    active_set.h \
    xyseriesiodevice.h \
    playerscontrol.h \
    qaudiolevel.h \
    audiorecorder.h \
    wavfilewriter.h \
    audiodecoder.h \
    playlistmodel.h \
    login.h \
    qdatejalali.h


FORMS    += vajegangui.ui \
    audiorecorder.ui \
    login.ui

RESOURCES += \
    resource.qrc

win32:RC_ICONS += voice.ico
