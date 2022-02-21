#include "download.h"
#include "ui_download.h"

download::download(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::download)
{
    ui->setupUi(this);
}

download::~download()
{
    delete ui;
}
