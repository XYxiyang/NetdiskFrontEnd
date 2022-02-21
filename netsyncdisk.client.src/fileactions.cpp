#define _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_SECURE_NO_WARNINGS
#include"fileactions.h"
#include<QDebug>
#include<QDateTime>
#include<QTime>
#include<QCoreApplication>
#include"MD5.h"
#include<stdio.h>
#include"communication.h"

// 宽字节字符串转多字节字符串
using namespace std;
extern Communication com;
extern netdisk_message msg;
extern string clientname;
extern string uploading_file;
extern string downloading_file;
extern string logfile;
boolean threads[500];
int thread_pos=0;
string FileTree;

string retMD5(string filename){
    ifstream in;
    in.open((filename.c_str()),ios::in);
    MD5 m(in);
    string md5code=m.toString();
    in.close();
    return md5code;
}

void sendfile(string filename,string path,string md5,int opt=SEND_FILE,string localpath=""){//还没写断点续传
    string file;
    //qDebug()<<filename.c_str()<<endl;
    //qDebug()<<localpath.c_str()<<endl;
    ifstream fin(localpath.c_str(),ios::binary);//
    int count=0;
    char temp_ch;
    uploading_file=filename;
    while (1)
    {
        if(!fin.get(temp_ch)){
            //qDebug()<<"content"<<file.c_str()<<endl;
            com.send_message(opt,filename,true,path,md5,file,-1,true);
            file.clear();
            com.recv_message(msg);

            if(msg.op==FINISH)
                return;

        }
        count++;
        file += temp_ch;
        if(count==SENDFILESIZE){
            count=0;
            com.send_message(opt,filename,true,path,md5,file);
            file.clear();
            com.recv_message(msg);
            if(msg.op==FINISH)
                continue;
        }
    }
    uploading_file.clear();
}
string retFilename(string in){
    if(in.find_last_of("\\")!=in.npos){
        return in.substr(in.find_last_of("\\")+1);
    }
    return in;
}
boolean retifisFile(string path){
    WIN32_FIND_DATAA fdfile;
    if(FindFirstFileA(path.c_str(), &fdfile)==INVALID_HANDLE_VALUE){
        return false;
    }
    //qDebug()<<"path"<<QString::fromStdString((dir+"\\"+name))<<endl;
    if (fdfile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
        return false;
    }
    else return true;
}
string GbkToUtf8(const char *src_str)
{
    int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len + 1]; memset(str, 0, len + 1);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
    string strTemp = str;
    if (wstr) delete[] wstr;
    if (str) delete[] str;
    return strTemp;
}
string changegang(const string in){
    string send=in;
    for(int i=0;i<(int)in.length();i++)
    {
        if(send[i]=='\\')
            send[i]='/';
    }
    return send;
}

void writeLog(string file,string operation,string status,string content){
    fstream out;
    out.open(file.c_str(),ios::out|ios::app);
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss.zzz ddd");
    out<<"["<<current_date.toStdString()<<"]"<<endl;
    out<<"operation:"<<operation<<endl;
    out<<"status:"<<status<<endl;
    out<<content<<endl<<endl;

    out.close();

}

void W2C(wchar_t* pwszSrc, int iSrcLen, char* pszDest, int iDestLen)
{
    ::RtlZeroMemory(pszDest, iDestLen);
    // 宽字节字符串转多字节字符串
    ::WideCharToMultiByte(CP_ACP,
        0,
        pwszSrc,
        (iSrcLen / 2),
        pszDest,
        iDestLen,
        NULL,
        NULL);
}
void checkFileschange(string dir,string path,boolean* run) {
    if(dir.find("\r")!=dir.npos)
        dir.erase(dir.find("\r"));
    HANDLE dir_handle = CreateFileA(dir.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

    if (dir_handle == INVALID_HANDLE_VALUE) {
        exit(0);
    }

    char szTemp[MAX_PATH] = { 0 };
    boolean bRet = false;
    DWORD dwRet = 0;
    DWORD dwBufferSize = 2048;
    BYTE* pBuf = new BYTE[dwBufferSize];
    if (NULL == pBuf)
    {
        exit(0);
    }
    do
    {
        FILE_NOTIFY_INFORMATION* pFileNotifyInfo = (FILE_NOTIFY_INFORMATION*)pBuf;
        ::RtlZeroMemory(pFileNotifyInfo, dwBufferSize);
        // 设置监控目录
        bRet = ::ReadDirectoryChangesW(dir_handle,
            pFileNotifyInfo,
            dwBufferSize,
            TRUE,
            FILE_NOTIFY_CHANGE_SIZE |  // in file or subdir
            FILE_NOTIFY_CHANGE_ATTRIBUTES |
            FILE_NOTIFY_CHANGE_DIR_NAME | // creating, deleting a directory or sub
            FILE_NOTIFY_CHANGE_FILE_NAME |
            FILE_NOTIFY_CHANGE_CREATION,
            //FILE_NOTIFY_CHANGE_LAST_WRITE,//如果不关闭这项会导致目录中的文件夹也显示被更改
            &dwRet,
            NULL,
            NULL);
        if (FALSE == bRet)
        {
            exit(0);
            break;
        }

        // 判断操作类型并显示
        string oldname;
        while (1) {
            // 将宽字符转换成窄字符
            if(!(*run))
                break;
            W2C((wchar_t*)(&pFileNotifyInfo->FileName), pFileNotifyInfo->FileNameLength, szTemp, MAX_PATH);
            switch (pFileNotifyInfo->Action)
            {
            case FILE_ACTION_ADDED:
            {
                // 新增文件
                //qDebug()<<"[File Added Action]\n"<<QString::fromStdString(GbkToUtf8(szTemp))<<endl;
                string name=GbkToUtf8(szTemp);
                if(name[0]=='~'||name[0]=='.')
                    break;
                boolean isfile=retifisFile(dir+"\\"+szTemp);              
                if (!isfile){
                    //qDebug()<<"folder"<<endl;
                    com.send_message(SEND,retFilename(szTemp),false,changegang((path+szTemp)),"","");
                    com.recv_message(msg);
                    writeLog(logfile,"Upload a folder","success","Successfully uploaded a folder");
                }
                else {
                    string md5code=retMD5(dir+"\\"+name);
                    //qDebug()<<"file"<<endl;
                    com.send_message(SEND,retFilename(szTemp),true,changegang((path+szTemp)),md5code,"");
                    com.recv_message(msg);
                    if(msg.op==SURE_GET){
                        sendfile(retFilename(szTemp),changegang(path+szTemp),md5code);
                        writeLog(logfile,"Upload a new file","success","Successfully uploaded a file");
                    }
                    else if(msg.op==NOT_GET){
                        com.send_message(FINISH,retFilename(szTemp),true,changegang((path+szTemp)),md5code,"");
                        //com.recv_message(msg);
                        //if(msg.op==FINISH){
                            writeLog(logfile,"Upload an existed file","success","Successfully uploaded a file");
                        //}
                    }
                    else if(msg.op==EXIST){
                        ;
                    }
                    break;
                }

                /*qDebug()<<QString::fromStdString(dir+"\\"+name)<<endl;
                qDebug()<<QString::fromStdString(md5code)<<endl;
                qDebug()<<QString::fromStdString(path+name)<<endl;*/

            }
            case FILE_ACTION_REMOVED:
            {
                //删除文件
                if(szTemp[0]=='~'||szTemp[0]=='.')
                    break;
                boolean isfile=retifisFile(dir+"\\"+szTemp);    
                if(isfile){
                    com.send_message(REMOVE,retFilename(szTemp),true,changegang((path+szTemp)),"","");
                    com.recv_message(msg);
                    writeLog(logfile,"Remove a file","success","Successfully removed a file");
                }
                else{
                    com.send_message(REMOVE,retFilename(szTemp),false,changegang((path+szTemp)),"","");
                    com.recv_message(msg);
                    writeLog(logfile,"Remove a folder","success","Successfully removed a folder");
                }
                //qDebug()<<"[File Removed Action]\n"<<QString::fromStdString(GbkToUtf8(szTemp))<<endl;

                
                break;
            }
            case FILE_ACTION_MODIFIED:
            {
                // 修改文件
                string name=GbkToUtf8(szTemp);
                //qDebug()<<"[File Modified Action]\n"<< QString::fromStdString(GbkToUtf8(szTemp))<<endl;


                //qDebug()<<QString::fromStdString(dir+"\\"+name)<<endl;
                    string md5code=retMD5(dir+"\\"+name);
                    //qDebug()<<QString::fromStdString(md5code)<<endl;
                    com.send_message(CHANGE,retFilename(szTemp),true,
                                     changegang(path+szTemp),md5code,"");
                    com.recv_message(msg);
                    if(msg.op==SURE_GET){
                        sendfile(retFilename(szTemp),changegang(path+szTemp),md5code);
                        com.recv_message(msg);
                        if(msg.op==FINISH){
                            writeLog(logfile,"Upload an existed file","success","Successfully uploaded a file");
                        }
                        //writeLog(logfile,"Upload a modified file","success","Successfully uploaded a modified file");
                    }
                    else if(msg.op==NOT_GET){
                        com.send_message(FINISH,retFilename(szTemp),true,changegang((path+szTemp)),md5code,"");
                        com.recv_message(msg);
                        if(msg.op==FINISH){
                            writeLog(logfile,"Upload an existed file","success","Successfully uploaded a file");
                        }
                    }
                    else if(msg.op==EXIST){
                        ;
                    }
                break;
            }
            case FILE_ACTION_RENAMED_NEW_NAME:
            {
                string name=GbkToUtf8(szTemp);
                boolean isfile=retifisFile(dir+"\\"+szTemp);
                if(isfile){
                    string md5code=retMD5(dir+"\\"+name);
                    //qDebug()<<QString::fromStdString(md5code)<<endl;
                    com.send_message(RENAME,retFilename(szTemp),true,
                                     changegang(path+oldname),md5code,"");
                    //qDebug()<<szTemp<<endl;
                    //qDebug()<<retFilename(szTemp).c_str()<<endl;
                    com.recv_message(msg);
                    writeLog(logfile,"Rename a file","success","Successfully uploaded a renamed file");
                }
                else{
                    com.send_message(CHANGE,retFilename(szTemp),false,
                                     changegang(path+oldname),"","");
                    com.recv_message(msg);
                    writeLog(logfile,"Rename a folder","success","Successfully uploaded a renamed folder");
                }
                //qDebug()<<"[File New Name]\n"<<QString::fromStdString(GbkToUtf8(szTemp))<<endl;
                break;
            }
            case FILE_ACTION_RENAMED_OLD_NAME:
            {
                //qDebug()<<"[File Old Name]\n"<<QString::fromStdString(GbkToUtf8(szTemp))<<endl;
                oldname=szTemp;
                break;
            }
            default:
            {
                break;
            }
            }
            if (pFileNotifyInfo->NextEntryOffset != 0)//多步操作，移动偏移量
            {
                pFileNotifyInfo = (FILE_NOTIFY_INFORMATION*)(((BYTE*)pFileNotifyInfo) + pFileNotifyInfo->NextEntryOffset);
            }
            else
            {
                break;
            }
        }
        if(!(*run))
            break;
    } while (bRet);

    ::CloseHandle(dir_handle);
    delete[] pBuf;
    pBuf = NULL;
}
void openMonitorThread(string dir,string path) {
    std::thread test1(checkFileschange, dir,path,&threads[thread_pos]);
    test1.detach();

    Sleep(10);
    writeLog(logfile,"Add monitor thread","success","Start to monitor "+dir);
}
void startMonitor(string file) {
    fstream in;
    in.open(file.c_str(), ios::in);
    for(int i=0;i<thread_pos;i++)
        threads[i]=false;
    while (1) {
        char buff[1024] = { 0 };
        in.getline(buff, 1024);
        if (!in.good())
            break;
        string line = buff;
        cout << line << endl;
        if (line.find(">") != line.npos) {
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
            //qDebug() << QString::fromStdString(line.substr(line.find(">") + 1)) << endl;
            string path=line.substr(line.find_first_of("\\")+1);
            path=path.erase(path.find(">"));
            openMonitorThread(line.substr(line.find(">") + 1),path+"\\");
            threads[thread_pos]=true;
            thread_pos++;
            if(thread_pos==500)
                thread_pos=0;
        }

    }

    in.close();
}
string findlinkFolder(string in,string path,boolean &linked,int length) {
    linked = false;
    if ((int)path.length() < length)
        return "no";
    int pos1 = 0, pos2 = 0;

    path.erase(path.length()- 4);
    if ((int)path.length() < length)
            return "no";
    string folder = path.substr(length);
    //qDebug()<<QString::fromStdString(folder)<<endl;
    while (1) {
        if (pos2 == (int)in.length()) {
            return "nothing";
        }
        if (in[pos2] == '\n') {
            string temp;
            if(in[pos2-1]=='\r')
                temp= in.substr(pos1, pos2 - pos1);//xiugai
            else
                temp= in.substr(pos1, pos2 - pos1+1);
            //qDebug()<<"temp"<<QString::fromStdString(temp)<<endl;
            if (temp.find(folder) != temp.npos) {
                if (temp.find(">") != temp.npos) {
                    linked = true;
                    return temp.substr(temp.find(">")+1);
                }
                else {
                    linked = false;
                    return "ababa";
                }
            }
            pos1 = pos2 + 1;
        }
        pos2++;
    }

}
boolean search_folder(const char para[])
{
    WIN32_FIND_DATAA fdfile;
    HANDLE hFind = FindFirstFileA(para, &fdfile);//第一个参数是路径名，可以使用通配符，fd存储有文件的信息
    boolean done = true;
    while (1)
    {
        done = FindNextFileA(hFind, &fdfile); //返回的值如果为0则没有文件要寻了
        if (!done)
            break;
        if (!strcmp(fdfile.cFileName, "..") || !strcmp(fdfile.cFileName, "."))
            continue;
        if (fdfile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            return true;
            FindClose(hFind);
        }
    }
    FindClose(hFind);
    return false;
}
void open_folder(const char para[], int round, int ctrl[],string content,int length,int opt,string path)
{
    //qDebug()<<"para"<<para<<endl;
    WIN32_FIND_DATAA fdfile;
    HANDLE hFind = FindFirstFileA(para, &fdfile);//第一个参数是路径名，可以使用通配符，fd存储有文件的信息
    if (hFind == INVALID_HANDLE_VALUE)
    {
        cout << "无效的路径 - ";
        for (int i = 2; para[i + 1] != '*'; i++)
        {
            if (para[i] <= 'z' && para[i] >= 'a')
                cout << char(para[i] - 32);
            else
                cout << para[i];
        }
        cout << "\n没有子文件夹\n\n";
        return;
    }
    boolean done = true, found_ = search_folder(para);
    if (!found_)
        ctrl[round] = 0;
    int if_is_round1 = 1;
    while (1)
    {
        done = FindNextFileA(hFind, &fdfile);//返回的值如果为0则没有文件要寻了

        if (!done && if_is_round1)
            return;
        if (!strcmp(fdfile.cFileName, "..") || !strcmp(fdfile.cFileName, "."))
            continue;

        if (!done)
            break;
        if_is_round1 = 0;
        if (fdfile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;
        else
        {
            for (int i = 1; i <= round; i++)
                if (ctrl[i])
                    FileTree.append("|   ");
                else
                    FileTree.append("    ");
        }
        FileTree.append(fdfile.cFileName);
        FileTree.append("\n");
        if(opt==CHECKMD5){
            //qDebug()<<"sendfile"<<endl;
            //qDebug()<<fdfile.cFileName<<endl;
            string localpath = para;
            localpath = localpath.erase(localpath.find_last_of("\\") + 1);
            string temp=path+fdfile.cFileName;
            string md5code=retMD5(localpath+"\\" + (fdfile.cFileName) );
            com.send_message(INITIAL_CLIENT,fdfile.cFileName,true,changegang(temp),md5code,"");
            com.recv_message(msg);
            if(msg.op==SURE_GET){
                sendfile(fdfile.cFileName,changegang(temp),md5code,INITIAL_CLIENT,localpath+"\\" + (fdfile.cFileName));
                writeLog(logfile,"Upload a new file","success","Successfully uploaded a file");
            }
            else if(msg.op==NOT_GET){
                //sendfile(fdfile.cFileName,changegang(temp),md5code,INITIAL_CLIENT);
                //com.send_message(FINISH,fdfile.cFileName,true,changegang(temp),md5code,"");
                //com.recv_message(msg);
                //if(msg.op==FINISH){
                    writeLog(logfile,"Upload an existed file","success","Successfully uploaded a file");
                //}
            }
            else if(msg.op==EXIST){
                ;
            }
        }
        if (!done)
        {
            FileTree.append("\n");
            break;
        }
    }
    for (int i = 1; i <= round; i++)
        if (ctrl[i])
            FileTree.append("|   ");
        else
            FileTree.append("    ");
    FileTree.append("\n");
    FindClose(hFind);
    done = true;
    if (!found_)
    {
        return;
    }


    WIN32_FIND_DATAA fdfolder;
    HANDLE hfFind = FindFirstFileA(para, &fdfolder);
    while (1)
    {
        boolean done = true;
        done = FindNextFileA(hfFind, &fdfolder); //返回的值如果为0则没有文件要寻

        if (!done)
            break;
        if (!(fdfolder.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || (fdfolder.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
            continue;
        if (!strcmp(fdfolder.cFileName, ".."))
            continue;
        else//找到的是文件夹
        {
            for (int i = 1; i <= round - 1; i++)
                if (ctrl[i])
                    FileTree.append("|   ");
                else
                    FileTree.append("    ");
        }
        char filename[260] = { 0 };
        strcat(filename, fdfolder.cFileName);
        WIN32_FIND_DATAA temp;
        HANDLE tempFind = FindFirstFileA(para, &temp);
        boolean final = true;
        while (1)
        {
            final = FindNextFileA(tempFind, &temp);
            if (!strcmp(temp.cFileName, filename))
                break;
        }
        while (1)
        {
            final = FindNextFileA(tempFind, &temp);
            if (temp.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                break;
            if (!final)
            {
                //final_folder = true;
                break;
            }

        }
        if (final){
            FileTree.append("+---");
            FileTree.append(fdfolder.cFileName);
            FileTree.append("\n");
        }
        else
        {
            FileTree.append("\\---");
            FileTree.append(fdfolder.cFileName);
            FileTree.append("\n");
            ctrl[round] = 0;
        }
        ctrl[round + 1] = 1;
        char adr[512] = { 0 };
        for (int i = 0; para[i] != '*'; i++)
            adr[i] = para[i];
        strcat(adr, filename);
        boolean islinked;
        string tempstr=path;
        tempstr+=fdfolder.cFileName;
        if(opt==CHECKMD5){
            com.send_message(INITIAL_CLIENT,fdfolder.cFileName,false,changegang(tempstr),"","");
            com.recv_message(msg);
            if(msg.op==FINISH||msg.op==EXIST){
                writeLog(logfile,"Upload a folder","succesee","Successfully uploaded a folder");
            }
        }

        strcat(adr, "\\*.*");

        string a=findlinkFolder(content, adr, islinked, length);
        //qDebug()<<islinked<<endl;
        if (islinked) {
            ////qDebug()<<QString::fromStdString(a)<<endl;

            memset(adr, 0, 260);
            a.erase(std::remove(a.begin(), a.end(), '\r'), a.end());
            a.erase(std::remove(a.begin(), a.end(), '\n'), a.end());
            strcat(adr,a.c_str());
            strcat(adr, "\\*.*");
        }
        ////qDebug()<<"adr"<<adr<<endl;
        if(strcmp(adr,para))
            open_folder(adr, round + 1, ctrl,content,length,opt,tempstr+="\\");
        if (!done)
        {
            FindClose(hfFind);
            FileTree.append("\n");
            return;
        }
    }
}

string openFile(string filename,int length,int opt) {
    //从文件中拿到名字，如果没有箭头表示没有被绑定
    //否则表示被绑定，替换为后面的内容，调用open_folder
    FileTree.clear();
    ifstream in;
    in.open(filename.c_str(), ios::in);
    int ctrl[20] = { 0,1 };
    string content;
    while (in.good())
        content.insert(content.length(),1,(char)in.get());
    //qDebug()<<QString::fromStdString(content)<<endl;
    in.close();
    FileTree.append(clientname+"-root\n");
    //open_folder("D:\\linux_beta\\*.*",1,ctrl,content,100);
    open_folder(("C:\\mycloud\\"+clientname+"\\"+clientname+"-root\\*.*").c_str(),1, ctrl, content,length,opt,"");
    return GbkToUtf8(FileTree.c_str());
}

boolean addDir(string filename,string dirname,string clientusrname) {
    fstream out;
    out.open(filename, ios::out | ios::app |ios::in);
    while (1) {
        char buff[1024] = { 0 };
        out.getline(buff, 1024);
        if (!out.good())
            break;
        string child = buff;
        if (child.find(dirname, 0) != child.npos){
            writeLog(logfile,"Add cloud path","fail","Cloud path already exist");
            //cout << "path already exist" << endl;
            return false;
        }
    }
    out.clear();
    out << dirname << endl;
    string path;
    path += path1;
    path += clientusrname;
    path += path2;
    path += dirname;
    path.append("\\");
    MakeSureDirectoryPathExists(path.c_str());
    out.close();
    startMonitor(filename);

    sendfile("usrconfig.conf","","",SENDCONFIG,"C:\\mycloud\\"+clientname+"\\usrconfig.conf");

    com.send_message(SEND,dirname.substr(dirname.find_last_not_of("\\")+1)
                     ,false,dirname.substr(dirname.find("\\")+1),"","");
    com.recv_message(msg);
    if(msg.op!=NOT_GET)
        return false;

    writeLog(logfile,"Add cloud path","success","Successfully added a cloud path");
    return true;
}
boolean bondDir(string filename,string dir1,string dir2) {

    WIN32_FIND_DATAA fdfile;
    HANDLE hFind = FindFirstFileA((dir2+"\\*.*").c_str(), &fdfile);
    if (hFind == INVALID_HANDLE_VALUE) {
        writeLog(logfile,"Bond cloud path","fail","local folder doesn't exist");
        //cout << "file doesn't exist" << endl;
        return false;//目的路径不存在
    }
    int level = 0;
    fstream file;
    file.open(filename.c_str(), ios::in | ios::out);
    for (int i = 0; i < (int)dir1.length(); i++) {
        if (dir1[i] == '\\')
            level++;
    }
    file.seekg(0, ios::beg);
    boolean found=false;
    while (1) {
        char buff[1024] = { 0 };
        file.getline(buff, 1024);
        if (!file.good())
            break;

        int cur_level = 0;
        boolean bonded = false;
        int i = 0;
        for (i = 0; buff[i] != '\0'; i++) {
            if (buff[i] == '\\')
                cur_level++;
            if (buff[i] == '>') {
                bonded = true;
                break;
            }
        }
        string child = buff;
        //cout << child << endl;
        //cout << cur_level << ' ' << level << endl;
        if (child.find(dir1, 0) != child.npos && bonded == true && cur_level>level) {//子文件夹被绑定了
            writeLog(logfile,"Bond cloud path","fail","A child path have been boned");
            //cout << "child path have been boned" << endl;
            return false;
        }
        if (dir1.find(child.substr(0,i), 0) != dir1.npos && bonded == true && cur_level < level) {//子文件夹被绑定了
            cout << "father path have been boned" << endl;
            return false;
        }
        if (child.find(dir1, 0) != child.npos && cur_level == level)
            found = true;

    }
    file.close();
    if (!found) {
        writeLog(logfile,"Bond cloud path","fail","netdisk folder "+dir1+" dosen't exist");
        //cout << "netdisk folder dosen't exist" << endl;
        return false;
    }
    file.open(filename.c_str(), ios::in | ios::out);
    string content;
    while(1){
        char buff[1024] = { 0 };
        file.getline(buff, 1024);
        if (!file.good())
            break;

        int cur_level = 0;
        int i = 0;
        for (i = 0; buff[i] != '\0'; i++) {
            if (buff[i] == '\\')
                cur_level++;
            if (buff[i] == '>')
                break;
        }
        string line = buff;
        if (line.find(dir1) != line.npos && cur_level == level) {
            content.append(dir1);
            content.append(">");
            content.append(dir2);
            content.append("\r\n");
            continue;
        }
        content.append(buff);
        content.append("\n");
    }
    //qDebug()<<content.c_str()<<endl;
    file.close();
    file.open(filename.c_str(), ios::out | ios::trunc);
    //cout <<  content << endl;
    file << content;
    file.close();
    startMonitor(filename);
    com.send_message(BIND_DIR,"",false,dir1.substr(dir1.find("\\")+1),"","");

    com.recv_message(msg);
    if(msg.op!=FINISH)
        return false;
    sendfile("usrconfig.conf","","",SENDCONFIG,"C:\\mycloud\\"+clientname+"\\usrconfig.conf");
    writeLog(logfile,"Bond cloud path","success","Successfully bond "+dir2+" to "+dir1);
    openFile(("C:\\mycloud\\"+clientname+"\\usrconfig.conf").c_str(),
             ("C:\\mycloud\\"+clientname+"\\").length(),CHECKMD5);
    com.send_message(FINISH_INITIAL,"",false,"","","");

    receiveFiles();
    return true;
}
boolean deleteDir(string filename, string dirname,string clientusrname) {//可以优化 是否删除目录下的全部文件
    fstream out;
    out.open(filename, ios::out | ios::app | ios::in);
    int level = 0;
    for (int i = 0; i < (int)dirname.length(); i++) {
        if (dirname[i] == '\\')
            level++;
    }
    string content;
    boolean found=false;
    while (1) {
        char buff[1024] = { 0 };
        out.getline(buff, 1024);
        if (!out.good())
            break;

        int cur_level = 0;
        int i = 0;
        for (i = 0; buff[i] != '\0'; i++) {
            if (buff[i] == '\\')
                cur_level++;
            if (buff[i] == '>')
                break;
        }
        string child = buff;
        //cout << child << endl;
        //cout << cur_level << ' ' << level << endl;
        if (child.find(dirname, 0) != child.npos&&cur_level>level) {
            writeLog(logfile,"Delete cloud path","fail","Other folders in this folder");
            //cout << "other folders in this folder" << endl;
            return false;
        }
        if (child.find(dirname, 0) != child.npos && cur_level == level) {
            if(child.find(">")!=child.npos){
                 writeLog(logfile,"Delete cloud path","fail","Can't delete a bonded folder");
                return false;
            }
            found=true;
            continue;

        }
        content += child;
        content.append("\n");
    }
    if (!found) {
        writeLog(logfile,"Delete cloud path","fail",dirname+" dosen't exist");
        //cout << "netdisk folder dosen't exist" << endl;
        return false;
    }
    string path;
    path += path1;
    path += clientusrname;
    path += path2;
    path += dirname;
    path.append("\\");
    RemoveDirectoryA(path.c_str());
    out.close();
    out.open(filename.c_str(), ios::out | ios::trunc);
    out << content;
    out.close();
    startMonitor(filename);
    sendfile("usrconfig.conf","","",SENDCONFIG);
    com.send_message(REMOVE,dirname.substr(dirname.find_last_not_of("\\")+1)
                     ,false,dirname.substr(dirname.find("\\")+1),"","");
    com.recv_message(msg);
    if(msg.op!=FINISH)
        return false;
    writeLog(logfile,"Delete cloud path","success","Successfully deleted a cloud path");
    return true;
}
boolean unbondDir(string filename, string dirname) {
    fstream out;
    out.open(filename, ios::out | ios::app | ios::in);
    int level = 0;
    for (int i = 0; i < (int)dirname.length(); i++) {
        if (dirname[i] == '\\')
            level++;
    }
    string content;
    boolean found = false;
    while (1) {
        char buff[1024] = { 0 };
        out.getline(buff, 1024);
        if (!out.good())
            break;

        int cur_level = 0;
        //boolean bonded = false;
        int i = 0;
        for (i = 0; buff[i] != '\0'; i++) {
            if (buff[i] == '\\')
                cur_level++;
            if (buff[i] == '>')
                break;
        }
        string child = buff;
        //cout << child << endl;
        //cout << cur_level << ' ' << level << endl;
        if (child.find(dirname, 0) != child.npos && cur_level == level) {
            if (buff[i] != '>') {
                writeLog(logfile,"Unbond cloud path","fail",dirname+"isn't bonded");
                //cout << "this folder isn't bonded" << endl;
                return false;
            }
            found = true;
            content += child.substr(0,child.find(">"));
            content.append("\n");
            continue;
        }
        content += child;
        content.append("\n");
    }
    if (!found) {
        writeLog(logfile,"Unbond cloud path","fail",dirname+"doesn't exist");
        //cout << "netdisk folder dosen't exist" << endl;
        return false;
    }
    out.close();
    out.open(filename.c_str(), ios::out | ios::trunc);
    out << content;
    out.close();
    startMonitor(filename);
    com.send_message(RM_BIND_DIR,"usrconfig.conf",true,dirname.substr(dirname.find("\\")+1),"","");
    com.recv_message(msg);
    sendfile("usrconfig.conf","","",SENDCONFIG);

    writeLog(logfile,"Unbond cloud path","success","Successfully unbonded a cloud path");
    return true;
}
void createFoldersbyFile(string filename,string dir) {
    string path = dir+"\\";
    fstream file;
    file.open(filename.c_str(), ios::in);
    while (1) {
        char buff[1024] = { 0 };
        file.getline(buff, 1024);
        if (!file.good())
            break;
        int i = 0;
        for (i = 0; buff[i] != '\0'&&buff[i]!='\r'; i++) {
            if (buff[i] == '>') {

                break;
            }
        }
        buff[i] = '\0';
        MakeSureDirectoryPathExists((path+buff+"\\").c_str());
    }
    writeLog(logfile,"Create folders","success","Successfully created folders");
    file.close();
}
string makesureConfigexist() {
    string clientusrname = clientname;//到时候根据登录名进行修改
    char winusrname[256] = { 0 };
    DWORD dwSize = 256;
    GetUserNameA(winusrname, &dwSize);
    string file;
    file += file1;
    file += clientusrname;
    file += file2;
    fstream in;
    in.open(file.c_str());
    string md5code;
    if (in) {
        //createFoldersbyFile(file, file1 + "fyl06" + file2 + clientusrname);
        //bondDir(file,clientusrname + "-root\\folderB", "D:\\linux_beta");
        //addDir(file, "rootB",clientusrname);
        //cout<<deleteDir(file, "Liu-root\\folderA", clientusrname);
        //cout << unbondDir(file, clientusrname + "-root\\folderA");
        in.close();
        return retMD5(file);
    }//配置文件存在，可以读取绑定目录
    else {
        string path;
        path += path1;
        path += clientusrname;
        path += path2;
        MakeSureDirectoryPathExists(path.c_str());
        fstream out;
        out.open(file.c_str(), ios::out);
        path += clientusrname;
        path.append("-root\\");
        MakeSureDirectoryPathExists(path.c_str());
        out << clientusrname << "-root" << endl;
        MakeSureDirectoryPathExists((path+"folderA\\").c_str());
        out << clientusrname << "-root\\folderA" << endl;
        MakeSureDirectoryPathExists((path + "folderB\\").c_str());
        out << clientusrname << "-root\\folderB" << endl;
        MakeSureDirectoryPathExists((path + "folderC\\").c_str());
        out << clientusrname << "-root\\folderC" << endl;

        out.close();

        return retMD5(file);

    }//进行目录绑定
    return "";
}

string retLocalpath(string path,string filename){
    ifstream in;
    in.open((file1+clientname+file2).c_str(),ios::in);
    while (1) {
        char buff[1024] = { 0 };
        in.getline(buff, 1024);
        if (!in.good())
            break;
        string line=buff;
        if(line.find(">")!=line.npos){
            string withoutroot=line.substr(line.find_first_of("\\")+1);
            withoutroot.erase(withoutroot.find(">"));
            if(path.find(withoutroot)!=path.npos&&path!=withoutroot){
                string ret;
                ret=line.substr((line.find(">")+1));
                ret+="\\";
                for(int i=0;i<(int)path.length();i++)
                    if(path[i]=='/')
                        path[i]='\\';
                ret+=path.substr(path.find("\\")+1);
                ret.erase(std::remove(ret.begin(), ret.end(), '\r'), ret.end());
                in.close();
                return ret;
            }
        }
    }
    return "";
}

void receiveFiles(){
    while(1){
        com.recv_message(msg);
        string path=retLocalpath(msg.path,msg.filename);
        path.erase(std::remove(path.begin(), path.end(), '\r'), path.end());
        if(msg.op==FINISH_INITIAL){
            break;
        }
        if(msg.is_file){
            WIN32_FIND_DATAA fdfile;
            //qDebug()<<path.c_str()<<endl;
            if(FindFirstFileA(path.c_str(), &fdfile)==INVALID_HANDLE_VALUE){//不存在
                ofstream out;
                out.open(path.c_str(),ios::out);
                downloading_file=msg.filename;
                com.send_message(SURE_GET,msg.filename,true,msg.path,"","");
                while(1){
                    com.recv_message(msg);
                    out<<msg.content;
                    com.send_message(FINISH,msg.filename,true,msg.path,"","");
                    if(msg.is_tail)
                        break;

                }
                out.close();
                downloading_file.clear();
            }
            else
                com.send_message(EXIST,msg.filename,true,msg.path,"","");
        }
        else{
            MakeSureDirectoryPathExists((path+"\\").c_str());
            com.send_message(EXIST,msg.filename,true,msg.path,"","");
        }
        //存在返回exist
        //不存在返回sure
        
    }
}
