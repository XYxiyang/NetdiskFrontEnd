#include "netdisk.h"

#include <QApplication>
#include"communication.h"

Communication com("10.60.102.252",20600);
netdisk_message msg;
string clientname;
string downloading_file;
string uploading_file;
string logfile;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    netdisk w;
    com.connection();
    w.show();
    return a.exec();
}
