#include "dialog_regist.h"
#include "ui_dialog_regist.h"
#include "communication.h"
#include "fileactions.h"
#include<QDebug>
#include"MD5.h"
extern Communication com;
extern netdisk_message msg;
extern string clientname;

Dialog_regist::Dialog_regist(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_regist)
{
    ui->setupUi(this);
}

Dialog_regist::~Dialog_regist()
{
    delete ui;
}

bool Dialog_regist::check_patern(string in){
    if(in.length()<8){
        ui->msg->setText("密码至少长度为8");
        return false;
    }
    bool types[5]={false};
    for(int i=0;i<(int)in.length();i++){
        if(in[i]<='z'&&in[i]>='a')
            types[0]=true;
        else if(in[i]<='Z'&&in[i]>='A')
            types[1]=true;
        else if(in[i]<='9'&&in[i]>='0')
            types[2]=true;
        else
            types[3]=true;
    }
    if(!(types[2]&&types[0])&&!(types[2]&&types[1])){
        ui->msg->setText("密码需要包含字母和数字");
        return false;
    }
    return true;
}

void Dialog_regist::on_finish_clicked()
{
    QString passwd1=ui->password1->toPlainText();
    QString passwd2=ui->password2->toPlainText();
    if(passwd1!=passwd2){
        ui->msg->setText("密码输入不一致");
        return;
    }

    if(!check_patern(passwd1.toStdString()))
        return ;
    netdisk_message msg;
    ui->msg->clear();
    ofstream out;
    out.open("./tempfile",ios::out);
    out<<passwd1.toStdString();
    out.close();
    ifstream of("./tempfile",ios::in|ios::binary);
    MD5 Md5(of);
    string MD5passwd=Md5.toString();
    of.close();
    rmdir("./tempfile");
    //qDebug()<<MD5passwd.c_str()<<endl;
    com.send_usermessage(REGIST,ui->username->toPlainText().toStdString(),
                         ui->account->toPlainText().toStdString(),
                         MD5passwd);

    com.recv_message(msg);
    print(msg);
    if(!msg.user_correct){
        ui->msg->setText("账号已经存在");
        return;
    }
    ui->msg->clear();
    if(msg.user_correct){
        ui->msg->setText("注册成功,3s后自动跳转");
    }

    Sleep(3000);
    this->close();

}
