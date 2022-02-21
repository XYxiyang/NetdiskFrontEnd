#ifndef DIALOG_DOWNLOAD_H
#define DIALOG_DOWNLOAD_H

#include <QDialog>

namespace Ui {
class Dialog_download;
}

class Dialog_download : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_download(QWidget *parent = nullptr);
    ~Dialog_download();

private:
    Ui::Dialog_download *ui;
};

#endif // DIALOG_DOWNLOAD_H
