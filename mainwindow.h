#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindowbase.h"
#include <QtGui>
#include <QList>
#include <QDir>
#include <QFileDialog>
#include <QTimer>
#include <QButtonGroup>

#include "QMultiProgressWidget.h"

class QFontLabel : public QLabel
{
    Q_OBJECT
public:
    QFontLabel(QString text = "Aa", int size = 15){
        pointSize = size;
        this->setNewText(text);
        QFont currentFont = this->font();
        currentFont.setPointSize(pointSize);
        this->setFont(currentFont);
    }
    void mousePressEvent(QMouseEvent *){
        emit this->selectFont(this->font());
    }
public slots:
    void setNewText(QString text){
        if(text.isEmpty()){
            setNewText("Aa");
            return;
        }
        this->setText(text);
    }
    void setNewFont(QFont font){
        font.setPointSize(pointSize);
        this->setFont(font);
    }
    void setNewSize(int size){
        pointSize = size;
        QFont currentFont = this->font();
        currentFont.setPointSize(pointSize);
        this->setFont(currentFont);
    }
signals:
    void selectFont(QFont);
private:
    int pointSize;
};

struct FontData{
    int id;
    QString file;
    QStringList families;
};

namespace Ui {
    class MainWindowBase;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    void changeEvent(QEvent *e);

private:
    Ui::MainWindowBase *ui;
    QFontDatabase database;
    QList<QFont> fonts;
    QStringList files;
    QFontLabel *sample;
    QList<FontData> fonts_data;
    int nbCols;
    int currentSize;
    QMultiProgressWidget *progress;

protected:
    void updateFontCount(int nb);
    void displayAllFont();

public slots:
    void changeText();
    void showFontLine(QString text = "");
    void showFontGrid();
    void getFonts(int id);
    void openFolder();
    void loadFonts();
    void loadDerfaultFont();
    void install();
    void setOptionsVisible(bool visibility);

    void displayFont(QFont);
    void changeDisplay(int);
    void pressApply();

signals:
    void textChanged(QString);
    void fontChanged(QFont);
    void setInstallEnabled(bool);
    void setSampleSize(int);
    void setSize(int);
    void setTextProgress(int,QString);
    void setMinimumProgress(int,int);
    void setMaximumProgress(int,int);
    void setValueProgress(int,int);
};

#endif // MAINWINDOW_H
