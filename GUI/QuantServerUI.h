#pragma once

#include <QWidget>
#include <QPushButton>
#include "../Include/Settings.h"


class StartStopBtnWidget : public QWidget {
    
    Q_OBJECT
    
public:
    StartStopBtnWidget(QWidget *parent = 0);
    SETTINGS ui_settings;
    
public slots:
    void startServer();
    
private:
    QPushButton *startBtn;
    QPushButton *stopBtn;
    QPushButton *pauseBtn;
};
