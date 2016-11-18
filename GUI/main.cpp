#include <QApplication>
#include "QuantServerUI.h"

int main(int argc, char *argv[]) {
    
    QApplication app(argc, argv);
    
    StartStopBtnWidget window;
    
    window.resize(250, 150);
    window.setWindowTitle("StartStopBtnWidget");
    window.show();
    
    return app.exec();
}
