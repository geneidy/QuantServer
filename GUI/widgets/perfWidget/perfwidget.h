#ifndef PERFWIDGET_H
#define PERFWIDGET_H

#include <QWidget>
namespace Ui {
    class PerfWidget;
}

class PerfWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PerfWidget(QWidget *parent = 0);
    ~PerfWidget();
private slots:
    void printPerc();

private:
    Ui::PerfWidget *ui;

};

#endif // PERFWIDGET_H
