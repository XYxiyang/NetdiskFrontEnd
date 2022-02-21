#include "./communication.h"
void print(netdisk_message& msg)
{
    qDebug() << "消息号 " << msg.no << endl;
    qDebug() << "操作符 " << msg.op << endl;
    qDebug() << "文件名 " << msg.filename.c_str() << endl;
    qDebug() << "路径 " << msg.path.c_str() <<endl;
    qDebug() << "是否文件 " << msg.is_file << endl;
    qDebug() << "是否文件结尾部分 " << msg.is_tail << endl;
    qDebug() << "是否文件内容 " << msg.content.c_str() << endl;
    qDebug() << "md5码 " << msg.md5.c_str() << endl;
    qDebug() << "用户名 " <<msg.username.c_str() <<endl;
    qDebug() << "用户账号 "<< msg.userid.c_str() <<endl;
    qDebug() << "用户密码 " << msg.passwd.c_str() << endl;
    qDebug() << "账号密码是否合法 " <<msg.user_correct << endl;
}
string Communication::message_to_string(netdisk_message & msg)
{
    string re;
    re+=char(msg.no);
    re+=char(msg.op);
    if(msg.op==LOGIN||msg.op==LOGOUT||msg.op==REGIST){
        re+=(char)(msg.user_correct+1);
        re+=msg.username;
        re+="\t";
        re+=msg.userid;
        re+="\t";
        re+=msg.passwd;
        re+='\t';
    }
    else if(msg.op!=SURE_GET&&msg.op!=NOT_GET){
        re+=(char)(msg.is_tail+1);
        re+=(char)(msg.is_file+1);
        re+=msg.filename;
        re+="\t";
        re+=msg.path;
        re+="\t";
        if(msg.op!=BIND_DIR&&msg.op!=RM_BIND_DIR){
            re+=msg.md5;
            re+="\t";
            if(msg.op==SEND_FILE||msg.op==SENDCONFIG || msg.op == INITIAL_CLIENT || msg.op == INITIAL_SERVER ){
                re+=msg.content;
                re+="\t";
            }
        }
    }
    //qDebug()<<"sendstr"<<re.c_str()<<endl;
    return re;
}

netdisk_message Communication::string_to_message(string &msg)
{
    netdisk_message re;
    re.no=msg[msgno_begin];
    re.op=msg[op_begin];
    if(re.op==LOGIN||re.op==LOGOUT||re.op==REGIST){
        re.user_correct=(msg[usercorrect_begin]-1);
        int pos=usercorrect_begin+1;
        while(msg[pos]!='\t'){
            re.username+=msg[pos];
            pos++;
        }
        pos++;
        while(msg[pos]!='\t'){
            re.userid+=msg[pos];
            pos++;
        }
        pos++;
        while(msg[pos]!='\t'){
            re.passwd+=msg[pos];
            pos++;
        }
    }
    else if(re.op!=SURE_GET&&re.op!=NOT_GET){
        re.is_tail=(msg[flagtail_begin]-1);
        re.is_file=(msg[flagfile_begin]-1);
        int pos=flagfile_begin+1;
        while(msg[pos]!='\t'){
            re.filename+=msg[pos];
            pos++;
        }
        pos++;
        while(msg[pos]!='\t'){
            re.path+=msg[pos];
            pos++;
        }
        if(re.op!=BIND_DIR&&re.op!=RM_BIND_DIR){
            pos++;
            while(msg[pos]!='\t'){
                re.md5+=msg[pos];
                pos++;
            }
            pos++;
            if(re.op==SEND_FILE||re.op==SENDCONFIG  || re.op == INITIAL_CLIENT || re.op == INITIAL_SERVER){
                while(msg[pos]!='\t'){
                    re.content+=msg[pos];
                    pos++;
                }
            }
        }
    }
    return re;
} 
int Communication::send_configmessage(int op,string filename,string content,int no){
    int message_no=no;
    if(no==-1){
        for(int i=1;i<MAXMESSAGE;i++){
            if(message_count_use[i]==0){
                message_no=i;
                break;
            }
        }
    }
    netdisk_message msg(message_no,op,filename,0,"","",content,"","","",0,0);
    string sendstr=message_to_string(msg);
    if(send(sclient,sendstr.c_str(),sendstr.length(),0)<=0){
        cout<<"fail to send message, please check the network"<<endl;
        return myERROR;
    }
    if(no==-1)
        message_count_use[message_no]=1;
    return message_no;
}
// 返回消息号
int Communication::send_usermessage(int op,string username,string userid,string passwd,int no)
{
    int message_no=no;
    if(no==-1){
        for(int i=1;i<MAXMESSAGE;i++){
            if(message_count_use[i]==0){
                message_no=i;
                break;
            }
        }
    }
    netdisk_message msg(message_no,op,"",0,"","","",username,userid,passwd,0,0);
    string sendstr=message_to_string(msg);
    netdisk_message msgtemp=string_to_message(sendstr);
    if(send(sclient,sendstr.c_str(),sendstr.length(),0)<=0){
        cout<<"fail to send message, please check the network"<<endl;
        return myERROR;
    }
    if(no==-1)
        message_count_use[message_no]=1;
    return message_no;
}
// 返回消息号
int Communication::send_message(int op,string filename,boolean is_file,string path,string md5,string content,int no,boolean is_tail)
{
    int message_no=no;
    if(no==-1){
        for(int i=1;i<MAXMESSAGE;i++){
            if(message_count_use[i]==0){
                message_no=i;
                break;
            }
        }
    }
    netdisk_message msg(message_no,op,filename,is_file,path,md5,content,"","","",0,is_tail);
    string sendstr=message_to_string(msg);
    netdisk_message msgtest=string_to_message(sendstr);
    //cout <<msgtest<<endl;
    if(send(sclient,sendstr.c_str(),sendstr.length(),0)<=0){
        cout<<"fail to send message, please check the network"<<endl;
        return myERROR;
    }
    if(no==-1)
        message_count_use[message_no]=1;
    return message_no;
}
Communication::Communication(string ip,int port)
{
    this->ip=ip;
    this->port=port;
    this->ConnectError=false;
    memset(message_count_use,0,MAXMESSAGE);
    // this->connection();
}
int Communication::connection(){
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(sockVersion, &wsaData) != 0)
    {
        return 0;
    }
    sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sclient == INVALID_SOCKET)
    {
        //qDebug()<<strerror(errno)<<endl;
        cout<<"invalid socket!"<<endl;
        this->ConnectError=true;
        return myERROR;
    }
    sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(this->port);
    serAddr.sin_addr.S_un.S_addr = inet_addr(this->ip.c_str());
    if(connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
    {  //连接失败
        cout<<"connect error !"<<endl;
        closesocket(sclient);
        this->ConnectError=true;
        return myERROR;
    }
    return myOK;
}

boolean Communication::connecterror(){
    return this->ConnectError;
}

int Communication::recv_message(netdisk_message &recv_content)
{
    string recvstr;
    char buf[SENDSIZE];
    if(recv(sclient,buf,SENDSIZE,0)<=0)
        return myERROR;
    recvstr=string(buf);
    //qDebug()<<"before"<<endl;
    recv_content=string_to_message(recvstr);
    if(recv_content.op==FINISH){// 如果通信结束，把消息号释放
        message_count_use[recv_content.no]=0;
    }
    //qDebug()<<"after"<<endl;
    //print(recv_content);
    return myOK;
}

int Communication::disconnection(){
    if(connecterror()==false){
        closesocket(sclient);
    }
    return myOK;
}

Communication::~Communication(){
    disconnection();
}

int Communication::REconnection()
{
    while(1){
        if(this->connection()==myOK)
            return myOK;
        Sleep(100);
    }
    return myERROR;
}
