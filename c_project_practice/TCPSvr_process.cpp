/* $Id: TCPSvr_process.cpp,v 1.2 2005/09/02 02:49:24 antfang Exp $
 *      This is unpublished proprietary
 *      source code of Tencent Ltd.
 *      The copyright notice above does not
 *      evidence any actual or intended
 *      publication of such source code.
 *
 *      NOTICE: UNAUTHORIZED DISTRIBUTION, ADAPTATION OR USE MAY BE
 *      SUBJECT TO CIVIL AND CRIMINAL PENALTIES.
 *
 *
 *      FILE:        TCPSvr_process.cpp
 *      DESCRIPTION:
 *                   The process section of pre-forked sub-processes server. You can 
 *                   put your business logics in the function ReceiveProcessRequest.
 *
 */
#define ACCEPT_LOCK
//#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
 #include <signal.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream> 
#include "globe.h"
#include "param.h"
#include "TCPSvr_main.h"
#include "TCPSvr_process.h"
#include "serverlog.h"
#include "TCPSvr_err.h"
#include <mcheck.h>
/*************************************************************/
/* global variables                                          */
/*************************************************************/
extern Config     stConfig;        /* global config structure               */
extern int        iAcceptSempID;
extern u_short  * piProcessCount; 
extern int        iNewSocket;
extern sigset_t   iSigset;         /* signal set to be blocked              */
extern int        iProcessID;      /* process index in the PID area         */
extern int        iPCShmSempID;    /* process count shared memory semp ID   */
extern int        iPCShmID;        /* process status shared memory id       */
extern int        iNewSocket;      /* listened socket                       */
extern int        iListenfd;       /* listened filed                        */
extern char     g_szServerName[20];     /*server name: ICBC/CFT*/
//int (*dlfun)(char * ,char *);
int (*dlfun)(char * ,    char *,      void* (*)(string),     void(*)(string,void*)  ,     string(*)(string) ,char *,string&);
int (*dlfun2)(char *);
//so需定义的资源释放方法 在so里必须定义为MyFreeGlobeFun
int (*dlfreefun)(string,    void* (*)(string)   );
char              s_szErrMsg[255];
key_t             g_tProcNumShmKey;
key_t             g_tProcNumSemKey;
key_t             g_tAcceptSemKey;
key_t             g_tSoUpdateShmKey;
key_t             g_tSoUpdateSemKey;
key_t             g_tSoHandleShmKey;
key_t             g_tSoHandleSemKey;

int               g_iSysStatus = SYS_STATUS_NORMAL;

char              g_szServiceCode[MAX_CGI_LEN];
 char g_szClientIP[100];
int               g_iPayChannelSubId;

map<string, string>  SysPairMap;
map<string, string> SOValuePairMap;
map<string, string> AuthorIpPairMap;

//zhichen20111205 供业务so保存全局变量的MAP,KEY 命名一定是SO的名字(区分大小写)+ 变量名 如：CmdEchoDemo.GLOBE
map<string, void *> ProcessGlobeValueMap;
//zhichen20111205 进程SO缓存MAP,提高效率
map<string, void *> ProcessSoMap;

extern  SoUpdateInfo  * psProcessSoUpdateInfo;
extern  int        iPCSoShmID;
extern  int        iPCSoSemID;

extern  ProcessHandleInfo  * psProcessSoHandleInfo;
extern  int        iPCSoHandleShmID;
extern  int        iPCSoHandleSemID;
extern  int        sendMonitorFlag;

static char tmpLogPath[50];
static char tmpPidPath[50];
static char tmpSoPath [50];

string g_orgClientIp;
int    g_orgClientPort;
string g_orgClientTag;
string g_orgCommand;
string g_orgCommandType;
string g_orgFromType;
string g_orgFromUserid;
string g_orgFromToolid;
string g_fromorgcommand;
string g_fromorgcommandtype;
string g_fromorgcalltype;
string g_fromorguin;
string g_http_referer;

int CallDllFun(const char *so, const char *command, char * fun, char * param, char *oparam, char *AfterReturnOparam,string& longOparam);

int CallDllFun2(const char *so, const char *command, char * fun, char * param);

using namespace std;


// del = 1 需删除原有指针  返回 NULL

void* GetGlobeFun(string K)
{
	return ProcessGlobeValueMap[K];
}

void   SetGlobeFun(string K,void* V)
{
	ProcessGlobeValueMap[K] = V;
}


int SendReportMsg(const string &msg)
{
	int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in svr;
	bzero(&svr, sizeof(svr));
	svr.sin_family = AF_INET;
	inet_pton(AF_INET, stConfig.sLogReportUdpIP, &svr.sin_addr);
	svr.sin_port = htons(stConfig.iLogReportUdpPort);
	int res= sendto(sock_fd, msg.c_str(), msg.length(), 0, (struct sockaddr *)&svr, sizeof(svr));
	close(sock_fd);
	return res;

}

//发送udp监控信息,走logserver
int SendMonitorMsg2(int udpFd,const string &msg)
{
	struct sockaddr_in svr;
	bzero(&svr, sizeof(svr));
	svr.sin_family = AF_INET;
	inet_pton(AF_INET, stConfig.sServerMonitorUdpIP, &svr.sin_addr);
	svr.sin_port = htons(stConfig.iServerMonitorUdpPort);
	return sendto(udpFd, msg.c_str(), msg.length(), 0, (struct sockaddr *)&svr, sizeof(svr));
}

//richardzha 20121210
//发送udp监控信息,走logserver
int SendMonitorMsg2(const string &msg)
{
	if (0 == sendMonitorFlag)
		return 0;
	int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	int res = SendMonitorMsg2(sock_fd,msg);
	close(sock_fd);
	return res;
}


//回调函数供so调用so
//返回处理结果
string CallOtherSo(string req)
{
//modify by jameszhou in 2012-06-18 把接收字符串由原来定长改成无限制长度

//    char          Request[MAX_REQ_LEN + 1];

	req.append("&");req.append("fromtype=");req.append(g_orgFromType);
	req.append("&");req.append("fromuserid=");req.append(g_orgFromUserid);
	req.append("&");req.append("fromtoolid=");req.append(g_orgFromToolid);
	req.append("&");req.append("fromorgcommand=");req.append(g_orgCommand);
	req.append("&");req.append("fromorgcommandtype=");req.append(g_orgCommandType);
	req.append("&");req.append("fromorgcalltype=");req.append("CALLOTHERSO");
	req.append("&");req.append("http_referer=");req.append(g_http_referer);
	char *Request;
	int iLen = req.length() + 1;
	char	RequestTmp[1024];
	if(iLen<1024){
		memset(RequestTmp, 0x00, iLen );
		strcpy(RequestTmp, req.c_str())	;
	}else{
		Request = new char[ iLen ];
		memset(Request, 0x00, iLen );
		strcpy(Request, req.c_str())	;

	}


//	char          AfterReturnOparam[MAX_ANS_LEN+1]   ;
	char *AfterReturnOparam;
	AfterReturnOparam = new char[ iLen + 1024 ];
	memset(AfterReturnOparam, 0x00, iLen + 1024 );

	strcpy(AfterReturnOparam, req.c_str());


//end modify

    char          MsgSentByServer[MAX_ANS_LEN+1]   ;
    
    int           iRet;
    char          szComment[200];
    char          szCommand[100];
    string rsp = "";
	char  szCommandType[128]; //input1 ��type
   // memset(Request, 0x00, sizeof(Request));
    //strcpy(Request,req.c_str());
    if(iLen<1024){
    	TLib_Log_LogMsg("Child 1024process=[%d], CallOtherSo begin REQUEST=[%s]",getpid(), RequestTmp);
		memset(szCommand, 0x00, sizeof(szCommand));
		if( CParam::getParam(RequestTmp, "command", szCommand, 100) != 0)
		{
			rsp = "result=-1001&msg=not find command";
			TLib_Log_LogMsg("CallOtherSo ERR Child process=[%d],REQUEST=[%s] not find command",getpid(), RequestTmp);
			//modify by jameszhou
			delete []AfterReturnOparam;
			//end modify
			return rsp;
		}
		memset(szCommandType, 0x00, sizeof(szCommandType));
		if( CParam::getParam(RequestTmp, "input1", szCommandType, 128) != 0){
			memset(szCommandType, 0x00, sizeof(szCommandType));
			if( CParam::getParam(RequestTmp, "type", szCommandType, 128) != 0){
				memset(szCommandType, 0x00, sizeof(szCommandType));
			}
		}
    }else{
		TLib_Log_LogMsg("Child process=[%d], CallOtherSo begin REQUEST=[%s]",getpid(), Request);
		memset(szCommand, 0x00, sizeof(szCommand));
		if( CParam::getParam(Request, "command", szCommand, 100) != 0)
		{
			rsp = "result=-1001&msg=not find command";
			TLib_Log_LogMsg("CallOtherSo ERR Child process=[%d],REQUEST=[%s] not find command",getpid(), Request);
			//modify by jameszhou
			delete []Request;
			delete []AfterReturnOparam;
			//end modify
			return rsp;
		}
		memset(szCommandType, 0x00, sizeof(szCommandType));
		if( CParam::getParam(Request, "input1", szCommandType, 128) != 0){
			memset(szCommandType, 0x00, sizeof(szCommandType));
			if( CParam::getParam(Request, "type", szCommandType, 128) != 0){
				memset(szCommandType, 0x00, sizeof(szCommandType));
			}
		}
    }

    memset(MsgSentByServer, 0x00, sizeof(MsgSentByServer));
//    memset(AfterReturnOparam, 0x00, sizeof(AfterReturnOparam));

    memset(szComment, 0x00, sizeof(szComment));
    string soTmpPath = SysPairMap["BusinessSoPath"] +  "/%s.so";
    sprintf(szComment, soTmpPath.c_str(),szCommand);

    string longOparam;
    if(iLen<1024){
    	 iRet =  CallDllFun(szComment, szCommand, "MyProcess", RequestTmp, MsgSentByServer,AfterReturnOparam,longOparam);
    }else{
    	 iRet =  CallDllFun(szComment, szCommand, "MyProcess", Request, MsgSentByServer,AfterReturnOparam,longOparam);
    }


    if (longOparam.length() > 0){
    	rsp = longOparam;
    }else{
    	rsp = MsgSentByServer;
    }

    char monitormsg[10240];
    memset(monitormsg, 0x00,sizeof(monitormsg));
    int costTime = 0;
    sprintf(monitormsg,"lid=KBMS_LOGSERVER_MSG&Command=%s&Commandtype=%s&FunctionRet=%d&ServerTime=%ld&ServerIP=%s&ServerPort=%d&ClientIP=%s&ClientPort=%d&ClientType=%s&CostTime=%d&ClientIPTag=%s&FromType=%s&FromUserId=%s&FromToolId=%s&OrgCommand=%s&OrgCommandtype=%s&FromOrgCommand=%s&FromOrgCommandtype=%s&FromOrgCalltype=%s&FromOrgUin=%s",
    		szCommand,szCommandType,iRet,time(NULL),stConfig.szBindIP,stConfig.iPort,g_orgClientIp.c_str(),g_orgClientPort,"KBMS",costTime,g_orgClientTag.c_str(),g_orgFromType.c_str(),g_orgFromUserid.c_str(),g_orgFromToolid.c_str(),g_orgCommand.c_str(),g_orgCommandType.c_str(),g_fromorgcommand.c_str(),g_fromorgcommandtype.c_str(),g_fromorgcalltype.c_str(),g_fromorguin.c_str());
    //SendMonitorMsg2(monitormsg);

    string reportMsg(monitormsg);
    reportMsg.append("||###||");
    if(iLen<1024){
    	reportMsg.append(RequestTmp);
        TLib_Log_LogMsg("Child 1024process=[%d], CallOtherSo end  Ret=[%d],ANS=[%s] ,REQUEST=[%s]",getpid(),iRet,rsp.c_str(), RequestTmp);
    }else{
    	reportMsg.append(Request);
    	//modify by jameszhou
    	delete []Request;
        TLib_Log_LogMsg("Child process=[%d], CallOtherSo end  Ret=[%d],ANS=[%s] ,REQUEST=[%s]",getpid(),iRet,rsp.c_str(), Request);
    }
    reportMsg.append("||###||");
    reportMsg.append(rsp);
    reportMsg.append("\r\n");
    SendReportMsg(reportMsg);

	delete []AfterReturnOparam;

//end modify	

	return rsp;

}




/** zhichenlei 20111114
int CallDllFun(const char *so, char * fun, char * param, char *errmsg)
{	
	void *handle;
	int ret=1;
	char *dlError;
	handle = dlopen (so, RTLD_LAZY);
	if (!handle) {
		strcpy(errmsg, dlerror());
			return -1001;
	}	
	dlfun =(int (*)(char * , char *)) dlsym(handle,fun);
	dlError = dlerror();
	if ( dlError)  {
		strcpy(errmsg, dlError);
			return -1001;	
	}
	ret =(*dlfun)((char*)param, errmsg);	 
	dlclose(handle);
	return ret;
}
**/

//CallDllFun返回后关闭客户端socket，再次调用MyProcessAfterReturn so用
int CallDllFun2(const char *so, const char *command, char * fun, char * param)
{
	void *handle;
	int ret=1;
	char *dlError;
	TLib_Log_LogMsg("CallDllFun2 Child process[%d] use so=[%s] begin.",getpid(), so);

	string cmd(command);
	string soName = cmd + ".so";

	handle = ProcessSoMap[soName];

	if (handle ==  NULL){
		TLib_Log_LogMsg("CallDllFun2 Child process[%d] dlopen so=[%s] begin.",getpid(), so);
		handle = dlopen (so, RTLD_LAZY);
	}else{
		TLib_Log_LogMsg("CallDllFun2 Child process[%d] usemem so=[%s] begin.",getpid(), so);
	}

	if (!handle) {
		TLib_Log_LogMsg("CallDllFun2 Child process[%d] dlopen error=[%s]",getpid(), dlerror());
		return -1001;
	}
	dlfun2 =(int (*)(char *) ) dlsym(handle,fun);
	dlError = dlerror();
	if ( dlError)  {
		TLib_Log_LogMsg("CallDllFun2 Child process[%d] dlsym error=[%s]",getpid(), dlerror());
		dlclose(handle);
		ProcessSoMap[soName]= NULL;
		return -1002;
	}
	ret =(*dlfun2)((char*)param);

	ProcessSoMap[soName] = handle;

	TLib_Log_LogMsg("CallDllFun2 Child process[%d] use so=[%s] end.", getpid(),so);
	return ret;
}




/**
 *20111114 zhichenlei
 *调整errmsg为opram,并在处理出错时返回result 和msg 两个域。
 *调用端可直接解析这两个域
 *
 *AfterReturnOparam:不为空，则在MyProcess返回并把数据发回客户端并关闭socket后调用 MyProcessAfterReturn
 */
int CallDllFun(const char *so, const char *command, char * fun, char * param, char *oparam, char *AfterReturnOparam,string& longOparam)
{
	void *handle;
	int ret=1;
	char *dlError;
	TLib_Log_LogMsg("Child process[%d] use so=[%s] begin.",getpid(), so);

	string cmd(command);
	string soName = cmd + ".so";

	handle = ProcessSoMap[soName];

	if (handle ==  NULL){
		TLib_Log_LogMsg("Child process[%d] dlopen so=[%s] begin.",getpid(), so);
		handle = dlopen (so, RTLD_LAZY);
	}else{
		TLib_Log_LogMsg("Child process[%d] usemem so=[%s] begin.",getpid(), so);
	}

	if (!handle) {
		strcpy(oparam, "result=-1001&msg=System Busy");
		//strcat(oparam, dlerror());
		TLib_Log_LogMsg("Child process[%d] dlopen error=[%s]",getpid(), dlerror());
		return -1001;
	}
	//////dlfun =(int (*)(char * , char *)) dlsym(handle,fun);
	dlfun =(int (*)(char * ,char *,void* (*)(string),void(*)(string,void*) ,string(*)(string) ,char* ,string&)) dlsym(handle,fun);
	dlError = dlerror();
	if ( dlError)  {
		strcpy(oparam, "result=-1002&msg=System Busy");
		//strcat(oparam, dlerror());
		TLib_Log_LogMsg("Child process[%d] dlsym error=[%s]",getpid(), dlerror());
		dlclose(handle);
		ProcessSoMap[soName]= NULL;
		return -1002;
	}
	ret =(*dlfun)((char*)param,oparam,GetGlobeFun,SetGlobeFun,CallOtherSo,AfterReturnOparam,longOparam);

	//dlclose(handle);
	ProcessSoMap[soName] = handle;

	TLib_Log_LogMsg("Child process[%d] use so=[%s] end.", getpid(),so);
	return ret;
}


static int
BankRecordCommTime(struct timeval *tvbegin, struct timeval *tvend)
{
	 
	if(tvbegin->tv_sec == 0 && tvbegin->tv_usec ==0)
	{
		gettimeofday(tvbegin,NULL);
	     
	}
	if(tvend->tv_sec == 0 && tvend->tv_usec ==0)
	{
		gettimeofday(tvend,NULL);
	     
	}

	return (tvend->tv_sec*1000*1000+tvend->tv_usec -tvbegin->tv_sec*1000*1000-tvbegin->tv_usec)/1000;
	  
}

key_t
GetIpcKey(char *pchPath, int iKeyID)
{
    key_t       tIpcKey;

    if ((tIpcKey = ftok(pchPath, iKeyID)) == (key_t)-1) {
        return (key_t)E_FAIL;
    }

    return tIpcKey;
}


void 
UserCustSigusr1Child(void)
{
    TLib_Log_LogMsg("Child process[%d] recieve sigusr1 to exit", getpid());
    return ;
}

void 
UserCustSigusr1Father(void)
{
    TLib_Log_LogMsg("Father process[%d] recieve sigusr1 to exit", getpid());
    return ;
}

void 
UserCustAbnormalExit(void)
{
    TLib_Log_LogMsg("Child process[%d] abnormal to exit", getpid());
    return ;
}

void 
UserCustFatherExit(void)
{
    TLib_Log_LogMsg("Father process[%d] abnormal to exit", getpid());
    return ;
}

static int
InitIpcKey(char *pchHomePath)
{
    TLib_Log_LogMsg("------------------------------SysIpc Start---------------------------------");

    /*murydu modify begin*/
    if ((g_tProcNumShmKey = GetIpcKey(pchHomePath, stConfig.iCountShmKey)) == (key_t)E_FAIL) {
        fprintf(stderr, "get proc num shm ipc key error\n");
        return E_FAIL;
    }
    TLib_Log_LogMsg("ProcNumShmKey=[ipcrm -M 0x%08X],", g_tProcNumShmKey);
	printf("ProcNumShmKey=[ipcrm -M 0x%08X]\n", g_tProcNumShmKey);

    if ((g_tProcNumSemKey = GetIpcKey(pchHomePath, stConfig.iCountSemKey)) == (key_t)E_FAIL) {
        fprintf(stderr, "get proc num sem ipc key error\n");
        return E_FAIL;
    }
    TLib_Log_LogMsg("ProcNumSemKey=[ipcrm -S 0x%08X]", g_tProcNumSemKey);
	printf("ProcNumSemKey=[ipcrm -S 0x%08X]\n", g_tProcNumSemKey);

    if ((g_tAcceptSemKey = GetIpcKey(pchHomePath, stConfig.iAcceptSemKey)) == (key_t)E_FAIL) {
        fprintf(stderr, "get accept sem ipc key error\n");
        return E_FAIL;
    }
    TLib_Log_LogMsg("AcceptSemKey=[ipcrm -S 0x%08X]", g_tAcceptSemKey);
	printf("AcceptSemKey=[ipcrm -S 0x%08X]\n", g_tAcceptSemKey);

	//so更新控制信号
    if ((g_tSoUpdateShmKey = GetIpcKey(pchHomePath, stConfig.iSoUpdateShmKey)) == (key_t)E_FAIL) {
        fprintf(stderr, "get so update shm ipc key error\n");
        return E_FAIL;
    }
    TLib_Log_LogMsg("SoUpdateShmKey=[ipcrm -M 0x%08X]", g_tSoUpdateShmKey);
	printf("SoUpdateShmKey=[ipcrm -M 0x%08X]\n", g_tSoUpdateShmKey);


    if ((g_tSoUpdateSemKey = GetIpcKey(pchHomePath, stConfig.iSoUpdateSemKey)) == (key_t)E_FAIL) {
        fprintf(stderr, "get so update sem ipc key error\n");
        return E_FAIL;
    }
    TLib_Log_LogMsg("SoUpdateSemKey=[ipcrm -S 0x%08X]", g_tSoUpdateSemKey);
	printf("SoUpdateSemKey=[ipcrm -S 0x%08X]\n", g_tSoUpdateSemKey);

	//so处理时间控制信号
    if ((g_tSoHandleShmKey = GetIpcKey(pchHomePath, stConfig.iSoHandleShmKey)) == (key_t)E_FAIL) {
        fprintf(stderr, "get so Handle shm ipc key error\n");
        return E_FAIL;
    }
    TLib_Log_LogMsg("SoHandleShmKey=[ipcrm -M 0x%08X]", g_tSoHandleShmKey);
	printf("SoHandleShmKey=[ipcrm -M 0x%08X]\n", g_tSoHandleShmKey);


    if ((g_tSoHandleSemKey = GetIpcKey(pchHomePath, stConfig.iSoHandleSemKey)) == (key_t)E_FAIL) {
        fprintf(stderr, "get so Handle sem ipc key error\n");
        return E_FAIL;
    }
    TLib_Log_LogMsg("SoHandleSemKey=[ipcrm -S 0x%08X]", g_tSoHandleSemKey);
	printf("SoHandleSemKey=[ipcrm -S 0x%08X]\n", g_tSoHandleSemKey);



    /*murydu modify end*/

    /****************************************************************************
    if ((g_tProcNumShmKey = GetIpcKey(pchHomePath, PROCESS_COUNT_SHM_KEY)) == (key_t)E_FAIL) {
        fprintf(stderr, "get proc num shm ipc key error\n");
        return E_FAIL;
    }
    TLib_Log_LogMsg("ProcNumShmKey=[%u]", g_tProcNumShmKey);

    if ((g_tProcNumSemKey = GetIpcKey(pchHomePath, PROCESS_COUNT_SEM_KEY)) == (key_t)E_FAIL) {
        fprintf(stderr, "get proc num sem ipc key error\n");
        return E_FAIL;
    }
    TLib_Log_LogMsg("ProcNumSemKey=[%u]", g_tProcNumSemKey);

    if ((g_tAcceptSemKey = GetIpcKey(pchHomePath, ACCEPT_SEM_KEY)) == (key_t)E_FAIL) {
        fprintf(stderr, "get accept sem ipc key error\n");
        return E_FAIL;
    }
    //TLib_Log_LogMsg("AcceptSemKey=[%u]", g_tAcceptSemKey);
    ********************************************************************************/

    //TLib_Log_LogMsg("------------------------------SysIpc End-----------------------------------");

    return E_OK;
}

int InitSysConfig(char *pchFilePath)
{
	
	const int LINE_LENGTH = 1024;
	char buf[LINE_LENGTH];
       char ConfigFile[1024];
	
	char paramName[128];
	char paramValue[1024];
	    
	memset(ConfigFile, 0x00, sizeof(ConfigFile));
	sprintf(ConfigFile, pchFilePath);
	cout << "sys config file :" <<ConfigFile <<endl;
	//open the configuration file
	ifstream in(ConfigFile);
	assert(in);

	while (!in.eof()) {
		memset(buf, 0x00, sizeof(buf));
		in.getline(buf, LINE_LENGTH);
		if(buf[0] == '\0' || buf[0] == '#') {
			//neglect the blank and commemt line
			continue;
		}

		memset(paramName, 0x00, sizeof(paramName));
		memset(paramValue, 0x00, sizeof(paramValue));
		sscanf(buf, "%s : %s", paramName, paramValue);
		
		std::cout << "name: " << paramName << ", value: " << paramValue << endl;
	
		// store into map
		SysPairMap[paramName] = paramValue;		
	}

	if (SysPairMap.count("BindIP"))
	{
		memset(stConfig.szBindIP, 0x00, sizeof(stConfig.szBindIP));
		strncpy(stConfig.szBindIP, SysPairMap["BindIP"].c_str(), sizeof(stConfig.szBindIP));
		std::cout << "set BindIP to: " << stConfig.szBindIP << endl;
	}
	else
	{
		std::cout << "绑定IP必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("ListenPort")) 
	{
 		stConfig.iPort = atoi(SysPairMap["ListenPort"].c_str());
		std::cout << "set ListenPort to: " << stConfig.iPort<< endl;
	}
	else
	{
		std::cout << "监听端口必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("ProcessNum")) 
	{
 		stConfig.iProcessNum= atoi(SysPairMap["ProcessNum"].c_str());
		std::cout << "set ProcessNum to: " << stConfig.iProcessNum<< endl;
	}
	else
	{
		std::cout << "子进程数目必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("MaxProcessNum")) 
	{
 		stConfig.iMaxProcessNum= atoi(SysPairMap["MaxProcessNum"].c_str());
		std::cout << "set MaxProcessNum to: " << stConfig.iMaxProcessNum<< endl;
	}
	else
	{
		std::cout << "最大子进程数目必须配置" << endl;
		return -1;
	}

	if (stConfig.iMaxProcessNum > MAX_PROCESS_COUNT){
		std::cout << "配置的MaxProcessNum不能超过" << MAX_PROCESS_COUNT << endl;
		return -1;
	}



	if (SysPairMap.count("ForkInterval")) 
	{
 		stConfig.iForkInterval= atoi(SysPairMap["ForkInterval"].c_str());
		std::cout << "set ForkInterval to: " << stConfig.iForkInterval<< endl;
	}
	else
	{
		std::cout << "创建子进程间隔必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("KillInterval")) 
	{
 		stConfig.iKillInterval= atoi(SysPairMap["KillInterval"].c_str());
		std::cout << "set KillInterval to: " << stConfig.iKillInterval<< endl;
	}
	else
	{
		std::cout << "杀死子进程间隔必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("ForkExtraThreshold")) 
	{
 		stConfig.iForkExtraThreshold= atoi(SysPairMap["ForkExtraThreshold"].c_str());
		std::cout << "set ForkExtraThreshold to: " << stConfig.iForkExtraThreshold<< endl;
	}
	else
	{
		std::cout<< "创建额外进程阀值必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("FreeTimeBeforeKill")) 
	{
 		stConfig.iFreeTimeBeKill= atoi(SysPairMap["FreeTimeBeforeKill"].c_str());
		std::cout << "set FreeTimeBeforeKill to: " << stConfig.iFreeTimeBeKill<< endl;
	}
	else
	{
		std::cout << "杀死子进程前休眠时间必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("FatherSleepTime")) 
	{
 		stConfig.iFatherSleepTime= atoi(SysPairMap["FatherSleepTime"].c_str());
		std::cout << "set FatherSleepTime to: " << stConfig.iFatherSleepTime<< endl;
	}
	else
	{
		std::cout << "父进程休眠时间必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("SysStatus")) 
	{
 		stConfig.m_iSysStatus= atoi(SysPairMap["SysStatus"].c_str());
		std::cout << "set SysStatus to: " << stConfig.m_iSysStatus<< endl;
	}
	else
	{
		std::cout << "系统状态必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("ConnectionTimeOut")) 
	{
 		stConfig.iConnectionTimeOut= atoi(SysPairMap["ConnectionTimeOut"].c_str());
		std::cout << "set ConnectionTimeOut to: " << stConfig.iConnectionTimeOut<< endl;
	}
	else
	{
		std::cout << "连接超时必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("ProcessHandleTimeOut"))
	{
 		stConfig.iProcessHandleTimeOut= atoi(SysPairMap["ProcessHandleTimeOut"].c_str());
		std::cout << "set ProcessHandleTimeOut to: " << stConfig.iProcessHandleTimeOut<< endl;
	}
	else
	{
		std::cout << "进程最大处理时间必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("MaxLogSize"))
	{
 		stConfig.lMaxLogSize= atol(SysPairMap["MaxLogSize"].c_str());
		std::cout << "set MaxLogSize to: " << stConfig.lMaxLogSize<< endl;
	}
	else
	{
		std::cout << "最大日志文件值必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("MaxLogNum"))
	{
 		stConfig.iMaxLogNum= atoi(SysPairMap["MaxLogNum"].c_str());
		std::cout << "set MaxLogNum to: " << stConfig.iMaxLogNum<< endl;
	}
	else
	{
		std::cout << "最大日志文件个数值必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("PidFilePath"))
	{
		memset(tmpPidPath, 0x00, sizeof(tmpPidPath));
		strncpy(tmpPidPath, SysPairMap["PidFilePath"].c_str(), sizeof(tmpPidPath));
		std::cout << "set PidFile to: " << tmpPidPath<< endl;
	}
	else
	{
		std::cout << "pid文件必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("LogFilePath")) 
	{
		memset(tmpLogPath, 0x00, sizeof(tmpLogPath));
		strncpy(tmpLogPath, SysPairMap["LogFilePath"].c_str(), sizeof(tmpLogPath));
		std::cout << "set PidFile to: " << tmpLogPath<< endl;
	}
	else
	{
		std::cout << "日志文件必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("CountShmKey")) 
	{
 		stConfig.iCountShmKey= atoi(SysPairMap["CountShmKey"].c_str());
		std::cout << "set CountShmKey to: " << stConfig.iCountShmKey<< endl;
	}
	else
	{
		std::cout << "进程计数共享内存key必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("CountSemKey")) 
	{
 		stConfig.iCountSemKey= atoi(SysPairMap["CountSemKey"].c_str());
		std::cout << "set CountSemKey to: " << stConfig.iCountSemKey<< endl;
	}
	else
	{
		std::cout << "进程计数信号灯key必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("AcceptSemKey")) 
	{
 		stConfig.iAcceptSemKey= atoi(SysPairMap["AcceptSemKey"].c_str());
		std::cout << "set AcceptSemKey to: " << stConfig.iAcceptSemKey<< endl;
	}
	else
	{
		std::cout << "接受计数信号灯key必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("InitIpcKeyFile"))
	{
		std::cout << "set InitIpcKeyFile  to: " << SysPairMap["InitIpcKeyFile"].c_str() << endl;
	}
	else
	{
		std::cout << "Ipc key 值必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("BusinessSoPath"))
	{
		memset(tmpSoPath, 0x00, sizeof(tmpSoPath));
		strncpy(tmpSoPath, SysPairMap["BusinessSoPath"].c_str(), sizeof(tmpSoPath));
		std::cout << "set BusinessSoPath  to: " << tmpSoPath << endl;
	}
	else
	{
		std::cout << "业务so路径 必须配置" << endl;
		return -1;
	}

	//so更新控制信号
	if (SysPairMap.count("SoUpdateShmKey"))
	{
 		stConfig.iSoUpdateShmKey= atoi(SysPairMap["SoUpdateShmKey"].c_str());
		std::cout << "set SoUpdateShmKey to: " << stConfig.iSoUpdateShmKey<< endl;
	}
	else
	{
		std::cout << "so更新共享内存key必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("SoUpdateSemKey"))
	{
 		stConfig.iSoUpdateSemKey= atoi(SysPairMap["SoUpdateSemKey"].c_str());
		std::cout << "set SoUpdateSemKey to: " << stConfig.iSoUpdateSemKey<< endl;
	}
	else
	{
		std::cout << "so更新信号灯key必须配置" << endl;
		return -1;
	}

	//so处理时间控制信号
	if (SysPairMap.count("SoHandleShmKey"))
	{
 		stConfig.iSoHandleShmKey= atoi(SysPairMap["SoHandleShmKey"].c_str());
		std::cout << "set SoHandleShmKey to: " << stConfig.iSoHandleShmKey<< endl;
	}
	else
	{
		std::cout << "so业务处理时间控制共享内存key必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("SoHandleSemKey"))
	{
 		stConfig.iSoHandleSemKey= atoi(SysPairMap["SoHandleSemKey"].c_str());
		std::cout << "set SoHandleSemKey to: " << stConfig.iSoHandleSemKey<< endl;
	}
	else
	{
		std::cout << "so业务处理时间控制信号灯key必须配置" << endl;
		return -1;
	}

	if (SysPairMap.count("EnableIpLimit"))
	{
 		stConfig.iIpLimitEnable= atoi(SysPairMap["EnableIpLimit"].c_str());
		std::cout << "set EnableIpLimit to: " << stConfig.iIpLimitEnable<< endl;
	}
	else
	{
		std::cout << "是否启用ip限制必须配置" << endl;
		return -1;
	}

    if (SysPairMap.count("LogReportIP"))
    {
        memset(stConfig.sLogReportUdpIP, 0x00, sizeof(stConfig.sLogReportUdpIP));
        strncpy(stConfig.sLogReportUdpIP, SysPairMap["LogReportIP"].c_str(), sizeof(stConfig.sLogReportUdpIP));
        std::cout << "set LogReportUdpIP to: " << stConfig.sLogReportUdpIP << endl;
    }
    else
    {
        std::cout << "调用详情上报日志IP必须配置" << endl;
        return -1;
    }

    if (SysPairMap.count("LogReportPort"))
    {
        stConfig.iLogReportUdpPort = atoi(SysPairMap["LogReportPort"].c_str());
        std::cout << "set LogReportPort to: " << stConfig.iLogReportUdpPort<< endl;
    }
    else
    {
        std::cout << "调用详情上报日志端口必须配置" << endl;
        return -1;
    }
    if (SysPairMap.count("ServerMonitorIP"))
    {
        memset(stConfig.sServerMonitorUdpIP, 0x00, sizeof(stConfig.sServerMonitorUdpIP));
        strncpy(stConfig.sServerMonitorUdpIP, SysPairMap["ServerMonitorIP"].c_str(), sizeof(stConfig.sServerMonitorUdpIP));
        std::cout << "set ServerMonitorIP to: " << stConfig.sServerMonitorUdpIP << endl;
    }
    else
    {
        std::cout << "服务监控告警IP必须配置" << endl;
        return -1;
    }

    if (SysPairMap.count("ServerMonitorPort"))
    {
        stConfig.iServerMonitorUdpPort = atoi(SysPairMap["ServerMonitorPort"].c_str());
        std::cout << "set LogReportPort to: " << stConfig.iServerMonitorUdpPort<< endl;
    }
    else
    {
        std::cout << "服务监控告警端口必须配置" << endl;
        return -1;
    }
    if (SysPairMap.count("MaxProcessNump"))
    {
        stConfig.iMaxProcessNump = atoi(SysPairMap["MaxProcessNump"].c_str());
        std::cout << "set MaxProcessNump to: " << stConfig.iMaxProcessNump<< endl;
    }
    else
    {
        std::cout << "最大子进程总体阀值的分子" << endl;
        return -1;
    }
    if (SysPairMap.count("MaxProcessDenomp"))
    {
        stConfig.iMaxProcessDenomp = atoi(SysPairMap["MaxProcessDenomp"].c_str());
        std::cout << "set MaxProcessDenomp to: " << stConfig.iMaxProcessDenomp<< endl;
    }
    else
    {
        std::cout << "最大子进程总体阀值的分母" << endl;
        return -1;
    }
    if (stConfig.iMaxProcessNump <= 0 || stConfig.iMaxProcessDenomp <= 0){
        std::cout << "最大子进程总体上阀值配置错误" << endl;
        return -1;
    }

	return  0;

}

void InitSOConfig( )
{
	
	const int LINE_LENGTH = 1024;
	char buf[LINE_LENGTH];
       char ConfigFile[1024];
	
	char paramName[128];
	char paramValue[1024];
	    
	memset(ConfigFile, 0x00, sizeof(ConfigFile));
	sprintf(ConfigFile, "../etc/%s",  "so.conf");
	cout << "CFT config file :" <<ConfigFile <<endl;
	//open the configuration file
	ifstream in(ConfigFile);
	assert(in);

	while (!in.eof()) {
		memset(buf, 0x00, sizeof(buf));
		in.getline(buf, LINE_LENGTH);
		if(buf[0] == '\0' || buf[0] == '#') {
			//neglect the blank and commemt line
			continue;
		}

		memset(paramName, 0x00, sizeof(paramName));
		memset(paramValue, 0x00, sizeof(paramValue));
		if(sscanf(buf, "%s : %s", paramName, paramValue) < 2) {
			std::cout << "CFT configuration syntax error!" << endl << buf << endl;
			continue;
		}
		std::cout << "name: " << paramName << ", value: " << paramValue << endl;
	
		// store into map
		SOValuePairMap[paramName] = paramValue;	
		
	}

	return  ;

}


void InitAuthorIpConfig( )
{
	
	const int LINE_LENGTH = 4096;
	char buf[LINE_LENGTH];
       char ConfigFile[1024];
	
	char paramName[128];
	char paramValue[4096];
	    
	memset(ConfigFile, 0x00, sizeof(ConfigFile));
	sprintf(ConfigFile, "../etc/%s",  "author_ip.conf");
	cout << "AuthorIp config file :" <<ConfigFile <<endl;
	//open the configuration file
	ifstream in(ConfigFile);
	assert(in);

	while (!in.eof()) {
		memset(buf, 0x00, sizeof(buf));
		in.getline(buf, LINE_LENGTH);
		if(buf[0] == '\0' || buf[0] == '#') {
			//neglect the blank and commemt line
			continue;
		}

		memset(paramName, 0x00, sizeof(paramName));
		memset(paramValue, 0x00, sizeof(paramValue));
		if(sscanf(buf, "%s : %s", paramName, paramValue) < 2) {
			std::cout << "AuthorIp configuration syntax error!" << endl << buf << endl;
			continue;
		}
		string ipkey(paramName);
		string ipval(paramValue);
		std::cout << "ipkey: " << ipkey << ", ipval: " << ipval << endl;
		size_t index = 0;
		while (index < ipval.length()){
			size_t found = ipval.find(";",index);
			string one;
			if(found == string::npos){
				one = ipval.substr(index,ipval.length()-index);
				index = string::npos;
			}else{
				one = ipval.substr(index,found-index);
				index = found +1;
			}
			AuthorIpPairMap[one] = ipkey;
		}
	}
	map<string,string>::iterator it = AuthorIpPairMap.begin();
	while(it != AuthorIpPairMap.end()){
		std::cout << "ip: " << it->first << ", tag: " << it->second << endl;
		it++;
	}


	return  ;

}

int
UserCustInit(void)
{
    char   caConfPath[MAX_FILEPATH_LEN];
    char  *szHomePath;
    int iLen, iRet;
   	
    /**murydu modify begin***/
    memset(caConfPath, 0, sizeof(caConfPath));

    snprintf(caConfPath, sizeof(caConfPath)-1, "../etc/%s", "ActiveSvr.conf");
    FILE *fp = fopen(caConfPath, "r");
    if (NULL == fp)
    {
    	  printf("配置文件不存在或者不能读取[%s]!\n",caConfPath);
	  return -1;
    }
    fclose(fp);

    if (InitSysConfig(caConfPath) < 0)
    {
    	  printf("初始化配置文件失败!\n");
	  SysPairMap.clear();
         return -1;
    }
 

   snprintf(stConfig.sLogFilePath,  sizeof(stConfig.sLogFilePath)-1, "%s",  tmpLogPath);
   snprintf(stConfig.sPidFilePath,  sizeof(stConfig.sLogFilePath)-1, "%s",  tmpPidPath);
   snprintf(stConfig.sBusinessSoPath,  sizeof(stConfig.sBusinessSoPath)-1, "%s",  tmpSoPath);

   InitSOConfig( );  //动态库初始化
 
   InitAuthorIpConfig( );  //授权ip
    //TLib_Log_LogMsg("******************************SysConfig Start******************************");

   //zhichenlei 从配置文件读初始化IpcKey的配置
   iRet = InitIpcKey(const_cast<char*>(SysPairMap["InitIpcKeyFile"].c_str()) );

    if (iRet != E_OK)
    {
        fprintf(stderr, "InitIpcKey Error\n");
        return E_FAIL;
    }


    TLib_Log_LogMsg("SysStatus=[%d]", stConfig.m_iSysStatus);
    TLib_Log_LogMsg("BindIP=[%s]", stConfig.szBindIP);
    TLib_Log_LogMsg("ListenPort[%d]", stConfig.iPort);

    TLib_Log_LogMsg("******************************SysConfig End********************************");

    return 0;

}


int Write2App(int * iNewSocket,int iResult, int iErrCode, char *pchErrMsg)
{
	char   MsgSent[MAX_ANS_LEN+1]   ;	
	memset(MsgSent, 0x00, sizeof(MsgSent));  
	CParam::setParam(MsgSent, "Result", iResult, true);
	CParam::setParam(MsgSent, "ErrCode", iErrCode);
	CParam::setParam(MsgSent, "ErrMsg",pchErrMsg);
//	strcpy(pchErrMsg, MsgSent);
	CParam::AddNewline(MsgSent);
	int ret =Send(iNewSocket, MsgSent, strlen(MsgSent), 0);
    
	
	return OK;
}

/*zhichenlei 2011114
 *直接由业务逻辑返回结果
 */
int Write2App(int * iNewSocket, char *oparam)
{
	char   MsgSent[MAX_ANS_LEN+1]   ;
	memset(MsgSent, 0x00, sizeof(MsgSent));
	strcpy(MsgSent, oparam);
	CParam::AddNewline(MsgSent);
    int ret=	Send(iNewSocket, MsgSent, strlen(MsgSent), 0);

	return ret;
}

/*zhichenlei 20121213 超长字符串返回
 */
int WriteString2App(int * iNewSocket, string &longOparam)
{
	longOparam.append("\r\n");
	int ret=	Send(iNewSocket, longOparam.data(), longOparam.length(), 0);
	return ret;
}

/* the request process routine */
void 
ReceiveProcessRequest(unsigned long processSn,string &clientIP,int clientPort)
{
//    char          Request[MAX_REQ_LEN + 1];

//  modify by jameszhou in 2012-06-19  
//兼容无上限接收请求串
	char	*Request;
//end modify
    char          MsgSentByServer[MAX_ANS_LEN+1]   ;	
    int           iRet, iCmdCode = 0,   iSubId=0;	
    int           iReqLength=0;
    struct timeval stBegin, stEnd;
    char       szComment[200];
    char      szCommand[100];

	//  modify by jameszhou in 2012-06-19  
	// char          AfterReturnOparam[MAX_ANS_LEN+1] ; //不为空返回关闭socket后调用再次调用soMyProcessAfterReturn
	
    char	RequestTmp[1024];
	//兼容无上限接收请求串
		char	*AfterReturnOparam;
	//end modify

    //zhichen 20131204 ip限制
    string iptag = "";
    SetGlobeFun("KBMS.ClIENTIP.TAG",(void*)iptag.c_str());
    map<string,string>::iterator it = AuthorIpPairMap.find(clientIP);
    if(it == AuthorIpPairMap.end()){
        TLib_Log_LogMsg("Child process=[%d],clientIp=[%s],clientPort=[%d],IpLimitEnable=[%d],IpTag=[%s],INVALID connection nowtime=[%d]",getpid(),clientIP.c_str(),clientPort,stConfig.iIpLimitEnable,"",time(NULL));
        if(1==stConfig.iIpLimitEnable){
        	Write2App(&iNewSocket,"result=-1006&msg=Invalid Client");
        	return ;
        }
    }else{
    	iptag = it->second;
    	SetGlobeFun("KBMS.ClIENTIP.TAG",(void*)iptag.c_str()); //20131204客户端IP类型 配置文件
    	TLib_Log_LogMsg("Child process=[%d],clientIp=[%s],clientPort=[%d],IpLimitEnable=[%d],IpTag=[%s],OKVALID connection nowtime=[%d]",getpid(),clientIP.c_str(),clientPort,stConfig.iIpLimitEnable,iptag.c_str(),time(NULL));
    }


    memset(&stBegin, 0x00, sizeof(stBegin));
    memset(&stEnd, 0x00, sizeof(stEnd)); 
    BankRecordCommTime(&stBegin, &stEnd);
    /* 收请求包 */
    alarm(stConfig.iConnectionTimeOut);    	
    
	string reqMsg;

    if(Recv(&iNewSocket, reqMsg,  0) != OK)
    {
        /* time out or socket was closed */
        alarm(0);
        CloseSocket(&iNewSocket);
        TLib_Log_LogMsg("Child process=[%d],sn=[%d],end read socket nowtime=[%d] socket receive error or time out",getpid(),processSn,time(NULL));
        return;
    }

// modify by jameszhou  
	int iLen = reqMsg.length() + 1;
	


//20120625  jameszhou
//这里设定为reqMsg长度会出问题，因为返回串有可能在so里面新增参数内容，因此需要适当加长一点
	AfterReturnOparam = new char[ iLen + 1024 ];
	memset( AfterReturnOparam , 0x00, iLen+ 1024 );
	strcpy( AfterReturnOparam, reqMsg.c_str());
//end modify

    alarm(0);

    char  szCommandType[128]; //input1 或type值
    char  szClientType[128];//input4
	if(iLen<1024){
		memset(RequestTmp, 0x00, sizeof(RequestTmp) );
		strcpy(RequestTmp, reqMsg.c_str());
		TLib_Log_LogMsg("MESSAGE:Child 1024process=[%d],sn=[%d],end read socket nowtime=[%d],REQUEST=[%s]",getpid(),processSn, time(NULL), RequestTmp);
		memset(szCommand, 0x00, sizeof(szCommand));
		if( CParam::getParam(RequestTmp, "command", szCommand, 100) != 0)
		{
			Write2App(&iNewSocket,"result=-1004&msg=System Busy");
			TLib_Log_LogMsg("Child 1024process=[%d],sn=[%d],REQUEST=[%s] not find command",getpid(),processSn, RequestTmp);

			//modify by jameszhou
			//delete []Request;
			delete []AfterReturnOparam;
			//end modify
			return;
		}
		//zhichen 20120210取type(input1)值，监控用
		memset(szCommandType, 0x00, sizeof(szCommandType));
		if( CParam::getParam(RequestTmp, "input1", szCommandType, 128) != 0){
			memset(szCommandType, 0x00, sizeof(szCommandType));
			if( CParam::getParam(RequestTmp, "type", szCommandType, 128) != 0){
				memset(szCommandType, 0x00, sizeof(szCommandType));
			}
		}
		//取input4的值 用于鉴别客户端类型

		memset(szClientType, 0x00, sizeof(szClientType));
		if( CParam::getParam(RequestTmp, "input4", szClientType, 128) != 0){
			memset(szClientType, 0x00, sizeof(szClientType));
			if( CParam::getParam(RequestTmp, "from", szClientType, 128) != 0){
				memset(szClientType, 0x00, sizeof(szClientType));
			}
		}


	}else{
		Request = new char[ iLen ];
		memset(Request, 0x00, iLen );
		strcpy(Request, reqMsg.c_str());
		TLib_Log_LogMsg("MESSAGE:Child process=[%d],sn=[%d],end read socket nowtime=[%d],REQUEST=[%s]",getpid(),processSn, time(NULL), Request);
		memset(szCommand, 0x00, sizeof(szCommand));
		if( CParam::getParam(Request, "command", szCommand, 100) != 0)
		{
			Write2App(&iNewSocket,"result=-1004&msg=System Busy");
			TLib_Log_LogMsg("Child process=[%d],sn=[%d],REQUEST=[%s] not find command",getpid(),processSn, Request);

			//modify by jameszhou
			delete []Request;
			delete []AfterReturnOparam;
			//end modify
			return;
		}
    //zhichen 20120210取type(input1)值，监控用
		memset(szCommandType, 0x00, sizeof(szCommandType));
		if( CParam::getParam(Request, "input1", szCommandType, 128) != 0){
			memset(szCommandType, 0x00, sizeof(szCommandType));
			if( CParam::getParam(Request, "type", szCommandType, 128) != 0){
				memset(szCommandType, 0x00, sizeof(szCommandType));
			}
		}
    //取input4的值 用于鉴别客户端类型
		memset(szClientType, 0x00, sizeof(szClientType));
		if( CParam::getParam(Request, "input4", szClientType, 128) != 0){
			memset(szClientType, 0x00, sizeof(szClientType));
			if( CParam::getParam(Request, "from", szClientType, 128) != 0){
				memset(szClientType, 0x00, sizeof(szClientType));
			}
		}

	}




    memset(MsgSentByServer, 0x00, sizeof(MsgSentByServer));
    memset(AfterReturnOparam, 0x00,  iLen+ 1024 );
    memset(szComment, 0x00, sizeof(szComment));

    //zhichen从配置读取so的目录
    string soTmpPath = SysPairMap["BusinessSoPath"] +  "/%s.so";
    sprintf(szComment, soTmpPath.c_str(),szCommand);

    //20111218 zhichen设置进程开始处理时间标记
    SemLock(iPCSoHandleSemID);
	memset(psProcessSoHandleInfo+iProcessID,0x00,sizeof(ProcessHandleInfo));
    psProcessSoHandleInfo[iProcessID].processId = getpid();
    psProcessSoHandleInfo[iProcessID].processBegTime = time(NULL);
	psProcessSoHandleInfo[iProcessID].processEndTime = 0;
	strcpy(psProcessSoHandleInfo[iProcessID].soFileName,szCommand);
	strcpy(psProcessSoHandleInfo[iProcessID].soCommandType,szCommandType);
    SemUnlock(iPCSoHandleSemID);


    //20130318 zhichen 每个so最多占用总资源的1/3
    int takeProcessCount = 0;//so占用的进程数
    int takeProcessCountForType = 0;//so特定接口占用的进程数
    int useCount = 0;
    string nowSoName(szCommand);
    string nowTypeName(szCommandType);
	for(int i =0; i < stConfig.iMaxProcessNum ; i++){
		  if (psProcessSoHandleInfo[i].processId == 0 )
			  continue;
		  useCount++;
		  string tmpSoName(psProcessSoHandleInfo[i].soFileName);
		  string tmpTypeName(psProcessSoHandleInfo[i].soCommandType);
		  if (tmpSoName == nowSoName){
			  takeProcessCount ++;
			  if(tmpTypeName == nowTypeName)
				  takeProcessCountForType++;
		  }
	}


	//以KBMS开始的变量名只能为常量，每次调用设置，无需析构，否则存在内存问题
	//业务so自己判断是否超过频率
	SetGlobeFun("KBMS.ClIENTIP",(void*)clientIP.c_str()); //20130815客户端IP
	SetGlobeFun("KBMS.COMMAND",(void*)nowSoName.c_str()); //20130924so名字
	SetGlobeFun("KBMS.COMMANDTYPE",(void*)nowTypeName.c_str()); //20130924so接口名字
	char takeProcessCountStr[64];
	memset(takeProcessCountStr, 0x00, sizeof(takeProcessCountStr));
	sprintf(takeProcessCountStr, "%d", takeProcessCount);
	SetGlobeFun("KBMS.COMMAND.CURRENTCALLNUM",(void*)takeProcessCountStr); //20130924so名字当前调用进程数
	char takeProcessCountForTypeStr[64];
	memset(takeProcessCountForTypeStr, 0x00, sizeof(takeProcessCountForTypeStr));
	sprintf(takeProcessCountForTypeStr, "%d", takeProcessCountForType);
	SetGlobeFun("KBMS.COMMANDTYPE.CURRENTCALLNUM",(void*)takeProcessCountForTypeStr); //20130924so名字特定接口当前调用进程数


	//20140104 供callother时统计用
	g_orgClientIp = clientIP;
	g_orgClientPort = clientPort;
	g_orgClientTag = iptag;
	g_orgCommand = szCommand;
	g_orgCommandType = szCommandType;
    char  paraTmp[128];
    memset(paraTmp, 0x00, sizeof(paraTmp));
    if(iLen<1024){
    	g_orgFromType = "";
		if( CParam::getParam(RequestTmp, "fromtype", paraTmp, 128) == 0){
			g_orgFromType = paraTmp;
		}
		memset(paraTmp, 0x00, sizeof(paraTmp));
		g_orgFromUserid = "";
		if( CParam::getParam(RequestTmp, "fromuserid", paraTmp, 128) == 0){
			g_orgFromUserid = paraTmp;
		}
		memset(paraTmp, 0x00, sizeof(paraTmp));
		g_orgFromToolid = "";
		if( CParam::getParam(RequestTmp, "fromtoolid", paraTmp, 128) == 0){
			g_orgFromToolid = paraTmp;
		}

		memset(paraTmp, 0x00, sizeof(paraTmp));
		g_fromorgcommand = "";
		if( CParam::getParam(RequestTmp, "fromorgcommand", paraTmp, 128) == 0){
			g_fromorgcommand = paraTmp;
		}

		memset(paraTmp, 0x00, sizeof(paraTmp));
		g_fromorgcommandtype = "";
		if( CParam::getParam(RequestTmp, "fromorgcommandtype", paraTmp, 128) == 0){
			g_fromorgcommandtype = paraTmp;
		}

		memset(paraTmp, 0x00, sizeof(paraTmp));
		g_fromorgcalltype = "";
		if( CParam::getParam(RequestTmp, "fromorgcalltype", paraTmp, 128) == 0){
			g_fromorgcalltype = paraTmp;
		}

		memset(paraTmp, 0x00, sizeof(paraTmp));
		g_fromorguin = "";
		if( CParam::getParam(RequestTmp, "uin", paraTmp, 128) != 0){
			memset(paraTmp, 0x00, sizeof(paraTmp));
			if( CParam::getParam(RequestTmp, "qq", paraTmp, 128) != 0){
				memset(paraTmp, 0x00, sizeof(paraTmp));
				if( CParam::getParam(RequestTmp, "input5", paraTmp, 128) != 0){
					memset(paraTmp, 0x00, sizeof(paraTmp));
				}
			}
		}
		g_fromorguin = paraTmp;

		char  LongparaTmp[1280];
		memset(LongparaTmp, 0x00, sizeof(LongparaTmp));
		g_http_referer = "";
		if( CParam::getParam(RequestTmp, "http_referer", LongparaTmp, sizeof(LongparaTmp)) == 0){
			g_http_referer = LongparaTmp;
		}

    }else{
		g_orgFromType = "";
		if( CParam::getParam(Request, "fromtype", paraTmp, 128) == 0){
			g_orgFromType = paraTmp;
		}
		memset(paraTmp, 0x00, sizeof(paraTmp));
		g_orgFromUserid = "";
		if( CParam::getParam(Request, "fromuserid", paraTmp, 128) == 0){
			g_orgFromUserid = paraTmp;
		}
		memset(paraTmp, 0x00, sizeof(paraTmp));
		g_orgFromToolid = "";
		if( CParam::getParam(Request, "fromtoolid", paraTmp, 128) == 0){
			g_orgFromToolid = paraTmp;
		}

		memset(paraTmp, 0x00, sizeof(paraTmp));
		g_fromorgcommand = "";
		if( CParam::getParam(Request, "fromorgcommand", paraTmp, 128) == 0){
			g_fromorgcommand = paraTmp;
		}

		memset(paraTmp, 0x00, sizeof(paraTmp));
		g_fromorgcommandtype = "";
		if( CParam::getParam(Request, "fromorgcommandtype", paraTmp, 128) == 0){
			g_fromorgcommandtype = paraTmp;
		}

		memset(paraTmp, 0x00, sizeof(paraTmp));
		g_fromorgcalltype = "";
		if( CParam::getParam(Request, "fromorgcalltype", paraTmp, 128) == 0){
			g_fromorgcalltype = paraTmp;
		}

		memset(paraTmp, 0x00, sizeof(paraTmp));
		g_fromorguin = "";
		if( CParam::getParam(Request, "uin", paraTmp, 128) != 0){
			memset(paraTmp, 0x00, sizeof(paraTmp));
			if( CParam::getParam(Request, "qq", paraTmp, 128) != 0){
				memset(paraTmp, 0x00, sizeof(paraTmp));
				if( CParam::getParam(Request, "input5", paraTmp, 128) != 0){
					memset(paraTmp, 0x00, sizeof(paraTmp));
				}
			}
		}
		g_fromorguin = paraTmp;

		char  LongparaTmp[1280];
		memset(LongparaTmp, 0x00, sizeof(LongparaTmp));
		g_http_referer = "";
		if( CParam::getParam(Request, "http_referer", LongparaTmp, sizeof(LongparaTmp)) == 0){
			g_http_referer = LongparaTmp;
		}
    }

    int ret = -1;
    string longOparam;
    TLib_Log_LogMsg("PROCESS:REJECT:Child process=[%d],iMaxProcessNum[%d],iMaxProcessNump[%d],iMaxProcessDenomp[%d]",psProcessSoHandleInfo[iProcessID].processId,stConfig.iMaxProcessNum, stConfig.iMaxProcessNump, stConfig.iMaxProcessDenomp);
    if (takeProcessCount > (stConfig.iMaxProcessNum * stConfig.iMaxProcessNump)/stConfig.iMaxProcessDenomp){//总体上不超过1/3
    	TLib_Log_LogMsg("PROCESS:REJECT:Child process=[%d],sn=[%d],so=[%s],Begin call so,Set info to psProcessHandleInfo[%d],begtime[%d],takeProcessCount[%d],useCount[%d]",psProcessSoHandleInfo[iProcessID].processId,processSn,szCommand,iProcessID,psProcessSoHandleInfo[iProcessID].processBegTime,takeProcessCount,useCount);

    	longOparam = "result=-1005&msg=Reject Request";
    	iRet = -1005; //告警平台根据-1005进行告警配置
    }else{
    	TLib_Log_LogMsg("PROCESS:CALLDLL:Child process=[%d],sn=[%d],so=[%s],Begin call so,Set info to psProcessHandleInfo[%d],begtime[%d],takeProcessCount[%d],useCount[%d]",psProcessSoHandleInfo[iProcessID].processId,processSn,szCommand,iProcessID,psProcessSoHandleInfo[iProcessID].processBegTime,takeProcessCount,useCount);
    	if(iLen<1024){
    		iRet=  CallDllFun(szComment, szCommand, "MyProcess", RequestTmp, MsgSentByServer,AfterReturnOparam,longOparam);
    	}else{
    		iRet=  CallDllFun(szComment, szCommand, "MyProcess", Request, MsgSentByServer,AfterReturnOparam,longOparam);
    	}
    }

    if (longOparam.length() > 0){
    	ret = WriteString2App(&iNewSocket,longOparam);
    }else{
    	ret = Write2App(&iNewSocket,MsgSentByServer);
    	longOparam = MsgSentByServer;
    }


    if (iNewSocket >=0 ){
   	 close(iNewSocket);
   	 iNewSocket = -1;
    }

    //20120210发送监控UDP消息,在业务逻辑处理完，释放标记前发送
    memset(&stEnd, 0x00, sizeof(stEnd));
    char monitormsg[10240];
    memset(monitormsg, 0x00,sizeof(monitormsg));
    int costTime = BankRecordCommTime(&stBegin, &stEnd);
    sprintf(monitormsg,"lid=KBMS_LOGSERVER_MSG&Command=%s&Commandtype=%s&FunctionRet=%d&ServerTime=%ld&ServerIP=%s&ServerPort=%d&ClientIP=%s&ClientPort=%d&ClientType=%s&CostTime=%d&ClientIPTag=%s&FromType=%s&FromUserId=%s&FromToolId=%s&OrgCommand=%s&OrgCommandtype=%s&FromOrgCommand=%s&FromOrgCommandtype=%s&FromOrgCalltype=%s&FromOrgUin=%s",
    		szCommand,szCommandType,iRet,time(NULL),stConfig.szBindIP,stConfig.iPort,clientIP.c_str(),clientPort,szClientType,costTime,iptag.c_str(),g_orgFromType.c_str(),g_orgFromUserid.c_str(),g_orgFromToolid.c_str(),"","",g_fromorgcommand.c_str(),g_fromorgcommandtype.c_str(),g_fromorgcalltype.c_str(),g_fromorguin.c_str());
    //SendMonitorMsg2(monitormsg);

    string reportMsg(monitormsg);
    reportMsg.append("||###||");
    if(iLen<1024){
        reportMsg.append(RequestTmp);
    }else{
    	reportMsg.append(Request);
    }

    reportMsg.append("||###||");
    reportMsg.append(longOparam);
    reportMsg.append("\r\n");
    SendReportMsg(reportMsg);


    //20120312 判断是否需再次调用so的MyProcessAfterReturn方法
    if ( (NULL != AfterReturnOparam) &&  ( strlen(AfterReturnOparam) > 0 )  ){
    	//对客户端响应没有影响，所以不对结果进行判断,
    	CallDllFun2(szComment, szCommand, "MyProcessAfterReturn",AfterReturnOparam);
    }

    //20111218 zhichen处理完成，释放标记
    SemLock(iPCSoHandleSemID);
    memset(psProcessSoHandleInfo+iProcessID,0x00,sizeof(ProcessHandleInfo));
    SemUnlock(iPCSoHandleSemID);

    memset(&stEnd, 0x00, sizeof(stEnd));
    if (longOparam.length()>1024){
    	TLib_Log_LogMsg("MESSAGE:Child process=[%d],sn=[%d],MyProcessRet=[%d],nowtime=[%d],ANS=[to long][CostTime=%d][Send return:%d]",getpid(),processSn, iRet,time(NULL), BankRecordCommTime(&stBegin, &stEnd), ret);
    }else{
    	TLib_Log_LogMsg("MESSAGE:Child process=[%d],sn=[%d],MyProcessRet=[%d],nowtime=[%d],ANS=[%s][CostTime=%d][Send return:%d]",getpid(),processSn, iRet,time(NULL), longOparam.c_str(), BankRecordCommTime(&stBegin, &stEnd), ret);
    }

 
    alarm(0);
/* Process body ends  */

//modify by jameszhou
    if(iLen>=1024){
    	delete []Request;
    }
	delete []AfterReturnOparam;
//end modify
    return ;
}

//zhichenlei 20111209释放so的全局资源
void FreeSoGlobeValue(void *handle,string soname)
{
	if (handle == NULL)
		return;

    string cmd = soname.substr(0,soname.length() - 3);
    //检查是否有该SO的全局变量
    int have = 0;
    map<string, void *>::iterator it  =  ProcessGlobeValueMap.begin();
	 for (; it != ProcessGlobeValueMap.end(); it++ ){
		 if (it->first.find(cmd) == 0){
			 have = 1;
			 break;
		 }
	 }
	 if (have == 0){
		 TLib_Log_LogMsg("Child process[%d] ,so =[%s] ,no globe value in ProcessGlobeValueMap",getpid(), soname.c_str());
		 return;
	 }

	 //调用SO方法释放该so所有全局变量
	 dlfreefun =(int (*)(string,void* (*)(string)) ) dlsym(handle,"MyFreeGlobeFun");
	 char *dlError = dlerror();
	 if ( dlError)  {
		TLib_Log_LogMsg("Child process[%d] ,so =[%s] ,call MyFreeGlobeFun error[%s]",getpid(), soname.c_str(),dlerror());
	 }else{
		int freeret =(*dlfreefun)("",GetGlobeFun);
		TLib_Log_LogMsg("Child process[%d] ,so =[%s] ,call MyFreeGlobeFun result[%d]",getpid(), soname.c_str(),freeret);
	 }

	 //删除全局变量
	 it  =  ProcessGlobeValueMap.begin();
	 for (; it != ProcessGlobeValueMap.end(); ){
	 	if (it->first.find(cmd) == 0){
	 		ProcessGlobeValueMap.erase(it++);
	 	}else{
            ++it;
	 	}
	 }
}


void ChildSoUpdate()
{
	//检查本子进程是否有so需要更新
	int id = -1;
	string soname (psProcessSoUpdateInfo->soFileName);
	int childpid = getpid();
	for (int i =0; i < stConfig.iMaxProcessNum ; i++){
		  if (psProcessSoUpdateInfo->processId[i] == childpid ){
			  id = i;
			  break;
		  }
	}
	//调用so的方法释放资源(释放方法由SO自己定义)
	if (id >= 0){
		TLib_Log_LogMsg("Child process[%d] ,update so =[%s] begin",getpid(), soname.c_str());
		//检查是否缓存了SO
		map<string, void *> ::iterator it;
	    it = ProcessSoMap.find(soname);
		if( it != ProcessSoMap.end() ){
			void * handle =  it->second;
			//释放该进程该so的全局资源
			FreeSoGlobeValue(handle,soname);
			//释放该so
			dlclose(handle);
			ProcessSoMap.erase(it);
			TLib_Log_LogMsg("Child process[%d] ,delete in ProcessSoMap so =[%s] ok",getpid(), soname.c_str());
		}

		SemLock(iPCSoSemID);
		psProcessSoUpdateInfo->processId[id] = 0;
		SemUnlock(iPCSoSemID);
		TLib_Log_LogMsg("Child process[%d] ,update so =[%s] end",getpid(), soname.c_str());
	}
}


/* the SIGUSR2 child handler */
void
Sigusr2ChildHandler(int sig)
{
	//该进程正在处理业务
    if (iNewSocket >= 0)
        return;
    TLib_Log_LogMsg("Child process[%d] begin Sigusr2ChildHandler .",getpid());
    ChildSoUpdate();
    TLib_Log_LogMsg("Child process[%d] end   Sigusr2ChildHandler .",getpid());

}


/* the main child process */
void 
ChildProcess()
{
    struct sockaddr_in stSockAddr;
    int iSockAddrSize;
  
    char szErrMsg[1024];
    int iRet;
    sigset_t iSigset;
    
    unsigned long iSn = 0;

    signal(SIGCHLD, SIG_IGN);

    iNewSocket = -1;
  
    TLib_Log_LogMsg("Child process [%d] start...", getpid());
  /* install the SIGALRM handler */
    if(Signal(SIGALRM, SigalrmHandler, 0, 0, -1) == SIG_ERR)
    {
        TLib_Log_LogMsg("SIGALRM handler install error");
        UserCustAbnormalExit();
        exit(-1);
    }

    /* install the SIGUSR1 handler */
    if(Signal(SIGUSR1, Sigusr1ChildHandler, 0, 1, -1) == SIG_ERR)
    {
        TLib_Log_LogMsg("SIGUSR1 handler install error");
        UserCustAbnormalExit();
        exit(-1);
    }
    
    if(Signal(SIGUSR2, Sigusr2ChildHandler, 0, 1, -1) == SIG_ERR)
    {
        TLib_Log_LogMsg("SIGUSR2 handler install error");
        UserCustAbnormalExit();
        exit(-1);
    }

    sigemptyset(&iSigset);
    sigaddset(&iSigset, SIGUSR1);
    sigaddset(&iSigset, SIGUSR2);
    
    /* attach the process count shared memory */
    shmdt(piProcessCount);
    if((piProcessCount = (u_short *)shmat(iPCShmID, NULL, 0)) == (u_short *)-1){
        TLib_Log_LogMsg("shared memory attach error");
        UserCustAbnormalExit();
        exit(-1);
    }
    
    /* attach the update so shared memory */
    shmdt(psProcessSoUpdateInfo);
    if((psProcessSoUpdateInfo = (SoUpdateInfo *)shmat(iPCSoShmID, NULL, 0)) == (SoUpdateInfo *)-1){
        TLib_Log_LogMsg("shared update so memory attach error");
        UserCustAbnormalExit();
        exit(-1);
    }

    /* attach the handle so shared memory */
    shmdt(psProcessSoHandleInfo);
    if((psProcessSoHandleInfo = (ProcessHandleInfo *)shmat(iPCSoHandleShmID, NULL, 0)) == (ProcessHandleInfo *)-1){
        TLib_Log_LogMsg("shared handle so memory attach error");
        UserCustAbnormalExit();
        exit(-1);
    }



    iSockAddrSize = sizeof(stSockAddr);
    
    /******************************************/
    /* We Start to Process Requests Here!     */
    /******************************************/
    for(;;)
    {
        memset(&stSockAddr, 0, iSockAddrSize);
        //检查so是否要更新
        ChildSoUpdate();
        //20111216zhichen需注册信号防止业务处理逻辑重置了SIGALRM
        if(Signal(SIGALRM, SigalrmHandler, 0, 0, -1) == SIG_ERR)
        {
            TLib_Log_LogMsg("SIGALRM_1 handler install error");
            UserCustAbnormalExit();
            exit(-1);
        }

        /* needn't to lock if there is 4.4BSD kernel? */
        SAFE_ACCEPT(SemLock(iAcceptSempID));
        iNewSocket = Accept(iListenfd, (struct sockaddr *)&stSockAddr, &iSockAddrSize);
        SAFE_ACCEPT(SemUnlock(iAcceptSempID));
        string clientIP = inet_ntoa(stSockAddr.sin_addr);
        int clientPort = htons(stSockAddr.sin_port);

        TLib_Log_LogMsg("Child process=[%d],clientIp=[%s],clientPort=[%d],received a connection nowtime=[%d]",getpid(),clientIP.c_str(),clientPort,time(NULL));

        memset(g_szClientIP, 0x00, sizeof(g_szClientIP));
        strcpy(g_szClientIP, inet_ntoa(stSockAddr.sin_addr));
       
        if (iNewSocket >= 0)
        {
            /* block the SIGUSR1 SIGUSR2 to prevent dead lock */
            sigprocmask(SIG_BLOCK, &iSigset, NULL);
            /* increase the process count */
            SemLock(iPCShmSempID);
            piProcessCount[0]++;
            SemUnlock(iPCShmSempID);
            piProcessCount[iProcessID+1] = 1;
            //Unlock(iPCShmSempID);
            //sigprocmask(SIG_UNBLOCK, &iSigusr1_set, NULL);
            
            /* receive and process the requests */
             ReceiveProcessRequest(++iSn,clientIP,clientPort);
            
             if (iNewSocket >=0 ){
            	 close(iNewSocket);
            	 iNewSocket = -1;
             }
            
            /* block the SIGUSR1  SIGUSR2 to prevent dead lock */
            //sigprocmask(SIG_BLOCK, &iSigusr1_set, NULL);    
            /* decrease the process count */
            SemLock(iPCShmSempID);
            piProcessCount[0]--;
            SemUnlock(iPCShmSempID);
            piProcessCount[iProcessID+1] = 0;
            //Unlock(iPCShmSempID);
            sigprocmask(SIG_UNBLOCK, &iSigset, NULL);
        }
        else
        {
            /* sleep a proper time while network error occured */
            sleep(1);
        }
    }

    return ;
}

