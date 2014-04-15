FORMS     =  mainwindow.ui
HEADERS   = mainwindow.h
SOURCES   = main.cpp \
            mainwindow.cpp

RESOURCES += ressources.qrc
TRANSLATIONS = fontinstaller_fr.ts

unix {
    TARGET = fontinstaller
}

win32 {
    TARGET = FontInstaller
    RC_FILE = FontInstaller.rc
    OTHER_FILES += FontInstaller.rc
}
