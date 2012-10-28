/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindowbase.h"
#include <QtGui>
#include <QDebug>
#include <QList>
#include <QDir>
#include <QFileDialog>
#include <QTimer>
#include <QButtonGroup>

QT_BEGIN_NAMESPACE
class QTextEdit;
class QTreeWidget;
class QTreeWidgetItem;
QT_END_NAMESPACE

typedef QList<QTreeWidgetItem *> StyleItems;

class QFontLabel : public QLabel
{
    Q_OBJECT
public:
    QFontLabel(QString text = "Aa", int sampleSize = 15){
        size = sampleSize;
        this->setNewText(text);
        QFont currentFont = this->font();
        currentFont.setPointSize(size);
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
        font.setPointSize(size);
        this->setFont(font);
    }
signals:
    void selectFont(QFont);
private:
    int size;
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

protected:
    void updateFontCount(int nb);

public slots:
    void changeText();
    void showFontLine(QString text = "");
    void showFontGrid();
    void getFonts(int id);
    void openFolder();
    void loadFonts();
    void loadDerfaultFont();
    void install();

    void displayFont(QFont);

signals:
    void textChanged(QString);
    void fontChanged(QFont);
    void setInstallEnabled(bool);
};

#endif
