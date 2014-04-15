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
#include <QScrollBar>

#include <QTime>
#include <QTreeWidget>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <QEventLoop>
#include <QResizeEvent>

#include <QFontDatabase>

#include <QThreadPool>
#include <QRunnable>

#include <QSettings>

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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    bool needToUpdateText;

    QString selectedFileName;

    QStringList listCompare;

protected:
    void updateFontCount(int nb);
    int findNbCol(QList<int> sizes, int widthMax);

    void applyText();

    void resizeEvent(QResizeEvent *);

    void redrawTable();
    void redrawTable(QList<QTableWidgetItem*> list, QList<int> sizes);

    void compareItem(QTreeWidgetItem *item);
    void compareItem(QTableWidgetItem *item);

public slots:
    void changeText();
    void selectFolder();
    void openFolder(QString path);
    void loadDefaultFonts();

    void remove();
    void install();
    void installOneFont();

    void changeDisplay(int);

    void sizeChanged(int);
    void sampleSizeChanged(int);
    void textNeededChanded(QString);


    void compareSelected();

    void resizeEventPerso();
    void selectionChanged();

    void tabChanged(int);

};

#endif // MAINWINDOW_H
