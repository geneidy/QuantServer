#include <QApplication>
//#include <QString>

#include "QtxGui.h"

int main(int argc, char **argv) {
 
  QApplication a(argc, argv);
  
  QtxGui app;
  app.show();
  return a.exec();    
}
