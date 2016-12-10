#include "resourcesWidget.h"

ResourcesWidget::ResourcesWidget(QWidget* child)
    : QWidget(child)
{
    ui.setupUi(this);
    
    resourcesThread = new resourcesWorker(this);
    
    resourcesThread->start();
}    
