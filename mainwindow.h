#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <QList>
#include <QDir>
#include <QFileDialog>
#include <QTimer>
#include <QButtonGroup>
#include <QDebug>
#include <QProgressDialog>
#include <QProgressBar>
#include <QDesktopServices>
#include <QUrl>

#include <QTime>
#include <QTreeWidget>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <QEventLoop>

#include <qfontlabel.h>

#include <QThreadPool>
#include <QRunnable>

#define VERSION "0.1.0"

namespace Ui {
    class MainWindow;
}

struct FontStyleInfo{
    QString style;
    QFont font;
    int width;
    QTableWidgetItem *item;
};

struct FontFamilyInfo{
    QString family;
    QList<FontStyleInfo> styles;
};

struct FontFileInfo{
    QList<FontFamilyInfo> families;
    QString file;
    int id;
};

inline bool operator<(const FontFamilyInfo &key1, const FontFamilyInfo &key2)
{
    return key1.family < key2.family;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    QFontDatabase database;

    QFontLabel *sample;
    int nbCols;
    int currentSize;

    QProgressBar *progressDisplay;

    bool stop;

protected:
    void updateFontCount(int nb);
    int findNbCol(QList<int> sizes, int widthMax);

public slots:
    void changeText();
    void selectFolder();
    void openFolder(QString path);
    void loadDefaultFont();
    void install();
    void setOptionsVisible(bool visibility);

    void displayFont(FontDisplay);
    void changeDisplay(int);

    void sizeChanged(int);
    void sampleSizeChanged(int);
    void nbColumnsChanged(int);
    void textNeededChanded(QString);


    void displaySelected();

    void aboutToQuit();

    void resizeEventPerso();

    void selectionChanged();
    void itemChanged(QTableWidgetItem*);
    void itemChanged(QTreeWidgetItem*);
signals:
    void textChanged(QString);
    void fontChanged(FontDisplay);
    void setInstallEnabled(bool);
    void setSampleSize(int);
    void setSize(int);
    void needText(QString);

    void open(QString);

};

#endif // MAINWINDOW_H
