TARGET = PoetryGenerator

OBJECTS_DIR = obj

QT += opengl core

isEqual(QT_MAJOR_VERSION, 5) {
        cache()
}

INCLUDEPATH += $$PWD/include

MOC_DIR = moc

SOURCES += $$PWD/main.cpp \
                     $$PWD/src/*.cpp

HEADERS += $$PWD/include/*.hpp

OTHER_FILES += readme.md \
               ./shaders/*.glsl \
               ./config/*.*cfg

CONFIG += console
CONFIG += c++11

win32 {
    INCLUDEPATH += C:\SDL2\include
    INCLUDEPATH += C:\SDL2\include
    LIBS += -L"C:/SDL2/lib/x64/" -lSDL2 -lSDL2main -lSDL2_image -lSDL2_mixer
    #LIBS += -L"C:\SDL2\i686-w64-mingw32\lib" -mwindows -lmingw32 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2main -lSDL2
    LIBS += -L"C:/NGL/lib/NGL.lib" -lNGL

    PRE_TARGETDEPS += C:/NGL/lib/NGL.lib
    INCLUDEPATH += -I c:/boost
    INCLUDEPATH += C:/NGL/include/
    DEFINES += GL42
    DEFINES += WIN32
    DEFINES += _WIN32
    DEFINES += _USE_MATH_DEFINES
    LIBS += -LC:/NGL/lib/ -lNGL
    LIBS += -lOpenGL32
    DEFINES += NO_DLL
}

unix {
    QMAKE_CXXFLAGS += $$system(sdl2-config --cflags)

    QMAKE_CXXFLAGS -= -O
    QMAKE_CXXFLAGS -= -O1
    QMAKE_CXXFLAGS -= -O2

    QMAKE_CXXFLAGS *= -O3

    LIBS += $$system(sdl2-config --libs)
    LIBS += -lSDL2_image -lSDL2_mixer -lSDL2_ttf
}

!equals(PWD, $${OUT_PWD}){
        copydata.commands = echo "creating destination dirs";
        # now make a dir
        copydata.commands += mkdir -p $$OUT_PWD/shaders;
        copydata.commands += echo "copying files";
        # then copy the files
        copydata.commands += $(COPY_DIR) $$PWD/shaders/* $$OUT_PWD/shaders/;
        # now make sure the first target is built before copy
        first.depends = $(first) copydata
        export(first.depends)
        export(copydata.commands)
        # now add it as an extra target
        QMAKE_EXTRA_TARGETS += first copydata
}

NGLPATH = $$(NGLDIR)
isEmpty(NGLPATH){ # note brace must be here
        message("including $HOME/NGL")
        include($(HOME)/NGL/UseNGL.pri)
}
else{ # note brace must be here
        message("Using custom NGL location")
        include($(NGLDIR)/UseNGL.pri)
}
