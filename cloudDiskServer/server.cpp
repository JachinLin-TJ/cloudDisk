#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<sys/fcntl.h>
#include<net/if.h>
#include<iostream>
#include <vector>
#include <mysql.h>
using namespace std;
MYSQL *mysql;
MYSQL_RES *result; 
MYSQL_ROW  row;
void connect_mysql()//���ݿ�����
{
	mysql = mysql_init( NULL );	/* ���ӳ�ʼ�� */
	if ( !mysql )
	{
		printf("mysql_init failed\n");
		exit(0);
	}

	mysql = mysql_real_connect( mysql, "localhost", "root", "root123", "cloudDisk", 0, NULL, 0 );	/* ����ʵ������ */
	/* �����ֱ�Ϊ����ʼ�������Ӿ��ָ�룬������������IP�����û��������룬���ݿ�����0��NULL��0����������������Ĭ�ϰ�װmysql>������²��ø� */
    mysql_set_character_set(mysql, "gbk"); 
	if ( mysql ){
		printf( "MariaDB connect success\n" );
	}
	else{
		printf( "MariaDB connect failed\n" );
        exit(0);
	}

}
//ע���˻������ݿ��в����˻����룬���ɹ�����1
bool reg(vector<string>& args)
{
    char sqlcmd[100];
    sprintf(sqlcmd, "insert into user values(\"%s\",\"%s\")", args[1].c_str(),args[2].c_str());
    cout<<sqlcmd<<endl;
    if (mysql_query(mysql, sqlcmd)) {
    	cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
    	return 0;
    }
    return 1;
}
//��¼�˻���У�����룬������Ϸ���1
bool login(vector<string>& args)
{
    char sqlcmd[100];
    sprintf(sqlcmd, "select * from user where username=\'%s\'", args[1].c_str());
    cout<<sqlcmd<<endl;
    if (mysql_query(mysql, sqlcmd)) {
    	cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
    	return 0;
    }
    
    if ((result = mysql_store_result(mysql))==NULL) {
    	cout << "mysql_store_result failed" << endl;
    	return 0;
    }

    row=mysql_fetch_row(result);
    if(row==NULL)
        return 0;
    if(row[1]!=args[2])
        return 0;
    return 1;

}
//�ַ���split
void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c)//�ַ����ָ��
{
  std::string::size_type pos1, pos2;
  pos2 = s.find(c);
  pos1 = 0;
  while(std::string::npos != pos2)
  {
    v.push_back(s.substr(pos1, pos2-pos1));
 
    pos1 = pos2 + c.size();
    pos2 = s.find(c, pos1);
  }
  if(pos1 != s.length())
    v.push_back(s.substr(pos1));
}
//�������
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


int main(int argc, char** argv){
    int sockfd,connfd;
    sockaddr_in servaddr;

    int port=-1;
    if(argc!=3)
    {
        printf("param not right!!\n");
        return 0;
    }

    for(int i=1;i<argc;i=i+2)
	{
		if(strcmp(argv[i],"--port")==0)
            port=atoi(argv[i+1]);
	}
    if(port<0||port>65535)
    {
        printf("port is not in [0~65535]\n");
        return 0;
    }

    // printf("port is %d\n",port);

    connect_mysql();




    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
        printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }
    
    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));


    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if( bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
        printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }

    if( listen(sockfd, 10) == -1){
        printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }

    printf("======waiting for client's request======\n");

    if( (connfd = accept(sockfd, (struct sockaddr*)NULL, NULL)) == -1){
        printf("accept socket error: %s(errno: %d)\n",strerror(errno),errno);
    }
    
    printf("======connect success======\n");

    vector<string> args;
    char  buff[4096];
    //�����ͻ��˵Ķ���
    while(1)
    {
        memset(buff,0,sizeof(buff));
        int n=read(connfd,buff,100);
        if(n<=0)
            break;
        cmdArgs(buff, args);

        if(args[0]=="register")
        {
            if(reg(args))
            {
                write(connfd,"1",1);
            }
            else
                write(connfd,"0",1);
        }
        else if(args[0]=="login")
        {
            if(login(args))
            {
                write(connfd,"1",1);
            }
            else
                write(connfd,"0",1);
            
        }
        else if(args[0]=="bind")
        {
            //���Ͻ����ļ���ֱ���ͻ��˷���end
            while(1)
            {
                string fileheader;
                vector<string> fileinfo;
                int filesize;

                memset(buff,0,sizeof(buff));
                int n=read(connfd,buff,200);//�����ļ�ͷ��Ϣ
                fileheader=buff;
                cout<<fileheader<<endl;
                if(fileheader=="end")
                    break;
                SplitString(fileheader,fileinfo,"\n");//0Ϊ�ļ�����1Ϊ�ļ���С��2ΪMD5��
                filesize=atoi(fileinfo[1].c_str());
                write(connfd,"1",1);//�ظ��ͻ��ˣ�׼���ý����ļ�

                char* filedata=(char*)malloc(sizeof(char) * filesize);
                int leftsize=filesize;
                int readsize=0;
                
                while(1)
                {
                    n=read(connfd,filedata+readsize,leftsize);//�����ļ�
                    readsize+=n;
                    leftsize-=n;
                    if(leftsize==0)
                        break;
                }



                string clouDir=args[2]+"/"+fileinfo[0];

                FILE* fp = fopen(clouDir.c_str(),"wb");
                fwrite(filedata,1,filesize,fp);
                fclose(fp);
                free(filedata);

                
                
                write(connfd,"1",1);//�ظ��ͻ��ˣ�׼���ý����¸��ļ�

            }
        }

    }

    return 0;
}