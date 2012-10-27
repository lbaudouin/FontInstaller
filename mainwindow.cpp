#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
   : QMainWindow(parent)
{
    setupUi(this);

    connect(buttonTry,SIGNAL(clicked()),this,SLOT(changeText()));
    connect(lineEdit,SIGNAL(returnPressed()),this,SLOT(changeText()));
    connect(buttonFolder,SIGNAL(clicked()),this,SLOT(openFolder()));
    connect(buttonDefault,SIGNAL(clicked()),this,SLOT(loadDerfaultFont()));

    QStringList families = database.families();
    this->setWindowTitle("Font Sample - " + QString::number(families.size()) + " fonts");

    QButtonGroup *buttonGroup = new QButtonGroup;
    buttonGroup->addButton(radioGrid,0);
    buttonGroup->addButton(radioLine,1);
    buttonGroup->setExclusive(true);

    lineEdit->setEnabled(false);
    buttonTry->setEnabled(false);

    sample = new QFontLabel("Aa",35);
    connect(this,SIGNAL(textChanged(QString)),sample,SLOT(setNewText(QString)));
    connect(this,SIGNAL(fontChanged(QFont)),sample,SLOT(setNewFont(QFont)));
    sampleLayout->addWidget(sample);

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    QPushButton *buttonInstall = new QPushButton("Install");
    buttonInstall->setEnabled(false);
    connect(this,SIGNAL(setInstallEnabled(bool)),buttonInstall,SLOT(setEnabled(bool)));
    buttonInstall->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Fixed);
    buttonLayout->addWidget(buttonInstall);
    connect(buttonInstall,SIGNAL(clicked()),this,SLOT(install()));
    QPushButton *buttonQuit = new QPushButton("Quit");
    buttonQuit->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Fixed);
    buttonLayout->addWidget(buttonQuit);
    connect(buttonQuit,SIGNAL(clicked()),this,SLOT(close()));
    sampleLayout->addLayout( buttonLayout );
}

void MainWindow::changeText()
{
     QString text = lineEdit->text().trimmed();
     emit this->textChanged(text);
}

void MainWindow::showFontGrid()
{

    lineEdit->setEnabled(true);
    buttonTry->setEnabled(true);
}

void MainWindow::showFontLine(QString text)
{
  QWidget *w = new QWidget;
  QVBoxLayout *layout = new QVBoxLayout;
  w->setLayout(layout);
  scrollArea->setWidget(w);

  this->setCursor(Qt::WaitCursor);
  for(int i=0;i<fonts.size();i++){
      QFontLabel *fontLabel = new QFontLabel;
      connect(this,SIGNAL(textChanged(QString)),fontLabel,SLOT(setNewText(QString)));
      connect(fontLabel,SIGNAL(selectFont(QFont)),this,SLOT(displayFont(QFont)));
      //QLabel *fontname = new QLabel;
      QFont font = fonts.at(i);
      fontLabel->setNewFont( font );
      fontLabel->setNewText( text.trimmed() );
      //fontname->setText( QString("%1 %2").arg(font.family()).arg(font.styleName()) );
      fontLabel->setToolTip( QString("%1 %2").arg(font.family()).arg(font.styleName()) );
      fontLabel->setStatusTip( QString("%1 %2").arg(font.family()).arg(font.styleName()) );
      //layout->addWidget(fontname,i,0);
      layout->addWidget(fontLabel);
  }
  this->setCursor(Qt::ArrowCursor);
  lineEdit->setEnabled(true);
  buttonTry->setEnabled(true);
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
    qDebug() << "Dir: " << dirpath;
    dirpath += "/";
    QDir dir(dirpath);
    files = dir.entryList(QStringList() << "*.ttf" << "*.TTF" << "*.otf" << "*.OTF");
    qDebug() << "Load " << files.size() << " font files";
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
    fonts.clear();
    fonts_data.clear();
    database.removeAllApplicationFonts();
    QList<int> ids;
    for(int i=0;i<files.size();i++){
      FontData fd;
      fd.file = files.at(i);
      fd.id = database.addApplicationFont( files.at(i) );
      fd.families = database.applicationFontFamilies( fd.id );
      ids.push_back(fd.id);
    }
    qDebug() << "Font Loaded (" << (double)time.elapsed()/1000.0 << "s - " << (double)time.elapsed()/(1000.0*files.size()) << "s/file)";
    time.restart();
    for(int i=0;i<ids.size();i++){
        getFonts(ids.at(i));
    }
    qDebug() << "Font Ready (" << (double)time.elapsed()/1000.0 << "s - " << (double)time.elapsed()/(1000.0*files.size()) << "s/font)";
    time.restart();
    showFontLine();
    qDebug() << "Font Display (" << (double)time.elapsed()/1000.0 << "s - " << (double)time.elapsed()/(1000.0*files.size()) << "s/font)";
    this->setWindowTitle("Font Sample - " + QString::number(fonts.size()) + " fonts");
}

void MainWindow::loadDerfaultFont()
{
    emit setInstallEnabled(false);
    QTime time;
    time.start();
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
      }
    }
    qDebug() << "Font Loaded (" << (double)time.elapsed()/1000.0 << "s - " << (double)time.elapsed()/(1000.0*files.size()) << "s/font)";
    time.restart();
    showFontLine();
    qDebug() << "Font Display (" << (double)time.elapsed()/1000.0 << "s - " << (double)time.elapsed()/(1000.0*files.size()) << "s/font)";

}

void MainWindow::install()
{

}

void MainWindow::displayFont(QFont font)
{
    emit this->fontChanged(font);
}
