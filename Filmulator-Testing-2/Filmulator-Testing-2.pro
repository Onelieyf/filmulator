# Add more folders to ship with the application, here
folder_01.source = qml/Filmulator-Testing-2
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += main.cpp \
    core/agitate.cpp \
    core/curves.cpp \
    core/develop.cpp \
    core/diffuse.cpp \
    core/exposure.cpp \
    core/imload.cpp \
    core/imread.cpp \
    core/imread_jpeg.cpp \
    core/imread_tiff.cpp \
    core/imwrite_jpeg.cpp \
    core/imwrite_tiff.cpp \
    core/initialize.cpp \
    core/layer_mix.cpp \
    core/merge_exps.cpp \
    core/output_file.cpp \
    core/postprocess.cpp \
    core/time_diff.cpp \
    core/filmimageprovider.cpp \
    database/sqlmodel.cpp \
    core/filmulate.cpp

# Installation path
# target.path =

# Please do not modify the following two lines. Required for deployment.
include(qtquick2applicationviewer/qtquick2applicationviewer.pri)
qtcAddDeployment()

OTHER_FILES += \
    Organize.qml

HEADERS += \
    core/filmsim.hpp \
    core/lut.hpp \
    core/matrix.hpp \
    core/filmimageprovider.h \
    database/sqlmodel.h \
    core/interface.h


QMAKE_CXXFLAGS += -DTOUT -O3 -fprefetch-loop-arrays -fopenmp -flto
#QMAKE_CFLAGS_DEBUG += -DTOUT -O3 -fprefetch-loop-arrays -fopenmp
QMAKE_LFLAGS += -O3 -fopenmp -flto

LIBS += -lpthread -ltiff -lexiv2 -ljpeg -lraw -lgomp

QT += sql
