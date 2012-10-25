#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
   : QMainWindow(parent)
{
    setupUi(this);

    connect(pushButton,SIGNAL(clicked()),this,SLOT(showFont()));
    connect(lineEdit,SIGNAL(returnPressed()),this,SLOT(showFont()));
}

void MainWindow::showFont()
{
  QString text = lineEdit->text().trimmed();

  QWidget *w = new QWidget;
  QGridLayout *layout = new QGridLayout;
  w->setLayout(layout);
  scrollArea->setWidget(w);

  this->setCursor(Qt::WaitCursor);

    QFontDatabase database;
    QStringList families = database.families();
    foreach(QString family, families){
        QStringList styles = database.styles(family);
        foreach(QString style, styles){
            if(style.isEmpty() || style.contains("oblique",Qt::CaseInsensitive))
                continue;

            int weight = database.weight(family,style);
            bool italic = database.italic(family,style);
            bool bold = database.bold(family,style);

            QLabel *label = new QLabel(text);
            QFont f = QFont(family, 32, weight, italic);
            f.setBold(bold);
            label->setFont(f);

            QLabel *fontname = new QLabel(QString("%1 %2").arg(family).arg(style));


            int n = layout->rowCount();
            layout->addWidget(fontname,n,0);
            layout->addWidget(label,n,1);
          }
    }

    this->setCursor(Qt::ArrowCursor);
}
