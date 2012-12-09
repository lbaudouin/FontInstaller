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
#include <QProgressDialog>

#include <QRawFont>

struct FontDisplay{
    QFont font;
    int size;
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
            emit this->selectFont(font_);
    }
    QString getFile(){
      return font_.file;
    }

public slots:
    void setNewFont(FontDisplay fontInfo){
        setNewFont(fontInfo.font);
        font_ = fontInfo;
        this->setStatusTip( fontInfo.name );
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
        rf_ = QRawFont::fromFont(font);
    }
    void setNewSize(int size){
        pointSize = size;
        QFont currentFont = this->font();
        currentFont.setPointSize(pointSize);
        this->setFont(currentFont);
    }
    void need(QString text){
      for(int i=0;i<text.size();i++){
        if(!rf_.supportsCharacter(text.at(i))){
          this->setVisible(false);
          return;
        }
      }
      this->setVisible(true);
    }

signals:
    void selectFont(FontDisplay);
    void choose(FontDisplay);

private:
    int pointSize;
    FontDisplay font_;
    QRawFont rf_;
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
    QStringList files;
    QFontLabel *sample;
    int nbCols;
    int currentSize;

    QList<FontDisplay> fontsDisplay;
    QList<QString> choiceFiles;
    QGridLayout *gridLayout;
    QVBoxLayout *lineLayout;
    QVBoxLayout *choiceLayout;
    QTimer *timerFile;
    QTimer *timerDisplay;
    int indexDisplay;
    QStringList listFiles;
    int indexFiles;

    QProgressBar *progressDisplay;

protected:
    void updateFontCount(int nb);
    void resetDisplay();

public slots:
    void changeText();
    void openFolder();
    void loadDefaultFont();
    void install();
    void setOptionsVisible(bool visibility);

    void displayFont(FontDisplay);
    void changeDisplay(int);

    void openOneFile();
    void displayOneFont();

    void addChoice(FontDisplay fontInfo);
    void clearChoice();

    void sizeChanged(int);
    void sampleSizeChanged(int);
    void nbColumnsChanged(int);
    void textNeededChanded(QString);

signals:
    void textChanged(QString);
    void fontChanged(FontDisplay);
    void setInstallEnabled(bool);
    void setSampleSize(int);
    void setSize(int);
    void needText(QString);
};

#endif // MAINWINDOW_H
