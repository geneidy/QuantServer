#ifndef RESOURCESWIDGET_H
#define RESOURCESWIDGET_H

#include <QWidget>
#include "resourcesworker.h"
#include "ui_resourcesWidget.h"

class ResourcesWidget : public QWidget
{
    Q_OBJECT
    
public:
    ResourcesWidget(QWidget* parent = 0);
    Ui::ResourcesWidget ui;
    
private:
    resourcesWorker* resourcesThread;
};





















#endif // RESOURCESWIDGET_H
