#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindowbase.h"
#include <QtGui>
#include <QList>
#include <QDir>
#include <QFileDialog>
#include <QTimer>
#include <QButtonGroup>
#include <QDebug>

struct FontDisplay{
    QFont font;
    int size;
    QString name;
    QString file;
};

struct FontInfo{
    QFont font;
    QString name;
    QString file;
};

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
    void mousePressEvent(QMouseEvent *e){
        if(e->button()==Qt::RightButton)
            emit this->choose(font_);
        else
            emit this->selectFont(this->font());
    }
public slots:
    void setNewFont(FontDisplay fontInfo){
        setNewFont(fontInfo.font);
        font_ = fontInfo;
    }

    void setNewText(QString text){
        if(text.isEmpty()){
            setNewText("Aa");
        }else{
            this->setText(text);
        }
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
    void choose(FontDisplay);

private:
    int pointSize;
    FontDisplay font_;
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

    QList<FontInfo> allFonts;

    QList<FontDisplay> fontsDisplay;
    QGridLayout *gridLayout;
    QVBoxLayout *lineLayout;
    QVBoxLayout *choiceLayout;
    QTimer *mainTimer;
    int mainIndex;

protected:
    void updateFontCount(int nb);

public slots:
    void changeText();
    void openFolder();
    void loadFonts();
    void loadDerfaultFont();
    void install();
    void setOptionsVisible(bool visibility);

    void displayFont(QFont);
    void changeDisplay(int);
    void displayAllFont();

    void displayOneFont();

    void addChoice(FontDisplay fontInfo);
    void clearChoice();

    void sizeChanged(int);
    void sampleSizeChanged(int);
    void nbColumnsChanged(int);

signals:
    void textChanged(QString);
    void fontChanged(QFont);
    void setInstallEnabled(bool);
    void setSampleSize(int);
    void setSize(int);
};

#endif // MAINWINDOW_H
