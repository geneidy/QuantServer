#include "QuantServerUI.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

StartStopBtnWidget::StartStopBtnWidget(QWidget *parent)
    : QWidget(parent) {
        
    QVBoxLayout *vbox = new QVBoxLayout(this);
    QHBoxLayout *hbox = new QHBoxLayout();
    
    startBtn = new QPushButton("Start", this);
    stopBtn = new QPushButton("Stop", this);
    pauseBtn = new QPushButton("Paused", this);
    
    hbox->addWidget(startBtn, 1, Qt::AlignRight);
    hbox->addWidget(stopBtn, 0);
    hbox->addWidget(pauseBtn, 0);
    
    vbox->addStretch(1);
    vbox->addLayout(hbox);
    
    connect(startBtn, SIGNAL(clicked()), this, SLOT(startServer()));
}

void StartStopBtnWidget::startServer() {
    ui_settings.start_stop_pause = 1;
}
