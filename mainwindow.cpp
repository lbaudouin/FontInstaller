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

    QButtonGroup *buttonGroup = new QButtonGroup;
    buttonGroup->addButton(ui->radioGrid,0);
    buttonGroup->addButton(ui->radioLine,1);
    buttonGroup->setExclusive(true);

    ui->lineEdit->setEnabled(false);
    ui->buttonTry->setEnabled(false);

    sample = new QFontLabel("Aa",35);
    connect(this,SIGNAL(textChanged(QString)),sample,SLOT(setNewText(QString)));
    connect(this,SIGNAL(fontChanged(QFont)),sample,SLOT(setNewFont(QFont)));
    ui->sampleLayout->addWidget(sample);

    QVBoxLayout *buttonLayout = new QVBoxLayout;
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

    ui->lineEdit->setEnabled(true);
    ui->buttonTry->setEnabled(true);
}

void MainWindow::showFontLine(QString text)
{
  QWidget *w = new QWidget;
  QVBoxLayout *layout = new QVBoxLayout;
  w->setLayout(layout);
  ui->scrollArea->setWidget(w);

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

      if(i==0) emit this->fontChanged(font);
  }
  this->setCursor(Qt::ArrowCursor);
  ui->lineEdit->setEnabled(true);
  ui->buttonTry->setEnabled(true);
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
    showFontLine();
    updateFontCount(fonts.size());
}

void MainWindow::loadDerfaultFont()
{
    emit setInstallEnabled(false);
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
      }
    }
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
