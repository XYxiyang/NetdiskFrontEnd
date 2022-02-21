#ifndef DIALOG_REGIST_H
#define DIALOG_REGIST_H

#include <QDialog>

namespace Ui {
class Dialog_regist;
}

class Dialog_regist : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_regist(QWidget *parent = nullptr);
    ~Dialog_regist();
    bool check_patern(std::string in);

private slots:

    void on_finish_clicked();

private:
    Ui::Dialog_regist *ui;
};

#endif // DIALOG_REGIST_H
