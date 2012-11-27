#ifndef QMULTIPROGRESSWIDGET_H
#define QMULTIPROGRESSWIDGET_H

#include <QWidget>
#include <QProgressBar>
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>

class QMultiProgressWidget : public QWidget
{
    Q_OBJECT
public:
    QMultiProgressWidget(int nbProgressBar,QStringList text){
        this->setWindowTitle(tr("Progress..."));
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        for(int i=0;i<nbProgressBar;i++){
            QLabel *label = new QLabel( (i<text.size())?text.at(i):"");
            QProgressBar *progress = new QProgressBar;
            mainLayout->addWidget(label);
            mainLayout->addWidget(progress);
            labelList << label;
            progressList << progress;
        }
        reset();
    }
    void reset(){
        for(int i=0;i<progressList.size();i++){
            progressList[i]->setValue( 0 );
            progressList[i]->setMinimum( 0 );
            progressList[i]->setMaximum( 0 );
            labelList[i]->setVisible(true);
            progressList[i]->setVisible(true);
        }
    }
    void hideProgress(int id){
        if(id<labelList.size()){
            labelList[id]->setVisible(false);
            progressList[id]->setVisible(false);
        }
    }
    void showProgress(int id){
        if(id<labelList.size()){
            labelList[id]->setVisible(true);
            progressList[id]->setVisible(true);
        }
    }
    int value(int id){
        if(id<progressList.size())
            return progressList[id]->value();
        return -1;
    }

private:
    QList<QLabel*> labelList;
    QList<QProgressBar*> progressList;
public slots:
    void setText(int id, QString text){
        if(id<labelList.size())
            labelList[id]->setText( text );
    }
    void setMaximum(int id, int maximum){
        if(id<progressList.size())
            progressList[id]->setMaximum( maximum );
    }
    void setMinimum(int id, int minimum){
        if(id<progressList.size())
            progressList[id]->setMinimum( minimum );
    }
    void setValue(int id, int value){
        if(id<progressList.size())
            progressList[id]->setValue( value );
    }
};

#endif // QMULTIPROGRESSWIDGET_H
