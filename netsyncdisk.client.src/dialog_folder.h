#ifndef DIALOG_FOLDER_H
#define DIALOG_FOLDER_H

#include <QDialog>
#define ADDPATH 1
#define DELETEPATH 2
#define BONDPATH 3
#define UNBONDPATH 4
namespace Ui {
class Dialog_folder;
}

class Dialog_folder : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_folder(QWidget *parent = nullptr);
    ~Dialog_folder();

private slots:
    void on_myfile_clicked();

    void on_link_clicked();

    void on_unlink_clicked();

    void on_addpath_clicked();

    void on_deletepath_clicked();

    void on_confirm_clicked();


    void on_filearray_clicked();

private:
    Ui::Dialog_folder *ui;
};

#endif // DIALOG_FOLDER_H
