#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#endif // COMMUNICATION_H
#include <iostream>
#include <string>
#include <winsock2.h>
#include<QDebug>
#include<fstream>
#include<errno.h>
using namespace std;

#define myOK 0
#define myERROR -1

/* 信息發送的操作符 */
#define INITIAL_CLIENT 1
#define INITIAL_SERVER 17
#define SEND 2 // 询问要不要发这个文件
#define SEND_FILE 3 // 发文件（真的文件）
#define REMOVE 4
#define RENAME 5
#define CHANGE 6 // 修改
#define SURE_GET 7 // 确认要收这个文件
#define NOT_GET 20 // 不真正接收此文件
#define EXIST 21 // 初始化比对时，完全相同
#define BIND_DIR 8 // 綁定目錄
#define RM_BIND_DIR 9 // 目錄解綁
#define FINISH 10 // 完成
#define LOGIN 11
#define LOGOUT 12
#define REGIST 13
#define FINISH_INITIAL 14
#define SENDCONFIG 15 // 发送记录目录的配置文件内容
#define GETCONFIG 16 // 索取初始化目录结构
/* 处理未完成的异常事件 */
#define PROCSEXCP 18
/* 开始同步状态 */
#define NORMAL 19
/* 初始等待询问状态 */
#define INITIAL_CONSULT 22
/* 信息發送字符串的信息 */
#define msgno_begin 0 // string中对应的位置
#define op_begin 1
#define flagtail_begin 2
#define flagfile_begin 3
#define SENDFILESIZE 1024 // 每次发送的文件大小
#define SENDSIZE 2048
#define usercorrect_begin 2 // 判断用户账号密码是否合法，包括注册、登录时都要用到
#define MAXMESSAGE 128
// 文件名，路径，md5码之间用\t作为分割，因为长度不定（呜呜
// 如果是传输文件，后面还跟文件内容
struct netdisk_message{
    int no; // 事件编号，用来确认是不是完成了,或者确认要不要真正传输文件,1bit
    int op; // 操作码 1bit
    boolean is_tail; // 是不是传输文件的最后一个部分
    boolean is_file; // true表示是文件
    string md5; // md5码
    string path; // 路径（不包括文件名
    string filename; // 文件名
    string content; // 内容
    string username;
    string userid;
    string passwd;
    boolean user_correct;
    netdisk_message(){
        ;
    }
    netdisk_message(int no,int op,string filename,boolean is_file,string path,string md5,string content,string username,string userid,string passwd,boolean user_correct,boolean is_tail){
        this->no=no;
        this->op=op;
        this->filename=filename;
        this->is_file=is_file;
        this->md5=md5;
        this->content=content;
        this->path=path;
        this->username=username;
        this->userid=userid;
        this->passwd=passwd;
        this->user_correct=user_correct;
        this->is_tail=is_tail;
    }
};

// 负责联网和通信，使用 Communication(string ip,string port); 初始化，然后调用connect连接
class Communication {
private:
    boolean message_count_use[MAXMESSAGE]; //标记该消息号是否用过
    string ip;
    int port;
    boolean ConnectError;

    string message_to_string(netdisk_message & msg);
    netdisk_message string_to_message(string &msg);
public:
    SOCKET sclient;//pritvate
    // 與服務端鏈接是否錯誤
    boolean connecterror();
    // 发送配置文件
    int send_configmessage(int op,string filename,string content,int no=-1);
    // 發送信息
    int send_message(int op,string filename,boolean is_file,string path="",string md5="",string content="",int no=-1,boolean is_tail=false);
    // 发送用户登录、登出、注册信息
    int send_usermessage(int op,string username,string useri,string passwd,int no=-1);   
    // 初始化類類型，调用完请接着调用connect函数来连接服务器
    Communication(string ip,int port);
    // 鏈接服務端
    int connection();
    // 斷開服務器鏈接
    int disconnection();
    ~Communication();
    // 接受來自服務端的信息，並返回到recv_content
    int recv_message(netdisk_message &recv_content);
    // 中途断网，尝试60s重连，成功返回ok
    int REconnection();
    
};
void print(netdisk_message& msg);
