#ifndef QFONTLABEL_H
#define QFONTLABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QRawFont>

struct FontDisplay{
    QFont font;
    int size;
    QString name;
    QString file;
};

class QFontLabel : public QLabel
{
    Q_OBJECT
public:
    QFontLabel(QString text = "Aa", int size = 15) : QLabel(text){
        pointSize = size;
        this->setNewText(text);
        QFont currentFont = this->font();
        currentFont.setPointSize(pointSize);
        this->setFont(currentFont);
    }

    QFontLabel(QWidget *parent, QString text = "Aa", int size = 15) : QLabel(text,parent){
        pointSize = size;
        this->setNewText(text);
        QFont currentFont = this->font();
        currentFont.setPointSize(pointSize);
        this->setFont(currentFont);
    }
    void mousePressEvent(QMouseEvent *e){
        if(e->button()==Qt::RightButton)
            emit this->choose(font_);
        else
            emit this->selectFont(font_);
    }
    QString getFile(){
      return font_.file;
    }

public slots:
    void setNewFont(FontDisplay fontInfo){
        setNewFont(fontInfo.font);
        font_ = fontInfo;
        this->setStatusTip( fontInfo.name );
    }

    void setNewText(QString text){
        if(text.isEmpty()){
            setNewText("Aa");
        }else{
            this->setText(text);
        }
    }
    void setNewFont(QFont font){
        font.setPointSize(pointSize);
        this->setFont(font);
        rf_ = QRawFont::fromFont(font);
    }
    void setNewSize(int size){
        pointSize = size;
        QFont currentFont = this->font();
        currentFont.setPointSize(pointSize);
        this->setFont(currentFont);
    }
    void need(QString text){
      for(int i=0;i<text.size();i++){
        if(!rf_.supportsCharacter(text.at(i))){
          this->setVisible(false);
          return;
        }
      }
      this->setVisible(true);
    }

signals:
    void selectFont(FontDisplay);
    void choose(FontDisplay);

private:
    int pointSize;
    FontDisplay font_;
    QRawFont rf_;
};

#endif // QFONTLABEL_H
