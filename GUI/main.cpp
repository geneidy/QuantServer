#include <QApplication>
//#include <QString>

#include "QuantServerMainWindow.h"

int main(int argc, char **argv) {
 
  QApplication a(argc, argv);
  
  QuantServerMainWindow app;
  app.show();
  return a.exec();    
}
