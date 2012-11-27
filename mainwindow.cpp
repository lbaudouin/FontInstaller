#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
   : QMainWindow(parent), ui(new Ui::MainWindowBase)
{
    ui->setupUi(this);

    connect(ui->buttonTry,SIGNAL(clicked()),this,SLOT(changeText()));
    connect(ui->lineEdit,SIGNAL(returnPressed()),this,SLOT(changeText()));
    connect(ui->buttonFolder,SIGNAL(clicked()),this,SLOT(openFolder()));
    connect(ui->buttonDefault,SIGNAL(clicked()),this,SLOT(loadDerfaultFont()));

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

    ui->listView->setVisible(false);

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
    connect(ui->buttonApply,SIGNAL(clicked()),this,SLOT(pressApply()));
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

    progress = new QMultiProgressWidget(3,QStringList()<<tr("Reading Files")<<tr("Loading Fonts")<<tr("Displaying Fonts"));
    connect(this,SIGNAL(setTextrogress(int,QString)),progress,SLOT(setText(int,QString)));
    connect(this,SIGNAL(setMinimumProgress(int,int)),progress,SLOT(setMinimum(int,int)));
    connect(this,SIGNAL(setMaximumProgress(int,int)),progress,SLOT(setMaximum(int,int)));
    connect(this,SIGNAL(setValueProgress(int,int)),progress,SLOT(setValue(int,int)));
    progress->show();
    progress->hide();
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
    ui->buttonApply->setVisible(visibility);
    ui->line_1->setVisible(visibility);
    ui->line_2->setVisible(visibility);
    ui->lineOptions->setVisible(visibility);
}

void MainWindow::pressApply()
{
    currentSize = ui->comboSize->currentText().toInt() ;
    emit this->setSize( currentSize );
    emit this->setSampleSize( ui->comboSampleSize->currentText().toInt() );
    int newNbCols = ui->comboColumns->currentText().toInt();
    if(nbCols!=newNbCols){
        nbCols = newNbCols;
        if(ui->radioGrid->isChecked())
            showFontGrid();
    }
}

void MainWindow::changeDisplay(int button)
{
    if(button==0)
        showFontGrid();
    else
        showFontLine();
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

void MainWindow::showFontGrid()
{
    QTime time;
    time.start();
    QWidget *w = new QWidget;
    QGridLayout *layout = new QGridLayout;
    w->setLayout(layout);

    this->setCursor(Qt::WaitCursor);
    emit this->setMaximumProgress(2,fonts.size()-1);
    emit this->setValueProgress(2,0);
    for(int i=0;i<fonts.size();i++){
        QFontLabel *fontLabel = new QFontLabel;
        connect(fontLabel,SIGNAL(selectFont(QFont)),this,SLOT(displayFont(QFont)));
        connect(this,SIGNAL(setSize(int)),fontLabel,SLOT(setNewSize(int)));
        QFont font = fonts.at(i);
        fontLabel->setNewFont( font );
        fontLabel->setNewSize( currentSize );
        fontLabel->setToolTip( QString("%1 %2").arg(font.family()).arg(font.styleName()) );
        fontLabel->setStatusTip( QString("%1 %2").arg(font.family()).arg(font.styleName()) );
        layout->addWidget(fontLabel,i/nbCols,i%nbCols);

        emit this->setValueProgress(2,i);
        if(i==0) emit this->fontChanged(font);
    }
    qDebug() << "Display Grid1" << time.elapsed();
    ui->scrollArea->setWidget(w);
    this->setCursor(Qt::ArrowCursor);
    ui->lineEdit->setEnabled(true);
    ui->buttonTry->setEnabled(true);
    qDebug() << "Display Grid2" << time.elapsed();
    progress->hide();
}

void MainWindow::showFontLine(QString text)
{
    QTime time;
    time.start();
  QWidget *w = new QWidget;
  QVBoxLayout *layout = new QVBoxLayout;
  w->setLayout(layout);

  this->setCursor(Qt::WaitCursor);
  for(int i=0;i<fonts.size();i++){
      QFontLabel *fontLabel = new QFontLabel;
      connect(this,SIGNAL(textChanged(QString)),fontLabel,SLOT(setNewText(QString)));
      connect(fontLabel,SIGNAL(selectFont(QFont)),this,SLOT(displayFont(QFont)));
      connect(this,SIGNAL(setSize(int)),fontLabel,SLOT(setNewSize(int)));
      QFont font = fonts.at(i);
      fontLabel->setNewFont( font );
      fontLabel->setNewSize( currentSize );
      fontLabel->setNewText( text.trimmed() );
      fontLabel->setToolTip( QString("%1 %2").arg(font.family()).arg(font.styleName()) );
      fontLabel->setStatusTip( QString("%1 %2").arg(font.family()).arg(font.styleName()) );
      layout->addWidget(fontLabel);

      emit this->setValueProgress(2,i);
      if(i==0) emit this->fontChanged(font);
  }
  qDebug() << "Display Line1" << time.elapsed();
  ui->scrollArea->setWidget(w);
  this->setCursor(Qt::ArrowCursor);
  ui->lineEdit->setEnabled(true);
  ui->buttonTry->setEnabled(true);
  qDebug() << "Display Line2" << time.elapsed();
}

void MainWindow::getFonts(int id)
{
    QStringList families = database.applicationFontFamilies( id );
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
      }
    }
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
    QTime time;
    time.start();
    progress->showProgress(0);
    progress->show();
    emit setInstallEnabled(true);
    fonts.clear();
    fonts_data.clear();
    database.removeAllApplicationFonts();
    QList<int> ids;
    for(int i=0;i<files.size();i++){
      FontData fd;
      fd.file = files.at(i);
      fd.id = database.addApplicationFont( files.at(i) );
      fd.families = database.applicationFontFamilies( fd.id );
      fonts_data.push_back(fd);
      ids.push_back(fd.id);
    }
    for(int i=0;i<ids.size();i++){
        getFonts(ids.at(i));
    }
    qDebug() << "Load" << time.elapsed();
    displayAllFont();
}

void MainWindow::loadDerfaultFont()
{
    QTime time;
    time.start();
    progress->hideProgress(0);
    progress->show();
    emit setInstallEnabled(false);
    fonts.clear();
    database.removeAllApplicationFonts();
    QStringList families = database.families();
    emit this->setMaximumProgress(1,families.size()-1);
    emit this->setValueProgress(1,0);
    int k=0;
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
      }

      emit this->setValueProgress(1,k);
      k++;
    }
    qDebug() << "Load" << time.elapsed();
    displayAllFont();
}

void MainWindow::displayAllFont()
{
    if(ui->radioGrid->isChecked())
        showFontGrid();
    else
        showFontLine();
    updateFontCount(fonts.size());
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
