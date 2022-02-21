 #include "netdisk.h"
#include "ui_netdisk.h"
#include "dialog_folder.h"
#include "dialog_regist.h"
#include "communication.h"
#include "fileactions.h"
#include "MD5.h"

extern Communication com;
extern netdisk_message msg;
extern string clientname;
extern string uploading_file;
extern string downloading_file;
extern string logfile;
netdisk::netdisk(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::netdisk)
{
    ui->setupUi(this);
}

netdisk::~netdisk()
{
    delete ui;
}


void netdisk::on_login_clicked()
{
    ofstream out;
    out.open("./tempfile",ios::out);
    out<<ui->password->toPlainText().toStdString();
    out.close();
    ifstream of("./tempfile",ios::in|ios::binary);
    MD5 Md5(of);
    string MD5passwd=Md5.toString();
    of.close();
    rmdir("./tempfile");
    clientname=ui->username->toPlainText().toStdString();
    //qDebug()<<MD5passwd.c_str()<<endl;
    MakeSureDirectoryPathExists(("C:\\mycloud\\"+clientname+"\\"+clientname+"-root\\").c_str());
    com.send_usermessage(LOGIN,ui->username->toPlainText().toStdString(),
                                   ui->username->toPlainText().toStdString(),
                                    MD5passwd);
    //qDebug()<<msg.user_correct<<endl;
    com.recv_message(msg);

    if(msg.user_correct==false){
        ui->show->setText("密码或账号错误");
        //qDebug()<<msg.user_correct<<"wrong"<<endl;
        return;
    }
    else
        ui->show->clear();
    //qDebug()<<"ok"<<endl;
    logfile=file1+clientname+"\\"+"run.log";

    com.recv_message(msg);
    if(msg.op==SENDCONFIG){
        ofstream out;
        downloading_file="usrconfig.conf";
        out.open("C:\\mycloud\\"+clientname+"\\usrconfig.conf",ios::out);
        while(1){
            //com.recv_message(msg);
            out<<msg.content;
            if(msg.is_tail==true){
                com.send_message(FINISH,"",false,"","","");
                break;
            }
        }
        out.close();
        downloading_file.clear();
    }
    else{
        string filename="C:\\mycloud\\"+clientname+"\\usrconfig.conf";
        string md5code=makesureConfigexist();
        string file;
        ifstream fin(filename.c_str(),ios::binary);//
        int count=0;
        char temp_ch;
        uploading_file="usrconfig.conf";
        while (1)
        {
            if(!fin.get(temp_ch)){
                com.send_message(SENDCONFIG,filename,true,"",md5code,file,-1,true);

                file.clear();
                com.recv_message(msg);
                if(msg.op==FINISH){

                    break;
                }
                else{
                    break;
                }
            }
            count++;
            file += temp_ch;
            if(count==SENDFILESIZE){
                count=0;
                com.send_message(SENDCONFIG,filename,true,"",md5code,file);
                file.clear();
                com.recv_message(msg);
                if(msg.op==FINISH)
                    continue;
            }
        }
        uploading_file.clear();

    }
    this->close();

    //RemoveDirectoryA(("C:\\mycloud\\"+clientname).c_str());
    createFoldersbyFile("C:\\mycloud\\"+clientname+"\\usrconfig.conf","C:\\mycloud\\"+clientname);
    openFile(("C:\\mycloud\\"+clientname+"\\usrconfig.conf").c_str(),
             ("C:\\mycloud\\"+clientname+"\\").length(),CHECKMD5);
    com.send_message(FINISH_INITIAL,"",false,"","","");

    receiveFiles();
    startMonitor("C:\\mycloud\\"+clientname+"\\usrconfig.conf");
    
    Dialog_folder df;
    df.show();
    df.exec();
    this->show();
}

void netdisk::on_regist_clicked()
{
    this->close();
    Dialog_regist dr;
    dr.show();
    dr.exec();
    this->show();
}
