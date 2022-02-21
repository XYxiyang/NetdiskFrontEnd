#include "dialog_download.h"
#include "ui_dialog_download.h"

Dialog_download::Dialog_download(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_download)
{
    ui->setupUi(this);
}

Dialog_download::~Dialog_download()
{
    delete ui;
}
