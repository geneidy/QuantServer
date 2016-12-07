#include "perfwidget.h"
#include "ui_perfwidget.h"

#include <stdio.h>
#include <iostream>
#include <QTimer>
#include <QDebug>

PerfWidget::PerfWidget(QWidget *child) :
    QWidget(child),
    ui(new Ui::PerfWidget)
{
    ui->setupUi(this);
    ui->widget->setSpeed(PerformanceWidget::FAST_UPDATE);
    ui->widget->setDisplayMode(CpuWidget::DRAW_GRAPH);

//        std::cout<<sysconf( _SC_PHYS_PAGES) * sysconf( _SC_PAGESIZE )<<std::endl;


}

PerfWidget::~PerfWidget()
{
    delete ui;
}


void PerfWidget::printPerc()
{


    ui->widget->repaint();

}


