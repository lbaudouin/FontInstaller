#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), needToUpdateText(false)
{
    QCoreApplication::setApplicationName( "FontInstaller" );

    QSettings settings(qAppName(),qAppName());

    ui->setupUi(this);

    updateFontCount(0);

    //Display mode
    QString displayType = settings.value("display","grid").toString();
    if(displayType=="grid"){
        ui->treeWidget->hide();
    }else{
        ui->tableWidget->hide();
        ui->radioGrid->setChecked(false);
        ui->radioLine->setChecked(true);
    }
    QButtonGroup *buttonGroup = new QButtonGroup;
    buttonGroup->addButton(ui->radioGrid,0);
    buttonGroup->addButton(ui->radioLine,1);
    buttonGroup->setExclusive(true);
    connect(buttonGroup,SIGNAL(buttonClicked(int)),this,SLOT(changeDisplay(int)));

    //Options
    ui->optionsWidget->hide();
    for(int i=7;i<41;i++)
        ui->comboSize->addItem(QString::number(i));
    int defaultSize = settings.value("size/default").toInt();
    if(defaultSize<7 || defaultSize>40)
        defaultSize = 15;
    ui->comboSize->setCurrentIndex(defaultSize-7);
    for(int i=7;i<101;i++)
        ui->comboSampleSize->addItem(QString::number(i));
    int sampleSize = settings.value("size/sample").toInt();
    if(sampleSize<7 || sampleSize>100)
        sampleSize = 35;
    ui->comboSampleSize->setCurrentIndex(sampleSize-7);

    connect(ui->comboSize,SIGNAL(currentIndexChanged(int)),this,SLOT(sizeChanged(int)));
    connect(ui->comboSampleSize,SIGNAL(currentIndexChanged(int)),this,SLOT(sampleSizeChanged(int)));

    ui->defaultFolderLineEdit->setText(settings.value("defaultFolder").toString());
    connect(ui->defaultFolderToolButton,SIGNAL(clicked()),this,SLOT(selectDefaultFolder()));

    //Sample
    QFont font = ui->sample->font();
    if(font.styleName().isEmpty())
        ui->fontNameLabel->setText(font.family());
    else
        ui->fontNameLabel->setText(font.family() + " - " + font.styleName());

    font.setPointSize(35);
    ui->sample->setFont(font);
    ui->sample->setText(ui->lineEdit->text());

    //Set UI
    ui->installFontButton->setDisabled(true);
    ui->scrollSample->setWidgetResizable(true);

    //Connect other signals
    connect(ui->lineEdit,SIGNAL(returnPressed()),this,SLOT(changeText()));
    connect(ui->buttonOptions,SIGNAL(toggled(bool)),ui->optionsWidget,SLOT(setVisible(bool)));
    connect(ui->installFontButton,SIGNAL(clicked()),this,SLOT(installOneFont()));
    connect(ui->installButton,SIGNAL(clicked()),this,SLOT(install()));
    connect(ui->removeButton,SIGNAL(clicked()),this,SLOT(remove()));
    connect(ui->buttonQuit,SIGNAL(clicked()),this,SLOT(close()));
    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(tabChanged(int)));

    connect(ui->buttonDefault,SIGNAL(clicked()),this,SLOT(loadDefaultFonts()));
    connect(ui->action_Default_Fonts,SIGNAL(triggered()),this,SLOT(loadDefaultFonts()));
    connect(ui->buttonFolder,SIGNAL(clicked()),this,SLOT(selectFolder()));
    connect(ui->action_Open_folder,SIGNAL(triggered()),this,SLOT(selectFolder()));

    connect(ui->compareButton,SIGNAL(clicked()),this,SLOT(compareSelected()));

    connect(ui->tableWidget,SIGNAL(itemSelectionChanged()),this,SLOT(selectionChanged()));
    connect(ui->treeWidget,SIGNAL(itemSelectionChanged()),this,SLOT(selectionChanged()));

    connect(ui->needCharCheckBox,SIGNAL(clicked(bool)),this,SLOT(needAllChararcters(bool)));

#ifndef __WIN32__
    connect(ui->action_Update_system_font_list,SIGNAL(triggered()),this,SLOT(updateSystemFontList()));
#else
    ui->action_Update_system_font_list->setVisible(false);
#endif

    connect(ui->action_About,SIGNAL(triggered()),this,SLOT(pressAbout()));
}

MainWindow::~MainWindow()
{
    QSettings settings(qAppName(),qAppName());
    settings.setValue("sizes/default",ui->comboSize->currentIndex()+7);
    settings.setValue("sizes/sample",ui->comboSampleSize->currentIndex()+7);
    settings.setValue("display",ui->radioGrid->isChecked()?"grid":"line");
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if(event->oldSize().width()!=event->size().width())
        redrawTable();
}

void MainWindow::redrawTable()
{
    QList<QTableWidgetItem*> selectedItem = ui->tableWidget->selectedItems();

    QList<QTableWidgetItem*> items;
    QList<int> sizes;
    for(int i=0;i<ui->tableWidget->rowCount();i++){
        for(int j=0;j<ui->tableWidget->columnCount();j++){
            QTableWidgetItem *item =  ui->tableWidget->item(i,j);
            if(item){
                sizes << item->data(Qt::UserRole+3).toInt();
                items << ui->tableWidget->takeItem(i,j);
            }
        }
    }
    redrawTable(items,sizes);
    if(selectedItem.size()==1){
        selectedItem.at(0)->setSelected(true);
    }
}

void MainWindow::redrawTable(QList<QTableWidgetItem*> items, QList<int> sizes)
{
    ui->tableWidget->clear();
    int width = ui->tableWidget->width() - ui->tableWidget->verticalScrollBar()->width();
    int nb = items.size();

    if(nb==0)
        return;

    int nbCols = findNbCol(sizes,width);
    int nbRow = (nb%nbCols==0?nb/nbCols:nb/nbCols+1);

    ui->tableWidget->setColumnCount(nbCols);
    ui->tableWidget->setRowCount(nbRow);

    for(int i=0;i<ui->tableWidget->rowCount();i++){
        for(int j=0;j<ui->tableWidget->columnCount();j++){
            if(i*nbCols+j<nb)
                ui->tableWidget->setItem(i,j,items.at(i*nbCols+j));
        }
    }

    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
}

void MainWindow::changeDisplay(int)
{
    if(ui->radioGrid->isChecked()){
        ui->tableWidget->show();
        ui->treeWidget->hide();
        redrawTable();
    }else{
        ui->tableWidget->hide();
        ui->treeWidget->show();
        if(needToUpdateText){
           applyText();
        }
    }
}

void MainWindow::updateFontCount(int nb)
{
    QString title = tr("Font Installer");
    if(nb!=0){
        title += " - " + QString::number(nb) + " " +tr("fonts");
    }
    this->setWindowTitle(title);
}

void MainWindow::selectDefaultFolder()
{
    QString defaultPath = "./Fonts/";
    QDir defaultDir;
    if(!defaultDir.exists(defaultPath)){
        defaultPath = QDir::homePath() + "/Documents/Fonts/";
        if(!defaultDir.exists(defaultPath)){
            defaultPath = QDir::homePath();
        }
    }

    QString folderPath = QFileDialog::getExistingDirectory(this,tr("Select default font folder"),defaultPath);

    if(folderPath.isEmpty())
        return;

    QSettings settings(qAppName(),qAppName());

    settings.setValue("defaultFolder",folderPath);

    ui->defaultFolderLineEdit->setText(folderPath);
}

void MainWindow::changeText()
{
    QString text = ui->lineEdit->text().trimmed();
    ui->sample->setText(text);
     if(ui->radioLine->isChecked()){
        applyText();
     }else{
         needToUpdateText = true;
     }
     needAllChararcters(ui->needCharCheckBox->isChecked());
}

void MainWindow::applyText()
{
    QString text = ui->lineEdit->text().trimmed();
    needToUpdateText = false;

    QTreeWidgetItem *root = ui->treeWidget->invisibleRootItem();

    if(root->childCount()==0)
        return;

    QProgressDialog *progressDialog = new QProgressDialog(tr("Applying"),"",0,root->childCount()-1,this);
    progressDialog->setWindowTitle( this->windowTitle() );
    progressDialog->setModal(true);

    QProgressBar* progressBar = progressDialog->findChild<QProgressBar*>();
    if(progressBar){
      progressBar->setFormat("%v/%m (%p%)");
    }

    progressDialog->show();

    for(int i=0;i<root->childCount();i++){
        for(int j=0;j<root->child(i)->childCount();j++){
           root->child(i)->child(j)->setText(1,text);
        }
        progressDialog->setValue( progressDialog->value()+1 );
        QCoreApplication::processEvents();
    }
    ui->treeWidget->resizeColumnToContents(1);
}

void MainWindow::selectFolder()
{
    QSettings settings(qAppName(),qAppName());

    QString defaultPath = settings.value("defaultFolder",QDir::homePath()).toString();
    QString dirpath = QFileDialog::getExistingDirectory(this, tr("Open Directory"), defaultPath, QFileDialog::ShowDirsOnly);

    QCoreApplication::processEvents();

    openFolder(dirpath);

}

void MainWindow::openFolder(QString path)
{
    if(path.isEmpty())
        return;

    QDir dir(path);
    if(!dir.exists())
        return;

    QStringList listFiles = dir.entryList(QStringList() << "*.ttf" << "*.TTF" << "*.otf" << "*.OTF");

    if(listFiles.isEmpty())
        return;

    this->setCursor( Qt::WaitCursor );

    QFontDatabase::removeAllApplicationFonts();

    QProgressDialog *progressDialog = new QProgressDialog(tr("Loading"),tr("Cancel"),0,listFiles.size()-1,this);
    progressDialog->setWindowTitle( this->windowTitle() );
    progressDialog->setModal(true);

    QProgressBar* progressBar = progressDialog->findChild<QProgressBar*>();
    if(progressBar){
      progressBar->setFormat("%v/%m (%p%)");
    }

    progressDialog->show();

    QCoreApplication::processEvents();

    QFontDatabase fontdatabase;

    ui->treeWidget->clear();
    ui->tableWidget->clear();

    ui->tableWidget->setColumnCount(1);
    ui->tableWidget->setRowCount(1);

    QString text = ui->lineEdit->text();

    QList<QTableWidgetItem*> tableItemList;
    QList<int> sizes;

    QList<FontFileInfo> fontFilesInfo;

    int nb = 0;
    for(int i=0;i<listFiles.size();i++){
        QString file = dir.absoluteFilePath(listFiles.at(i));

        int id = QFontDatabase::addApplicationFont( file );
        QStringList families = QFontDatabase::applicationFontFamilies( id );

        FontFileInfo fontFileInfo;
        fontFileInfo.file = file;

        foreach (const QString &family, families)  {

           QStringList stylesList = fontdatabase.styles(family);
           if(stylesList.isEmpty())
               continue;

           FontFamilyInfo fontFamilyInfo;
           fontFamilyInfo.family = family;


           QList<FontStyleInfo> fontStyleInfo;

           foreach (const QString &style, stylesList)  {

               if(style.isEmpty() || style.contains("oblique",Qt::CaseInsensitive))
                   continue;

               int weight = fontdatabase.weight(family,style);
               bool italic = fontdatabase.italic(family,style);
               bool bold = fontdatabase.bold(family,style);

               QFont font(family, ui->comboSize->currentIndex()+7, weight, italic);
               font.setBold(bold);

               nb++;

               QTableWidgetItem *tableItem = new QTableWidgetItem("Aa");
               tableItem->setStatusTip(family);
               tableItem->setToolTip(family);
               tableItem->setCheckState(Qt::Unchecked);
               tableItem->setData(Qt::UserRole,file);
               tableItem->setData(Qt::UserRole+1,family);
               tableItem->setData(Qt::UserRole+2,style);
               tableItem->setToolTip(family);
               tableItem->setFont(font);
               tableItemList << tableItem;

               ui->tableWidget->setItem(0,0,tableItem);
               ui->tableWidget->resizeColumnsToContents();
               ui->tableWidget->resizeRowsToContents();

               int itemWidth = ui->tableWidget->visualItemRect(tableItem).width();
               sizes << itemWidth;

               tableItem->setData(Qt::UserRole+3,itemWidth);

               FontStyleInfo fsi;
               fsi.style = style;
               fsi.font = font;
               fsi.width = ui->tableWidget->visualItemRect(tableItem).width();

               if(ui->radioGrid->isChecked())
                QCoreApplication::processEvents();

               ui->tableWidget->takeItem(0,0);

               fontStyleInfo.push_back(fsi);
           }

           fontFamilyInfo.styles = fontStyleInfo;

           if(!fontFamilyInfo.styles.isEmpty())
            fontFileInfo.families.push_back(fontFamilyInfo);
        }

        if(!families.isEmpty())
            fontFilesInfo.push_back(fontFileInfo);

        QFontDatabase::removeApplicationFont(id);

        if(progressDialog->wasCanceled()){
            ui->tableWidget->clear();
            ui->treeWidget->clear();
            ui->tableWidget->setColumnCount(0);
            ui->tableWidget->setRowCount(0);
            return;
        }

        progressDialog->setValue( progressDialog->value()+1 );
    }


    ui->tableWidget->setColumnCount(0);
    ui->tableWidget->setRowCount(0);

    QProgressDialog * progressDisplay = new QProgressDialog(tr("Displaying"),tr("Cancel"),0,nb,this);
    progressDisplay->setWindowTitle( this->windowTitle() );
    progressDisplay->setModal(true);
    progressDisplay->show();
    QCoreApplication::processEvents();

    nb = 0;
    for(int i=0;i<fontFilesInfo.size();i++){

        QString file = fontFilesInfo.at(i).file;
        fontFilesInfo[i].id = QFontDatabase::addApplicationFont( file );

        for(int j=0;j<fontFilesInfo.at(i).families.size();j++){

            QString family = fontFilesInfo.at(i).families.at(j).family;

            QTreeWidgetItem *familyItem = new QTreeWidgetItem(ui->treeWidget);
            familyItem->setText(0, family);
            familyItem->setExpanded(true);
            familyItem->setData(0,Qt::UserRole,file);

            for(int k=0;k<fontFilesInfo.at(i).families.at(j).styles.size();k++){

                QFont font = fontFilesInfo.at(i).families.at(j).styles.at(k).font;
                QString style = fontFilesInfo.at(i).families.at(j).styles.at(k).style;

                QTreeWidgetItem *styleItem = new QTreeWidgetItem(familyItem);
                styleItem->setCheckState(0,Qt::Unchecked);
                styleItem->setText(0, style);
                styleItem->setText(1, text);
                styleItem->setFont(1, font);
                styleItem->setStatusTip(1,family);
                styleItem->setToolTip(1,family);
                styleItem->setData(0,Qt::UserRole,file);
                styleItem->setData(0,Qt::UserRole+1,family);
                styleItem->setData(0,Qt::UserRole+2,style);
                styleItem->setData(1,Qt::UserRole,file);
                styleItem->setData(1,Qt::UserRole+1,family);
                styleItem->setData(1,Qt::UserRole+2,style);
                nb++;

                progressDisplay->setValue( progressDisplay->value()+1 );
            }
        }
        if(progressDisplay->wasCanceled()){
            ui->tableWidget->clear();
            ui->treeWidget->clear();
            ui->tableWidget->setColumnCount(0);
            ui->tableWidget->setRowCount(0);
            return;
        }
    }

    redrawTable(tableItemList,sizes);

    progressDisplay->setValue( progressDisplay->value()+1 );

    ui->treeWidget->resizeColumnToContents(0);
    ui->treeWidget->resizeColumnToContents(1);

    needAllChararcters(ui->needCharCheckBox->isChecked());

    ui->tableWidget->item(0,0)->setSelected(true);
    ui->treeWidget->invisibleRootItem()->child(0)->setSelected(true);

    this->setCursor( Qt::ArrowCursor );
}


void MainWindow::resizeEventPerso()
{

}

void MainWindow::loadDefaultFonts()
{

    QFontDatabase fontdatabase;

    QTreeWidget *fontTree = ui->treeWidget;
    fontTree->clear();

    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(1);
    ui->tableWidget->setColumnCount(1);


    QStringList families = fontdatabase.families();

    QProgressDialog *progressDialog = new QProgressDialog(tr("Loading"),tr("Cancel"),0,families.size()-1,this);
    progressDialog->setWindowTitle( this->windowTitle() );
    progressDialog->setModal(true);
    QProgressBar* progressBar = progressDialog->findChild<QProgressBar*>();
    if(progressBar){
      progressBar->setFormat("%v/%m (%p%)");
    }
    progressDialog->show();

    QList<QTableWidgetItem*> tableItemList;

    QList<int> sizes;

    int nb = 0;
    foreach (const QString &family, families)  {
       QTreeWidgetItem *familyItem = new QTreeWidgetItem(fontTree);
       familyItem->setText(0, family);
       familyItem->setExpanded(true);
       familyItem->setData(0,Qt::UserRole,"");
       familyItem->setData(0,Qt::UserRole+1,family);


       foreach (const QString &style, fontdatabase.styles(family))  {

           if(style.isEmpty() || style.contains("oblique",Qt::CaseInsensitive))
               continue;

           QTreeWidgetItem *styleItem = new QTreeWidgetItem(familyItem);
           styleItem->setText(0, style);

           QString text = ui->lineEdit->text();

           int weight = fontdatabase.weight(family,style);
           bool italic = fontdatabase.italic(family,style);
           bool bold = fontdatabase.bold(family,style);

           QFont font(family, ui->comboSize->currentIndex()+7, weight, italic);
           font.setBold(bold);

           styleItem->setText(1, text);
           styleItem->setFont(1, font);
           styleItem->setData(1,Qt::UserRole,"");
           styleItem->setData(1,Qt::UserRole+1,family);
           styleItem->setData(1,Qt::UserRole+2,style);
           styleItem->setCheckState(0,Qt::Unchecked);

           QTableWidgetItem *tableItem = new QTableWidgetItem("Aa");
           tableItem->setStatusTip(family);
           tableItem->setToolTip(family);
           tableItem->setCheckState(Qt::Unchecked);
           tableItem->setFont(font);
           tableItem->setData(Qt::UserRole,"");
           tableItem->setData(Qt::UserRole+1,family);
           tableItem->setData(Qt::UserRole+2,style);
           tableItemList << tableItem;

           ui->tableWidget->setItem(0,0,tableItem);
           ui->tableWidget->resizeColumnsToContents();
           ui->tableWidget->resizeRowsToContents();

           int itemWidth = ui->tableWidget->visualItemRect(tableItem).width();
           sizes << itemWidth;

           tableItem->setData(Qt::UserRole+3,itemWidth);

           if(ui->radioGrid->isChecked())
              QCoreApplication::processEvents();

           ui->tableWidget->takeItem(0,0);

           nb++;
       }

       if(progressDialog->wasCanceled()){
           ui->tableWidget->clear();
           ui->treeWidget->clear();
           ui->tableWidget->setColumnCount(0);
           ui->tableWidget->setRowCount(0);
           return;
       }

       progressDialog->setValue( progressDialog->value()+1 );
    }

    fontTree->resizeColumnToContents(0);
    fontTree->resizeColumnToContents(1);

    redrawTable(tableItemList,sizes);

    needAllChararcters(ui->needCharCheckBox->isChecked());

    ui->tableWidget->item(0,0)->setSelected(true);
    ui->treeWidget->invisibleRootItem()->child(0)->setSelected(true);

    this->setCursor( Qt::ArrowCursor );

    return;
}

int MainWindow::findNbCol(QList<int> sizes, int widthMax)
{
    int c = 0;
    int totalwidth = 0;
    while(totalwidth<widthMax){
        c++;
        QVector<int> cols(c);
        cols.reserve(c);
        for(int k=0;k<sizes.size();k++){
            cols[k%c] = qMax(cols[k%c],sizes.at(k));
        }
        totalwidth = 0;
        for(int k=0;k<cols.size();k++){
            totalwidth += cols.at(k);
        }
        if(c>100)
            break;
    }
    return qMax(1,c-1);
}

void MainWindow::installOneFont()
{
    if(QFile::exists(selectedFileName)){
        QDesktopServices::openUrl(QUrl("file:///" + selectedFileName, QUrl::TolerantMode));
    }
}

void MainWindow::install()
{
    QTreeWidgetItem *root = ui->compareTreeWidget->invisibleRootItem();
    for(int i=0;i<root->childCount();i++){
        QTreeWidgetItem *item = root->child(i);
        if(item->checkState(0)==Qt::Checked){
            QString filename = item->data(0,Qt::UserRole).toString();
            qDebug() << filename;
            if(!filename.isEmpty()){
                if(QFile::exists(filename)){
                    QDesktopServices::openUrl(QUrl("file:///" + filename, QUrl::TolerantMode));
                }
            }
            item->setCheckState(0,Qt::PartiallyChecked);
        }
    }
}

void MainWindow::remove()
{
    QList<QTreeWidgetItem*> toRemove;
    QTreeWidgetItem *root = ui->compareTreeWidget->invisibleRootItem();
    for(int i=0;i<root->childCount();i++){
        QTreeWidgetItem *item = root->child(i);
        if(item->checkState(0)==Qt::Checked){
            QString family = item->data(0,Qt::UserRole+1).toString();
            QString style = item->data(0,Qt::UserRole+2).toString();
            QString fontSignature = family + " - " + style;
            listCompare.removeAll(fontSignature);
            toRemove << item;
        }
    }
    for(int i=0;i<toRemove.size();i++)
        delete toRemove[i];

    int nbCompare = ui->compareTreeWidget->invisibleRootItem()->childCount();
    if(nbCompare>0){
        ui->tabWidget->setTabText(1,tr("Compare (%1)").arg(nbCompare));
    }else{
        ui->tabWidget->setTabText(1,tr("Compare"));
    }
}

void MainWindow::sizeChanged(int index)
{
  int currentSize = index + 7;

  QTreeWidgetItem *root = ui->treeWidget->invisibleRootItem();
  for(int i=0;i<root->childCount();i++){
      for(int j=0;j<root->child(i)->childCount();j++){
          QFont font = root->child(i)->child(j)->font(1);
          font.setPointSize(currentSize);
          root->child(i)->child(j)->setFont(1,font);
      }
  }
  QList<QTableWidgetItem*> items;
  for(int i=0;i<ui->tableWidget->rowCount();i++){
      for(int j=0;j<ui->tableWidget->columnCount();j++){
          QTableWidgetItem *item = ui->tableWidget->takeItem(i,j);
          if(item){
              QFont font = item->font();
              font.setPointSize(currentSize);
              item->setFont(font);
              items << item;
          }
      }
  }
  ui->tableWidget->clear();
  ui->tableWidget->setRowCount(1);
  ui->tableWidget->setColumnCount(1);

  QList<int> sizes;
  for(int i=0;i<items.size();i++){
      ui->tableWidget->setItem(0,0,items.at(i));
      ui->tableWidget->resizeColumnsToContents();
      ui->tableWidget->resizeRowsToContents();
      sizes << ui->tableWidget->visualItemRect(items.at(i)).width();
      ui->tableWidget->takeItem(0,0);
  }

  redrawTable(items,sizes);

  ui->treeWidget->resizeColumnToContents(1);
  ui->compareTreeWidget->resizeColumnToContents(1);
}

void MainWindow::sampleSizeChanged(int index)
{
    QFont font = ui->sample->font();
    font.setPointSize(index + 7);
    ui->sample->setFont(font);

    QCoreApplication::processEvents();

    ui->scrollSample->setFixedHeight( ui->sample->sizeHint().height() + ui->scrollSample->horizontalScrollBar()->height() + 15 );
}

void MainWindow::needAllChararcters(bool needAll)
{
    if(needAll){
        QString text = ui->lineEdit->text().trimmed();
        QStringList characters;
        for(int i=0;i<text.size();i++)
            characters << text.at(i);
        characters.removeDuplicates();

        for(int i=0;i<ui->tableWidget->rowCount();i++){
            for(int j=0;j<ui->tableWidget->columnCount();j++){
                QTableWidgetItem *item =  ui->tableWidget->item(i,j);
                if(item){
                    bool foundAll = true;
                    QRawFont rawFont = QRawFont::fromFont(item->font());
                    for(int i=0;i<characters.size();i++){
                        if(!rawFont.supportsCharacter(characters.at(i).at(0))){
                            foundAll = false;
                            break;
                        }
                    }
                    if(!foundAll)
                        item->setForeground(Qt::red);
                    else
                        item->setForeground(Qt::black);
                }
            }
        }
        QTreeWidgetItem *root = ui->treeWidget->invisibleRootItem();
        for(int i=0;i<root->childCount();i++){
            QTreeWidgetItem *item = root->child(i);
            bool foundAll = true;
            if(item->childCount()==0)
                continue;
            QRawFont rawFont = QRawFont::fromFont(item->child(0)->font(1));
            for(int i=0;i<characters.size();i++){
                if(!rawFont.supportsCharacter(characters.at(i).at(0))){
                    foundAll = false;
                    break;
                }
            }
            if(!foundAll){
                item->setForeground(0,Qt::red);
                for(int j=0;j<item->childCount();j++){
                    item->child(j)->setForeground(0,Qt::red);
                    item->child(j)->setForeground(1,Qt::red);
                }
            }
            else{
                item->setForeground(0,Qt::black);
                for(int j=0;j<item->childCount();j++){
                    item->child(j)->setForeground(0,Qt::black);
                    item->child(j)->setForeground(1,Qt::black);
                }
            }
        }
    }else{
        for(int i=0;i<ui->tableWidget->rowCount();i++){
            for(int j=0;j<ui->tableWidget->columnCount();j++){
                QTableWidgetItem *item =  ui->tableWidget->item(i,j);
                if(item){
                    item->setForeground(Qt::black);
                }
            }
        }
        QTreeWidgetItem *root = ui->treeWidget->invisibleRootItem();
        for(int i=0;i<root->childCount();i++){
            QTreeWidgetItem *item = root->child(i);
            item->setForeground(0,Qt::black);
            for(int j=0;j<item->childCount();j++){
                item->child(j)->setForeground(0,Qt::black);
                item->child(j)->setForeground(1,Qt::black);
            }
        }
    }
}

void MainWindow::compareItem(QTreeWidgetItem *item)
{
    item->setCheckState(0,Qt::PartiallyChecked);

    QString file = item->data(1,Qt::UserRole).toString();
    QString family = item->data(1,Qt::UserRole+1).toString();
    QString style = item->data(1,Qt::UserRole+2).toString();

    QString fontSignature = family + " - " + style;
    if(listCompare.contains(fontSignature))
        return;

    QFont font = item->font(1);

    QTreeWidgetItem *familyItem = new QTreeWidgetItem(ui->compareTreeWidget);
    familyItem->setCheckState(0,Qt::Unchecked);

    familyItem->setText(0, family);
    familyItem->setData(0,Qt::UserRole,file);
    familyItem->setData(0,Qt::UserRole+1,family);
    familyItem->setData(0,Qt::UserRole+2,style);
    familyItem->setExpanded(true);

    QString text = ui->lineEdit->text();

    QTreeWidgetItem *styleItem = new QTreeWidgetItem(familyItem);
    styleItem->setText(0, style);
    styleItem->setText(1, text);
    styleItem->setFont(1, font);
    styleItem->setData(1,Qt::UserRole,file);
    styleItem->setData(1,Qt::UserRole+1,family);
    styleItem->setData(1,Qt::UserRole+2,style);

    listCompare << fontSignature;
}

void MainWindow::compareItem(QTableWidgetItem *item)
{
    item->setCheckState(Qt::PartiallyChecked);

    QString file = item->data(Qt::UserRole).toString();
    QString family = item->data(Qt::UserRole+1).toString();
    QString style = item->data(Qt::UserRole+2).toString();

    QString fontSignature = family + " - " + style;
    if(listCompare.contains(fontSignature))
        return;

    QFont font = item->font();

    QTreeWidgetItem *familyItem = new QTreeWidgetItem(ui->compareTreeWidget);
    familyItem->setCheckState(0,Qt::Unchecked);

    familyItem->setText(0, family);
    familyItem->setData(0,Qt::UserRole,file);
    familyItem->setData(0,Qt::UserRole+1,family);
    familyItem->setData(0,Qt::UserRole+2,style);
    familyItem->setExpanded(true);

    QString text = ui->lineEdit->text();

    QTreeWidgetItem *styleItem = new QTreeWidgetItem(familyItem);
    styleItem->setText(0, style);
    styleItem->setText(1, text);
    styleItem->setFont(1, font);
    styleItem->setData(1,Qt::UserRole,file);
    styleItem->setData(1,Qt::UserRole+1,family);
    styleItem->setData(1,Qt::UserRole+2,style);

    listCompare << fontSignature;
}

void MainWindow::compareSelected()
{
    int nbChecked = 0;
    if(ui->radioLine->isChecked()){
        QTreeWidgetItem *root = ui->treeWidget->invisibleRootItem();
        int count = root->childCount();
        for(int i=0;i<count;i++){
            for(int j=0;j<root->child(i)->childCount();j++){
                QTreeWidgetItem *item = root->child(i)->child(j);
                if(item->checkState(0)==Qt::Checked){
                    compareItem(item);
                    nbChecked++;
                }
            }
        }
    }else{
        for(int i=0;i<ui->tableWidget->rowCount();i++){
            for(int j=0;j<ui->tableWidget->columnCount();j++){
                QTableWidgetItem *item = ui->tableWidget->item(i,j);
                if(item){
                    if(item->checkState()==Qt::Checked){
                        compareItem(item);
                        nbChecked++;
                    }
                }
            }
        }
    }

    if(nbChecked==0){
        //Add current selection
        if(ui->radioGrid->isChecked()){
            QList<QTableWidgetItem *> list = ui->tableWidget->selectedItems();
            if(list.size()==1){
                compareItem(list[0]);
            }
        }else{
            QList<QTreeWidgetItem *> list = ui->treeWidget->selectedItems();
            if(list.size()==1){
                if(list[0]->childCount()!=0){
                    compareItem(list[0]->child(0));
                }else{
                    compareItem(list[0]);
                }
            }
        }
    }

    ui->compareTreeWidget->resizeColumnToContents(0);
    ui->compareTreeWidget->resizeColumnToContents(1);

    int nbCompare = ui->compareTreeWidget->invisibleRootItem()->childCount();
    if(nbCompare>0){
        ui->tabWidget->setTabText(1,tr("Compare (%1)").arg(nbCompare));
    }else{
        ui->tabWidget->setTabText(1,tr("Compare"));
    }
}

void MainWindow::selectionChanged()
{
    QString fileName;
    QFont font;
    if(ui->radioGrid->isChecked()){
        QList<QTableWidgetItem *> list = ui->tableWidget->selectedItems();
        if(list.size()==1){
            font = list[0]->font();
            fileName = list[0]->data(Qt::UserRole).toString();
        }
    }else{
        QList<QTreeWidgetItem *> list = ui->treeWidget->selectedItems();
        if(list.size()==1){
            if(list[0]->childCount()!=0){
                font = list[0]->child(0)->font(1);
                fileName = list[0]->child(0)->data(1,Qt::UserRole).toString();
            }else{
                font = list[0]->font(1);
                fileName = list[0]->data(1,Qt::UserRole).toString();
            }
        }
    }
    font.setPointSize( ui->comboSampleSize->currentIndex() + 7 );
    ui->sample->setFont( font );

    if(font.styleName().isEmpty())
        ui->fontNameLabel->setText(font.family());
    else
        ui->fontNameLabel->setText(font.family() + " - " + font.styleName());

    if(!fileName.isEmpty()){
        if(QFile::exists(fileName)){
            ui->installFontButton->setEnabled(true);
        }else{
            ui->installFontButton->setDisabled(true);
        }
    }
    selectedFileName = fileName;

    QCoreApplication::processEvents();

    ui->scrollSample->setFixedHeight( ui->sample->sizeHint().height() + ui->scrollSample->horizontalScrollBar()->height() + 15 );
}

void MainWindow::tabChanged(int index)
{
    if(index==1){
        QTreeWidgetItem *root = ui->compareTreeWidget->invisibleRootItem();
        for(int i=0;i<root->childCount();i++){
            QTreeWidgetItem *item = root->child(i);
            QString filename = item->data(0,Qt::UserRole).toString();
            if(!filename.isEmpty()){
                QFontDatabase::addApplicationFont(filename);
            }
            for(int j=0;j<item->childCount();j++)
                item->child(j)->setText(1,ui->lineEdit->text());
        }
    }
}

#ifndef __WIN32__
void MainWindow::updateSystemFontList()
{
    this->setCursor(Qt::WaitCursor);
    QProcess::execute("fc-cache -s -f");
    this->setCursor(Qt::ArrowCursor);
}
#endif

void MainWindow::pressAbout()
{
    QMessageBox::information(this,tr("About"), tr("Written by %1 (%2)\nVersion: %3","author, year, version").arg(QString::fromUtf8("Léo Baudouin"),"2014",VERSION));
}
