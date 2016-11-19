#include <QApplication>
//#include <QString>

#include "QuantServer.h"

int main(int argc, char **argv) {
 
  QApplication a(argc, argv);
  
  QuantServer app;
  app.show();
  return a.exec();    
}
