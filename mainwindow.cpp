#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), needToUpdateText(false)
{
    ui->setupUi(this);

    ui->treeWidget->hide();

    connect(ui->lineEdit,SIGNAL(returnPressed()),this,SLOT(changeText()));
    connect(ui->buttonFolder,SIGNAL(clicked()),this,SLOT(selectFolder()));
    connect(ui->buttonDefault,SIGNAL(clicked()),this,SLOT(loadDefaultFont()));

    connect(ui->compareButton,SIGNAL(clicked()),this,SLOT(compareSelected()));

    connect(ui->tableWidget,SIGNAL(itemSelectionChanged()),this,SLOT(selectionChanged()));
    connect(ui->treeWidget,SIGNAL(itemSelectionChanged()),this,SLOT(selectionChanged()));

    updateFontCount(0);
    nbCols = 10;

    this->setMinimumSize(640,480);

    //Display mode
    QButtonGroup *buttonGroup = new QButtonGroup;
    buttonGroup->addButton(ui->radioGrid,0);
    buttonGroup->addButton(ui->radioLine,1);
    buttonGroup->setExclusive(true);
    connect(buttonGroup,SIGNAL(buttonClicked(int)),this,SLOT(changeDisplay(int)));

    //Options
    ui->optionsWidget->hide();
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
    connect(ui->editNeed,SIGNAL(textChanged(QString)),this,SLOT(textNeededChanded(QString)));

    //Sample
    QFont font = ui->sample->font();
    if(font.styleName().isEmpty())
        ui->fontNameLabel->setText(font.family());
    else
        ui->fontNameLabel->setText(font.family() + " - " + font.styleName());

    font.setPointSize(35);
    ui->sample->setFont(font);
    ui->sample->setText(ui->lineEdit->text());

    ui->installFontButton->setDisabled(true);

    connect(ui->buttonOptions,SIGNAL(toggled(bool)),ui->optionsWidget,SLOT(setVisible(bool)));
    connect(ui->installFontButton,SIGNAL(clicked()),this,SLOT(installOneFont()));
    connect(ui->installButton,SIGNAL(clicked()),this,SLOT(install()));
    connect(ui->removeButton,SIGNAL(clicked()),this,SLOT(remove()));
    connect(ui->buttonQuit,SIGNAL(clicked()),this,SLOT(close()));

    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(tabChanged(int)));
}

MainWindow::~MainWindow()
{
}

void MainWindow::changeDisplay(int)
{
    if(ui->radioGrid->isChecked()){
        ui->tableWidget->show();
        ui->treeWidget->hide();
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

void MainWindow::changeText()
{
     if(ui->radioLine->isChecked()){
        applyText();
     }else{
         needToUpdateText = true;
     }
}

void MainWindow::applyText()
{
    QString text = ui->lineEdit->text().trimmed();
    needToUpdateText = false;
    QTreeWidgetItem *root = ui->treeWidget->invisibleRootItem();
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
    QString defaultPath = "./Fonts/";
    QDir defaultDir(defaultPath);
    if(!defaultDir.exists())
        defaultDir.setPath("~/Documents/Fonts/");
    if(!defaultDir.exists())
        defaultPath = QDir::homePath();
    QString dirpath = QFileDialog::getExistingDirectory(this, tr("Open Directory"), defaultPath, QFileDialog::ShowDirsOnly);

    QCoreApplication::processEvents();

    openFolder(dirpath);

}
/*
void MainWindow::openFolder(QString path)
{
    QTime time;
    time.restart();

    QDir dir(path);
    QStringList listFiles = dir.entryList(QStringList() << "*.ttf" << "*.TTF" << "*.otf" << "*.OTF");

    QMap< FontFamilyInfo,QList<FontStyleInfo> > map;

    QThreadPool *pool = new QThreadPool(this);
    pool->setMaxThreadCount( QThread::idealThreadCount() );

    for(int i=0;i<listFiles.size();i++){
        QString file = dir.absoluteFilePath(listFiles.at(i));

        FontLoader *loader = new FontLoader(file,&map);
        pool->start(loader,1);
    }

    pool->waitForDone();

    qDebug() << "Load: " << time.elapsed() << map.size();
    time.restart();

    ui->treeWidget->clear();
    ui->tableWidget->clear();

    QList<QTableWidgetItem*> tableItemList;

    QString text = ui->lineEdit->text();

    int nb = 0;

    QMapIterator< FontFamilyInfo,QList<FontStyleInfo> > iterator(map);
    while (iterator.hasNext()) {
        iterator.next();
        QTreeWidgetItem *familyItem = new QTreeWidgetItem(ui->treeWidget);
        familyItem->setText(0, iterator.key().family);
        familyItem->setExpanded(true);
        familyItem->setCheckState(0,Qt::Unchecked);
        familyItem->setData(0,Qt::UserRole,iterator.key().file);

        QListIterator< FontStyleInfo > listIterator(iterator.value());
        while (listIterator.hasNext()) {
            FontStyleInfo styleInfo = listIterator.next();
            QTreeWidgetItem *styleItem = new QTreeWidgetItem(familyItem);
            styleItem->setStatusTip(1,iterator.key().family);
            styleItem->setToolTip(1,iterator.key().family);
            styleItem->setText(0, styleInfo.style);
            styleItem->setText(1, text);
            styleItem->setFont(1, styleInfo.font);


            QTableWidgetItem *tableItem = new QTableWidgetItem("Aa");
            tableItem->setStatusTip(iterator.key().family);
            tableItem->setToolTip(iterator.key().family);
            tableItem->setCheckState(Qt::Unchecked);
            tableItem->setData(Qt::UserRole,iterator.key().file);
            tableItem->setFont(styleInfo.font);
            tableItemList << tableItem;

            nb++;
        }

        ui->treeWidget->resizeColumnToContents(0);
        ui->treeWidget->resizeColumnToContents(1);
        ui->tableWidget->resizeColumnsToContents();
        ui->tableWidget->resizeRowsToContents();

    }

    ui->tableWidget->setColumnCount(nbCols);
    ui->tableWidget->setRowCount(nb/nbCols+1);

    for(int i=0;i<tableItemList.size();i++){
        ui->tableWidget->setItem(i/nbCols,i%nbCols,tableItemList.at(i));
    }

    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();

    qDebug() << "Display: " << time.elapsed() << nb;

    return;


}
*/


void MainWindow::openFolder(QString path)
{
    QFontDatabase::removeAllApplicationFonts();

    if(path.isEmpty())
        return;

    QTime time;
    time.restart();
    int nb = 0;

    QDir dir(path);
    if(!dir.exists())
        return;

    this->setCursor( Qt::WaitCursor );

    QStringList listFiles = dir.entryList(QStringList() << "*.ttf" << "*.TTF" << "*.otf" << "*.OTF");


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

    QList<FontFileInfo> fontFilesInfo;

    for(int i=0;i<listFiles.size();i++){
        QTime t;
        t.restart();

        QString file = dir.absoluteFilePath(listFiles.at(i));

        int id = QFontDatabase::addApplicationFont( file );
        QStringList families = QFontDatabase::applicationFontFamilies( id );

        FontFileInfo fontFileInfo;
        fontFileInfo.file = file;

        foreach (const QString &family, families)  {
           /*QTreeWidgetItem *familyItem = new QTreeWidgetItem(ui->treeWidget);
           familyItem->setText(0, family);
           familyItem->setExpanded(true);
           familyItem->setCheckState(0,Qt::Unchecked);
           familyItem->setData(0,Qt::UserRole,file);*/

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

               QFont font(family, 32, weight, italic);
               font.setBold(bold);


               /*QTreeWidgetItem *styleItem = new QTreeWidgetItem(familyItem);
               styleItem->setText(0, style);
               styleItem->setText(1, text);
               styleItem->setFont(1, font);
               styleItem->setStatusTip(1,family);
               styleItem->setToolTip(1,family);
               styleItem->setData(0,Qt::UserRole,file);*/

               /*QTableWidgetItem *tableItem = new QTableWidgetItem("Aa");
               tableItem->setStatusTip(family);
               tableItem->setToolTip(family);
               tableItem->setCheckState(Qt::Unchecked);
               tableItem->setData(Qt::UserRole,file);
               tableItem->setToolTip(family);
               tableItem->setFont(font);

               tableItemList << tableItem;

               ui->tableWidget->setRowCount(nb/nbCols+1);
               ui->tableWidget->setItem(nb/nbCols,nb%nbCols,tableItem);*/

               nb++;

               QTableWidgetItem *tableItem = new QTableWidgetItem("Aa");
               tableItem->setStatusTip(family);
               tableItem->setToolTip(family);
               tableItem->setCheckState(Qt::Unchecked);
               tableItem->setData(Qt::UserRole,file);
               tableItem->setToolTip(family);
               tableItem->setFont(font);

               ui->tableWidget->setItem(0,0,tableItem);
               ui->tableWidget->resizeColumnsToContents();
               ui->tableWidget->resizeRowsToContents();


               FontStyleInfo fsi;
               fsi.style = style;
               fsi.font = font;
               //fsi.item = tableItem;
               fsi.width = ui->tableWidget->visualItemRect(tableItem).width();

               if(ui->radioGrid->isChecked())
                QCoreApplication::processEvents();

               ui->tableWidget->takeItem(0,0);

               fontStyleInfo.push_back(fsi);
           }

           fontFamilyInfo.styles = fontStyleInfo;
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

    qDebug() << "Loading time: " << time.elapsed() << nb;

    QTime displayTime;
    displayTime.restart();

    QList<int> sizes;
    for(int i=0;i<fontFilesInfo.size();i++){
        for(int j=0;j<fontFilesInfo.at(i).families.size();j++){
            for(int k=0;k<fontFilesInfo.at(i).families.at(j).styles.size();k++){
                sizes << fontFilesInfo.at(i).families.at(j).styles.at(k).width;
            }
        }
    }

    nbCols = findNbCol(sizes,ui->tableWidget->width());


    QProgressDialog * progressDisplay = new QProgressDialog(tr("Displaying"),tr("Cancel"),0,nb-1,this);
    progressDisplay->setWindowTitle( this->windowTitle() );
    progressDisplay->setModal(true);
    progressDisplay->show();
    QCoreApplication::processEvents();

    ui->tableWidget->setColumnCount(nbCols);
    ui->tableWidget->setRowCount(nb/nbCols + (nb%nbCols==0?0:1));

    nb = 0;
    for(int i=0;i<fontFilesInfo.size();i++){

        QString file = fontFilesInfo.at(i).file;
        fontFilesInfo[i].id = QFontDatabase::addApplicationFont( file );

        for(int j=0;j<fontFilesInfo.at(i).families.size();j++){

            QString family = fontFilesInfo.at(i).families.at(j).family;

            QTreeWidgetItem *familyItem = new QTreeWidgetItem(ui->treeWidget);
            familyItem->setText(0, family);
            familyItem->setExpanded(true);
            familyItem->setCheckState(0,Qt::Unchecked);
            familyItem->setData(0,Qt::UserRole,file);

            for(int k=0;k<fontFilesInfo.at(i).families.at(j).styles.size();k++){

                QFont font = fontFilesInfo.at(i).families.at(j).styles.at(k).font;
                QString style = fontFilesInfo.at(i).families.at(j).styles.at(k).style;

                QTreeWidgetItem *styleItem = new QTreeWidgetItem(familyItem);
                styleItem->setText(0, style);
                styleItem->setText(1, text);
                styleItem->setFont(1, font);
                styleItem->setStatusTip(1,family);
                styleItem->setToolTip(1,family);
                styleItem->setData(0,Qt::UserRole,file);

                QTableWidgetItem *tableItem = new QTableWidgetItem("Aa");
                tableItem->setCheckState(Qt::Unchecked);
                tableItem->setData(Qt::UserRole,file);
                tableItem->setData(Qt::UserRole+1,family);
                tableItem->setData(Qt::UserRole+2,style);
                tableItem->setStatusTip(family);
                tableItem->setToolTip(family);
                tableItem->setFont(font);
                ui->tableWidget->setItem(nb/nbCols,nb%nbCols,tableItem);

                //ui->tableWidget->setItem(nb/nbCols,nb%nbCols,fontFilesInfo.at(i).families.at(j).styles.at(k).item);
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


    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
    ui->treeWidget->resizeColumnToContents(0);
    ui->treeWidget->resizeColumnToContents(1);

    ui->tableWidget->item(0,0)->setSelected(true);
    ui->treeWidget->invisibleRootItem()->child(0)->setSelected(true);

    QCoreApplication::processEvents();

    qDebug() << "Display time: " << displayTime.elapsed() << nb;
    qDebug() << "Total time: " << time.elapsed();

    this->setCursor( Qt::ArrowCursor );
}


void MainWindow::resizeEventPerso()
{

}

void MainWindow::loadDefaultFont()
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
       familyItem->setCheckState(0,Qt::Unchecked);
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

           QFont font(family, 32, weight, italic);
           font.setBold(bold);

           styleItem->setText(1, text);
           styleItem->setFont(1, font);
           styleItem->setData(1,Qt::UserRole,"");
           styleItem->setData(1,Qt::UserRole+1,family);
           styleItem->setData(1,Qt::UserRole+2,style);

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

           sizes << ui->tableWidget->visualItemRect(tableItem).width();

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

    nbCols = findNbCol(sizes, ui->tableWidget->width());

    ui->tableWidget->clear();

    ui->tableWidget->setColumnCount(nbCols);
    ui->tableWidget->setRowCount((nb/nbCols)+1);

    for(int i=0;i<tableItemList.size();i++){
        ui->tableWidget->setItem(i/nbCols,i%nbCols,tableItemList.at(i));
    }

    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();

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
        if(c>50)
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
            if(!filename.isEmpty()){
                if(QFile::exists(filename)){
                    QDesktopServices::openUrl(QUrl("file:///" + filename, QUrl::TolerantMode));
                }
            }
        }
    }
}

void MainWindow::remove()
{
    QTreeWidgetItem *root = ui->compareTreeWidget->invisibleRootItem();
    for(int i=0;i<root->childCount();i++){
        QTreeWidgetItem *item = root->child(i);
        if(item->checkState(0)==Qt::Checked){
            delete item;
        }
    }
    ui->tabWidget->setTabText(1,tr("Compare (%1)").arg(ui->compareTreeWidget->invisibleRootItem()->childCount()));
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
  for(int i=0;i<ui->tableWidget->rowCount();i++){
      for(int j=0;j<ui->tableWidget->columnCount();j++){
          QTableWidgetItem *item = ui->tableWidget->item(i,j);
          if(item){
              QFont font = item->font();
              font.setPointSize(currentSize);
              item->setFont(font);
          }
      }
  }
}

void MainWindow::sampleSizeChanged(int index)
{
    QFont font = ui->sample->font();
    font.setPointSize(index + 7);
    ui->sample->setFont(font);
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

void MainWindow::textNeededChanded(QString text)
{

}

void MainWindow::compareSelected()
{
    if(ui->radioLine->isChecked()){
        QTreeWidgetItem *root = ui->treeWidget->invisibleRootItem();
        int count = root->childCount();
        for(int i=0;i<count;i++){
            QTreeWidgetItem *item = root->child(i);
            if(item->checkState(0)==Qt::Checked){
                qDebug() << item->data(0,Qt::UserRole).toString() << item->data(0,Qt::UserRole+1).toString() << item->data(0,Qt::UserRole+2).toString();
                //TODO
                item->setCheckState(0,Qt::Unchecked);

                QTreeWidgetItem *copyItem = item->clone();
                ui->compareTreeWidget->addTopLevelItem(copyItem);
                copyItem->setExpanded(true);

                //if(item->data(0,Qt::UserRole).toString().isEmpty())
                //    copyItem->setFlags(Qt::ItemIsEnabled);
            }
        }
    }else{
        for(int i=0;i<ui->tableWidget->rowCount();i++){
            for(int j=0;j<ui->tableWidget->columnCount();j++){
                QTableWidgetItem *item = ui->tableWidget->item(i,j);
                if(item){
                    if(item->checkState()==Qt::Checked){
                        qDebug() << item->data(Qt::UserRole).toString() << item->data(Qt::UserRole+1).toString() << item->data(Qt::UserRole+2).toString();
                        //TODO
                        item->setCheckState(Qt::Unchecked);

                        QString file = item->data(Qt::UserRole).toString();
                        QString family = item->data(Qt::UserRole+1).toString();
                        QString style = item->data(Qt::UserRole+2).toString();
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

                    }
                }
            }
        }
    }
    ui->tabWidget->setTabText(1,tr("Compare (%1)").arg(ui->compareTreeWidget->invisibleRootItem()->childCount()));
    ui->compareTreeWidget->resizeColumnToContents(0);
    ui->compareTreeWidget->resizeColumnToContents(1);
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
