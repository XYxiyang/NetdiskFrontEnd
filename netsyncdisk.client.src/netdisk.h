#ifndef NETDISK_H
#define NETDISK_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class netdisk; }
QT_END_NAMESPACE

class netdisk : public QMainWindow
{
    Q_OBJECT

public:
    netdisk(QWidget *parent = nullptr);
    ~netdisk();

private slots:


    void on_login_clicked();

    void on_regist_clicked();

private:
    Ui::netdisk *ui;
};
#endif // NETDISK_H
