#pragma once

#include <QWidget>
#include <QPushButton>
#include "../Include/NQTV.h"


class StartStopBtnWidget : public QWidget {
    
    Q_OBJECT
    
public:
    StartStopBtnWidget(QWidget *parent = 0);
    
public slots:
    void setValue(int value);
    
private:
    QPushButton *startBtn;
    QPushButton *stopBtn;
    QPushButton *pauseBtn;
};
