#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
   : QMainWindow(parent), ui(new Ui::MainWindowBase)
{
    ui->setupUi(this);

    timerDisplay = new QTimer;
    timerDisplay->setInterval(1);
    timerDisplay->setSingleShot(false);
    connect(timerDisplay,SIGNAL(timeout()),this,SLOT(displayOneFont()));
    timerFile = new QTimer;
    timerFile->setInterval(5);
    timerFile->setSingleShot(false);
    connect(timerFile,SIGNAL(timeout()),this,SLOT(openOneFile()));

    progressDisplay = new QProgressBar;
    ui->statusBar->addWidget(progressDisplay);
    progressDisplay->hide();

    connect(ui->buttonTry,SIGNAL(clicked()),this,SLOT(changeText()));
    connect(ui->lineEdit,SIGNAL(returnPressed()),this,SLOT(changeText()));
    connect(ui->buttonFolder,SIGNAL(clicked()),this,SLOT(openFolder()));
    connect(ui->buttonDefault,SIGNAL(clicked()),this,SLOT(loadDerfaultFont()));
    connect(ui->buttonClear,SIGNAL(clicked()),this,SLOT(clearChoice()));

    updateFontCount(0);
    nbCols = 10;
    currentSize = 15;

    this->setMinimumSize(640,480);

    QButtonGroup *buttonGroup = new QButtonGroup;
    buttonGroup->addButton(ui->radioGrid,0);
    buttonGroup->addButton(ui->radioLine,1);
    buttonGroup->setExclusive(true);
    connect(buttonGroup,SIGNAL(buttonClicked(int)),this,SLOT(changeDisplay(int)));

    clearChoice();
    ui->splitter->setSizes(QList<int>() << 400 << 100);

    setOptionsVisible(false);

    for(int i=7;i<31;i++)
        ui->comboSize->addItem(QString::number(i));
    ui->comboSize->setCurrentIndex(8);
    for(int i=7;i<101;i++)
        ui->comboSampleSize->addItem(QString::number(i));
    ui->comboSampleSize->setCurrentIndex(28);
    for(int i=2;i<26;i++)
        ui->comboColumns->addItem(QString::number(i));
    ui->comboColumns->setCurrentIndex(8);

    connect(ui->comboSize,SIGNAL(currentIndexChanged(int)),this,SLOT(sizeChanged(int)));
    connect(ui->comboSampleSize,SIGNAL(currentIndexChanged(int)),this,SLOT(sampleSizeChanged(int)));
    connect(ui->comboColumns,SIGNAL(currentIndexChanged(int)),this,SLOT(nbColumnsChanged(int)));


    sample = new QFontLabel("Aa",35);
    connect(this,SIGNAL(textChanged(QString)),sample,SLOT(setNewText(QString)));
    connect(this,SIGNAL(fontChanged(FontDisplay)),sample,SLOT(setNewFont(FontDisplay)));
    connect(this,SIGNAL(setSampleSize(int)),sample,SLOT(setNewSize(int)));
    ui->sampleLayout->addWidget(sample);

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    QPushButton *buttonOptions = new QPushButton(tr("Options"));
    buttonOptions->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Fixed);
    buttonOptions->setCheckable(true);
    buttonLayout->addWidget(buttonOptions);
    connect(buttonOptions,SIGNAL(toggled(bool)),this,SLOT(setOptionsVisible(bool)));
    QPushButton *buttonInstall = new QPushButton(tr("Install"));
    buttonInstall->setEnabled(false);
    connect(this,SIGNAL(setInstallEnabled(bool)),buttonInstall,SLOT(setEnabled(bool)));
    buttonInstall->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Fixed);
    buttonLayout->addWidget(buttonInstall);
    connect(buttonInstall,SIGNAL(clicked()),this,SLOT(install()));
    QPushButton *buttonQuit = new QPushButton(tr("Quit"));
    buttonQuit->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Fixed);
    buttonLayout->addWidget(buttonQuit);
    connect(buttonQuit,SIGNAL(clicked()),this,SLOT(close()));
    ui->sampleLayout->addLayout( buttonLayout );
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::resetDisplay()
{
  timerDisplay->stop();
  QWidget *w = new QWidget;

  if(ui->radioGrid->isChecked()){
      gridLayout = new QGridLayout;
      w->setLayout(gridLayout);
  }else{
      lineLayout = new QVBoxLayout;
      w->setLayout(lineLayout);
  }
  ui->scrollArea->setWidget(w);
}

void MainWindow::setOptionsVisible(bool visibility)
{
    ui->optionsWidget->setVisible(visibility);
}

void MainWindow::changeDisplay(int)
{
  resetDisplay();
  indexDisplay = 0;
  timerDisplay->start();
}

void MainWindow::updateFontCount(int nb)
{
    QString title = tr("Font Install");
    if(nb!=0){
        title += " - " + QString::number(nb) + " " +tr("fonts");
    }
    this->setWindowTitle(title);
}

void MainWindow::changeText()
{
     QString text = ui->lineEdit->text().trimmed();
     emit this->textChanged(text);
}

void MainWindow::openFolder()
{
    timerFile->stop();
    timerDisplay->stop();
    QString defaultPath = "./Fonts/";
    QDir defaultDir(defaultPath);
    if(!defaultDir.exists())
        defaultPath = QDir::homePath();
    QString dirpath = QFileDialog::getExistingDirectory(this, tr("Open Directory"), defaultPath, QFileDialog::ShowDirsOnly);
    if(dirpath.isEmpty()){
        timerFile->start();
        timerDisplay->start();
        return;
    }
    dirpath += "/";
    QDir dir(dirpath);
    listFiles = dir.entryList(QStringList() << "*.ttf" << "*.TTF" << "*.otf" << "*.OTF");
    for(int i=0;i<listFiles.size();i++){
        listFiles[i].prepend(dirpath);
    }

    resetDisplay();
    indexDisplay = 0;
    fontsDisplay.clear();
    indexFiles = 0;
    timerFile->start();
}

void MainWindow::openOneFile()
{
    if(indexFiles<listFiles.size()){
      QString file = listFiles.at(indexFiles);
      int id = database.addApplicationFont( file );
      QStringList families = database.applicationFontFamilies( id );
      foreach(QString family, families){
        QStringList styles = database.styles(family);
        foreach(QString style, styles){
          if(style.isEmpty() || style.contains("oblique",Qt::CaseInsensitive))
              continue;

          int weight = database.weight(family,style);
          bool italic = database.italic(family,style);
          bool bold = database.bold(family,style);

          FontDisplay f;
          f.file = file;
          f.font = QFont(family, 32, weight, italic);
          f.font.setBold(bold);
          f.size = currentSize;
          f.name = QString("%1 %2").arg(f.font.family()).arg(f.font.styleName());

          QChar c = QString::fromUtf8("é").at(0);
          QRawFont rf = QRawFont::fromFont(f.font);
          if(!rf.supportsCharacter(c)){
            qDebug() << "No accent !" << f.file;
          }else{
            fontsDisplay.push_back(f);
          }

          if(!timerDisplay->isActive())
            timerDisplay->start();
        }
      }
      indexFiles++;
      updateFontCount(fontsDisplay.size());
    }else{
        timerFile->stop();
    }
}

void MainWindow::loadDerfaultFont()
{
    resetDisplay();
    indexDisplay = 0;
    fontsDisplay.clear();
    database.removeAllApplicationFonts();

    QStringList families = database.families();

    foreach(QString family, families){
      QStringList styles = database.styles(family);
      foreach(QString style, styles){
        if(style.isEmpty() || style.contains("oblique",Qt::CaseInsensitive))
            continue;

        int weight = database.weight(family,style);
        bool italic = database.italic(family,style);
        bool bold = database.bold(family,style);

        FontDisplay f;
        f.file = "";
        f.font = QFont(family, 32, weight, italic);
        f.font.setBold(bold);
        f.size = currentSize;
        f.name = QString("%1 %2").arg(f.font.family()).arg(f.font.styleName());

        QChar c = QString::fromUtf8("é").at(0);
        QRawFont rf = QRawFont::fromFont(f.font);
        if(!rf.supportsCharacter(c)){
          qDebug() << "No accent !" << f.name;
        }else{
          fontsDisplay.push_back(f);
        }


        if(!timerDisplay->isActive())
          timerDisplay->start();
      }
    }
    updateFontCount(fontsDisplay.size());
    for(int i=0;i<choiceFiles.size();i++)
      database.addApplicationFont( choiceFiles.at(i) );
}

void MainWindow::displayOneFont()
{
    if(indexDisplay<fontsDisplay.size()){
      FontDisplay f = fontsDisplay.at(indexDisplay);
      QFontLabel *fontLabel = new QFontLabel;
      if(ui->radioLine->isChecked())
          connect(this,SIGNAL(textChanged(QString)),fontLabel,SLOT(setNewText(QString)));
      connect(fontLabel,SIGNAL(selectFont(FontDisplay)),this,SLOT(displayFont(FontDisplay)));
      connect(this,SIGNAL(setSize(int)),fontLabel,SLOT(setNewSize(int)));
      connect(fontLabel,SIGNAL(choose(FontDisplay)),this,SLOT(addChoice(FontDisplay)));
      fontLabel->setNewFont( f );
      fontLabel->setNewSize( f.size );
      fontLabel->setToolTip( f.name );
      fontLabel->setStatusTip( f.name );
      if(ui->radioGrid->isChecked()){
          gridLayout->addWidget(fontLabel,indexDisplay/nbCols,indexDisplay%nbCols);
      }else{
          lineLayout->addWidget(fontLabel);
      }
      progressDisplay->setValue(indexDisplay);
      indexDisplay++;
      if(indexDisplay==fontsDisplay.size()){
          progressDisplay->hide();
          timerDisplay->stop();
      }
    }else{
        progressDisplay->hide();
        timerDisplay->stop();
        return;
    }
}

void MainWindow::install()
{
    QString file = sample->getFile();
    if(QFile::exists(file)){
        QDesktopServices::openUrl(QUrl("file:///" + file, QUrl::TolerantMode));
    }
}

void MainWindow::displayFont(FontDisplay fontInfo)
{
    sample->setNewFont(fontInfo);
    emit this->setInstallEnabled(!fontInfo.file.isEmpty());
}

void MainWindow::addChoice(FontDisplay fontInfo)
{
    ui->widget->setVisible(true);
    qDebug() << fontInfo.name;
    choiceFiles.push_back(fontInfo.file);
    QFontLabel *fontLabel = new QFontLabel;
    connect(this,SIGNAL(textChanged(QString)),fontLabel,SLOT(setNewText(QString)));
    connect(fontLabel,SIGNAL(selectFont(FontDisplay)),this,SLOT(displayFont(FontDisplay)));
    connect(this,SIGNAL(setSize(int)),fontLabel,SLOT(setNewSize(int)));
    fontLabel->setNewFont( fontInfo );
    fontLabel->setNewSize( fontInfo.size );
    fontLabel->setToolTip( fontInfo.name );
    fontLabel->setStatusTip( fontInfo.name );
    choiceLayout->addWidget(fontLabel);
    ui->scrollAreaChoice->setVisible(true);
}

void MainWindow::clearChoice()
{
    choiceFiles.clear();
    ui->widget->setVisible(false);
    choiceLayout = new QVBoxLayout;
    QWidget *w = new QWidget;
    w->setLayout(choiceLayout);
    ui->scrollAreaChoice->setWidget(w);
}


void MainWindow::sizeChanged(int index)
{
  currentSize = index + 7;
  emit this->setSize( index + 7 );
}

void MainWindow::sampleSizeChanged(int index)
{
  emit this->setSampleSize( index + 7 );
}

void MainWindow::nbColumnsChanged(int)
{
  int newNbCols = ui->comboColumns->currentText().toInt();
  if(nbCols!=newNbCols){
      nbCols = newNbCols;
      if(ui->radioGrid->isChecked())
        changeDisplay(0);
  }
}
