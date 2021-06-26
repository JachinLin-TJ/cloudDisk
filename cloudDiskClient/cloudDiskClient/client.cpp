#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <string.h>
#include <fstream>
#include <vector>
#include "md5.h"
#include <io.h>
using namespace std;
#pragma comment(lib, "ws2_32.lib")

vector<pair<string, string>> dirFileinfo;//ͬ���̵�Ŀ¼�ļ���Ϣ��·��+md5��
MD5 md5;

void send_file(SOCKET sockfd,string filepath,string filename, string username)
{
    FILE* fp = fopen(filepath.c_str(), "rb");
    struct stat fileInfo;
    stat(filepath.c_str(), &fileInfo);
    char* fileDate = (char*)malloc(sizeof(char) * fileInfo.st_size);
    int readbyte = fread(fileDate, 1, fileInfo.st_size, fp);

    MD5 md5;
    md5.update(fileDate, readbyte);


    string fileheader;

    fileheader = filename + "\n";
    fileheader += to_string(readbyte) +"\n";
    fileheader += md5.toString() + "\n";
    fileheader += username + "\n";
    fileheader += filepath + "\n";
    send(sockfd, fileheader.c_str(), fileheader.length(), 0);
    char res[5];
    memset(res, 0, sizeof(res));
    recv(sockfd, res, 1, 0);
    cout << res << endl;
    if (res[0] == '0')//�ļ��Ѿ����ڣ�����һ���ļ�
        return;



    send(sockfd, fileDate, readbyte,0);

    memset(res, 0, sizeof(res));
    recv(sockfd,res, 1, 0);
    cout << res << endl;
    free(fileDate);
    fclose(fp);

}


//���������еĲ���
void cmdArgs(const string& cmd, vector<string>& args) {
    args.clear();
    string str;
    unsigned int p, q;
    for (p = 0, q = 0; q < cmd.length(); p = q + 1) {
        q = cmd.find_first_of(" \n", p);
        str = cmd.substr(p, q - p);
        if (!str.empty()) {
            args.push_back(str);
        }
        if (q == string::npos)
            return;
    }
}
//Ŀ¼����
void listFiles(const char* dir)
{
    char dirNew[200];
    strcpy(dirNew, dir);
    strcat(dirNew, "\\*.*");    // ��Ŀ¼�������"\\*.*"���е�һ������

    intptr_t handle;
    _finddata_t findData;
    handle = _findfirst(dirNew, &findData);
    if (handle == -1)        // ����Ƿ�ɹ�
        return;

    //cout << dirNew << endl;
    do
    {
        if (findData.attrib & _A_SUBDIR)
        {
            if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
                continue;

            // ��Ŀ¼�������"\\"����������Ŀ¼��������һ������
            strcpy(dirNew, dir);
            strcat(dirNew, "\\\\");
            strcat(dirNew, findData.name);

            listFiles(dirNew);
        }
        else
        {
            char fileInfo[200];
            sprintf(fileInfo, "%s\\\\%s", dir, findData.name);
            dirFileinfo.push_back(make_pair(fileInfo, findData.name));

        }
            
    } while (_findnext(handle, &findData) == 0);

    _findclose(handle);    // �ر��������
}

int main(int argc, char** argv)
{

    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(sockVersion, &wsaData) != 0)
    {
        return 0;
    }
	SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in  servaddr;

    char* ip=NULL;
    int port = -1;
    for (int i = 1; i < argc; i = i + 2)
    {
        if (strcmp(argv[i], "--ip") == 0)
            ip = argv[i + 1];
        else if (strcmp(argv[i], "--port") == 0)
            port = atoi(argv[i + 1]);
    }
    if (port < 0 || port>65535)
    {
        printf("port is not in [0~65535]\n");
        return 0;
    }
    if (argc != 5)
    {
        printf("param not right!!\n");
        return 0;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.S_un.S_addr = inet_addr(ip);

    if (connect(sockfd, (sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR)
    {  //����ʧ�� 
        printf("connect error !\n");
        closesocket(sockfd);
        return 0;
    }


    //����ͻ��˹���
    printf("======connect success======\n");
    printf("======ͬ����======\n");
    string line = "man";
    vector<string> args;
    char username[20]="";
    char cmdname[40];
    sprintf(cmdname, "[ ͬ���� %s ]$ ", username);
    strcat(cmdname, username);
    while (1)
    {
        cmdArgs(line, args);
        if (line == "")
        {

        }
        else if (args[0] == "man")
        {
            cout << "<command˵��>" << endl;
            cout << "<�û�ע�᣺register+�û���+���� ʾ����register jachin tongji6666>" << endl;
            cout << "<�û���¼��login+�û���+���� ʾ����login jachin tongji6666>" << endl;
            cout << "<Ŀ¼�󶨣�bind+����Ŀ¼+����Ŀ¼ ʾ����bind C:\\\\Users\\\\67093\\\\Desktop\\\\test home>" << endl;
            cout << "<ʹ�ð�����man ʾ����man>" << endl;
        }
        else if (args[0] == "register")
        {
            int size = line.length();
            send(sockfd, (line).c_str(), size, 0);
            char res[5]="";
            recv(sockfd, res, 1, 0);
            if (res[0] == '1')//��������ݿ����ɹ�
            {
                cout << "ע��ɹ���" << endl;
            }
            else
            {
                cout << "ע��ʧ�ܣ����˻������ѱ�ע�ᣡ" << endl;
            }

        }
        else if (args[0] == "login")
        {
            int size = line.length();
            send(sockfd, (line).c_str(), size, 0);

            char res[5] = "";
            recv(sockfd, res, 1, 0);

            if (res[0] == '1')//�������֤������ȷ
            {
                cout << "��¼�ɹ���" << endl;
                strcpy(username, args[1].c_str());
                sprintf(cmdname, "[ ͬ���� %s ]$ ", username);
            }
            else
            {
                cout << "��¼ʧ�ܣ����������˻������ڣ�" << endl;
            }
        }
        else if (args[0] == "bind")
        {
            dirFileinfo.clear();
            if (strcmp(username,"")==0)
            {
                cout << "�û�δ��¼�����ȵ�¼" << endl;

            }
            else
            {
                string localDir = args[1];
                string cloudDir = args[2];
                listFiles(localDir.c_str());

                int size = line.length();
                send(sockfd, (line).c_str(), size, 0);

                for (int i = 0; i < dirFileinfo.size(); ++i)
                {
                    cout << "����ͬ����" << i + 1 << "���ļ�" << endl;
                    send_file(sockfd, dirFileinfo[i].first, dirFileinfo[i].second,username);
                }
                send(sockfd, "end", 3, 0);
            }
            
            

        }

        else
        {
            cout << "û�и����" << endl;
        }
        cout << cmdname;
        getline(cin, line);
    
    
    
    
    
    
    }

    

}