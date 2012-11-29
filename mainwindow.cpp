#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
   : QMainWindow(parent), ui(new Ui::MainWindowBase)
{
    ui->setupUi(this);

    mainTimer = new QTimer;
    mainTimer->setInterval(1);
    mainTimer->setSingleShot(false);
    connect(mainTimer,SIGNAL(timeout()),this,SLOT(displayOneFont()));

    connect(ui->buttonTry,SIGNAL(clicked()),this,SLOT(changeText()));
    connect(ui->lineEdit,SIGNAL(returnPressed()),this,SLOT(changeText()));
    connect(ui->buttonFolder,SIGNAL(clicked()),this,SLOT(openFolder()));
    connect(ui->buttonDefault,SIGNAL(clicked()),this,SLOT(loadDerfaultFont()));
    connect(ui->buttonClear,SIGNAL(clicked()),this,SLOT(clearChoice()));

    updateFontCount(0);
    nbCols = 5;
    currentSize = 15;

    this->setMinimumSize(640,480);

    QButtonGroup *buttonGroup = new QButtonGroup;
    buttonGroup->addButton(ui->radioGrid,0);
    buttonGroup->addButton(ui->radioLine,1);
    buttonGroup->setExclusive(true);
    connect(buttonGroup,SIGNAL(buttonClicked(int)),this,SLOT(changeDisplay(int)));

    ui->lineEdit->setEnabled(false);
    ui->buttonTry->setEnabled(false);

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
    ui->comboColumns->setCurrentIndex(3);

    connect(ui->comboSize,SIGNAL(currentIndexChanged(int)),this,SLOT(sizeChanged(int)));
    connect(ui->comboSampleSize,SIGNAL(currentIndexChanged(int)),this,SLOT(sampleSizeChanged(int)));
    connect(ui->comboColumns,SIGNAL(currentIndexChanged(int)),this,SLOT(nbColumnsChanged(int)));


    sample = new QFontLabel("Aa",35);
    connect(this,SIGNAL(textChanged(QString)),sample,SLOT(setNewText(QString)));
    connect(this,SIGNAL(fontChanged(QFont)),sample,SLOT(setNewFont(QFont)));
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

void MainWindow::setOptionsVisible(bool visibility)
{
    ui->labelSize->setVisible(visibility);
    ui->labelSampleSize->setVisible(visibility);
    ui->labelColumns->setVisible(visibility);
    ui->comboSize->setVisible(visibility);
    ui->comboSampleSize->setVisible(visibility);
    ui->comboColumns->setVisible(visibility);
    ui->line_1->setVisible(visibility);
    ui->line_2->setVisible(visibility);
    ui->lineOptions->setVisible(visibility);
}

void MainWindow::changeDisplay(int)
{
   displayAllFont();
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
    QString defaultPath = "./Fonts/";
    QDir defaultDir(defaultPath);
    if(!defaultDir.exists())
        defaultPath = QDir::homePath();
    QString dirpath = QFileDialog::getExistingDirectory(this, tr("Open Directory"), defaultPath, QFileDialog::ShowDirsOnly);
    if(dirpath.isEmpty())
        return;
    dirpath += "/";
    QDir dir(dirpath);
    files = dir.entryList(QStringList() << "*.ttf" << "*.TTF" << "*.otf" << "*.OTF");
    for(int i=0;i<files.size();i++){
        files[i].prepend(dirpath);
    }

    emit setInstallEnabled(true);
    QTimer *timer = new QTimer();
    timer->setSingleShot(true);
    connect(timer,SIGNAL(timeout()),this,SLOT(loadFonts()));
    timer->start(500);
}

void MainWindow::loadFonts()
{
    emit setInstallEnabled(true);
    fonts.clear();
    fonts_data.clear();
    database.removeAllApplicationFonts();
    for(int i=0;i<files.size();i++){
      FontData fd;
      fd.file = files.at(i);
      fd.id = database.addApplicationFont( files.at(i) );
      fd.families = database.applicationFontFamilies( fd.id );
      fonts_data.push_back(fd);

      QStringList families = database.applicationFontFamilies( fd.id );
      foreach(QString family, families){
        QStringList styles = database.styles(family);
        foreach(QString style, styles){
          if(style.isEmpty() || style.contains("oblique",Qt::CaseInsensitive))
              continue;

          int weight = database.weight(family,style);
          bool italic = database.italic(family,style);
          bool bold = database.bold(family,style);

          QFont f = QFont(family, 32, weight, italic);
          f.setBold(bold);
          fonts.push_back(f);

          FontInfo fontInfo;
          fontInfo.font = f;
          fontInfo.name = QString("%1 %2").arg(f.family()).arg(f.styleName());
          fontInfo.file = "";
          allFonts.push_back(fontInfo);
        }
      }
    }
    displayAllFont();
}

void MainWindow::loadDerfaultFont()
{
    emit setInstallEnabled(false);
    allFonts.clear();
    fonts.clear();
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

        QFont f = QFont(family, 32, weight, italic);
        f.setBold(bold);
        fonts.push_back(f);

        FontInfo fontInfo;
        fontInfo.font = f;
        fontInfo.name = QString("%1 %2").arg(f.family()).arg(f.styleName());
        fontInfo.file = "";
        allFonts.push_back(fontInfo);
      }
    }
    displayAllFont();
}

void MainWindow::displayAllFont()
{
    mainTimer->stop();
    fontsDisplay.clear();
    QWidget *w = new QWidget;

    if(ui->radioGrid->isChecked()){
        gridLayout = new QGridLayout;
        w->setLayout(gridLayout);
    }else{
        lineLayout = new QVBoxLayout;
        w->setLayout(lineLayout);
    }
    ui->scrollArea->setWidget(w);

    this->setCursor(Qt::WaitCursor);
    for(int i=0;i<fonts.size();i++){
        QFont font = fonts.at(i);
        FontDisplay f;
        f.font = font;
        f.size = currentSize;
        f.name = QString("%1 %2").arg(font.family()).arg(font.styleName());
        fontsDisplay.push_back(f);

        if(i==0) emit this->fontChanged(font);
    }
    this->setCursor(Qt::ArrowCursor);
    ui->lineEdit->setEnabled(true);
    ui->buttonTry->setEnabled(true);

    mainIndex = 0;
    mainTimer->start();

    updateFontCount(fonts.size());
}

void MainWindow::displayOneFont()
{
    if(mainIndex>=fontsDisplay.size()){
        mainTimer->stop();
        return;
    }
    FontDisplay f = fontsDisplay.at(mainIndex);
    QFontLabel *fontLabel = new QFontLabel;
    if(ui->radioLine->isChecked())
        connect(this,SIGNAL(textChanged(QString)),fontLabel,SLOT(setNewText(QString)));
    connect(fontLabel,SIGNAL(selectFont(QFont)),this,SLOT(displayFont(QFont)));
    connect(this,SIGNAL(setSize(int)),fontLabel,SLOT(setNewSize(int)));
    connect(fontLabel,SIGNAL(choose(FontDisplay)),this,SLOT(addChoice(FontDisplay)));
    fontLabel->setNewFont( f );
    fontLabel->setNewSize( f.size );
    fontLabel->setToolTip( f.name );
    fontLabel->setStatusTip( f.name );
    if(ui->radioGrid->isChecked()){
        gridLayout->addWidget(fontLabel,mainIndex/nbCols,mainIndex%nbCols);
    }else{
        lineLayout->addWidget(fontLabel);
    }
    mainIndex++;
    if(mainIndex==fontsDisplay.size())
        mainTimer->stop();
}

void MainWindow::install()
{
    QString family = sample->font().family();
    for(int i=0;i<fonts_data.size();i++){
        if(fonts_data.at(i).families.contains(family)){
            QDesktopServices::openUrl(QUrl("file:///" + fonts_data.at(i).file, QUrl::TolerantMode));
        }
    }
}

void MainWindow::displayFont(QFont font)
{
    emit this->fontChanged(font);
}

void MainWindow::addChoice(FontDisplay fontInfo)
{
    ui->widget->setVisible(true);
    qDebug() << fontInfo.name;
    QFontLabel *fontLabel = new QFontLabel;
    connect(this,SIGNAL(textChanged(QString)),fontLabel,SLOT(setNewText(QString)));
    connect(fontLabel,SIGNAL(selectFont(QFont)),this,SLOT(displayFont(QFont)));
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
    qDebug() << "Clear Choice";
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
          displayAllFont();
  }
}
