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
//so?????????????????????????????? ???so??????????????????MyFreeGlobeFun
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

//zhichen20111205 ?????????so?????????????????????MAP,KEY ???????????????SO?????????(???????????????)+ ????????? ??????CmdEchoDemo.GLOBE
map<string, void *> ProcessGlobeValueMap;
//zhichen20111205 ??????SO??????MAP,????????????
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


// del = 1 ?????????????????????  ?????? NULL

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

//??????udp????????????,???logserver
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
//??????udp????????????,???logserver
int SendMonitorMsg2(const string &msg)
{
	if (0 == sendMonitorFlag)
		return 0;
	int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	int res = SendMonitorMsg2(sock_fd,msg);
	close(sock_fd);
	return res;
}


//???????????????so??????so
//??????????????????
string CallOtherSo(string req)
{
//modify by jameszhou in 2012-06-18 ??????????????????????????????????????????????????????

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
	char  szCommandType[128]; //input1 ??????type
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

//CallDllFun????????????????????????socket???????????????MyProcessAfterReturn so???
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
 *??????errmsg???opram,???????????????????????????result ???msg ????????????
 *????????????????????????????????????
 *
 *AfterReturnOparam:??????????????????MyProcess??????????????????????????????????????????socket????????? MyProcessAfterReturn
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

	//so??????????????????
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

	//so????????????????????????
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
		std::cout << "??????IP????????????" << endl;
		return -1;
	}

	if (SysPairMap.count("ListenPort")) 
	{
 		stConfig.iPort = atoi(SysPairMap["ListenPort"].c_str());
		std::cout << "set ListenPort to: " << stConfig.iPort<< endl;
	}
	else
	{
		std::cout << "????????????????????????" << endl;
		return -1;
	}

	if (SysPairMap.count("ProcessNum")) 
	{
 		stConfig.iProcessNum= atoi(SysPairMap["ProcessNum"].c_str());
		std::cout << "set ProcessNum to: " << stConfig.iProcessNum<< endl;
	}
	else
	{
		std::cout << "???????????????????????????" << endl;
		return -1;
	}

	if (SysPairMap.count("MaxProcessNum")) 
	{
 		stConfig.iMaxProcessNum= atoi(SysPairMap["MaxProcessNum"].c_str());
		std::cout << "set MaxProcessNum to: " << stConfig.iMaxProcessNum<< endl;
	}
	else
	{
		std::cout << "?????????????????????????????????" << endl;
		return -1;
	}

	if (stConfig.iMaxProcessNum > MAX_PROCESS_COUNT){
		std::cout << "?????????MaxProcessNum????????????" << MAX_PROCESS_COUNT << endl;
		return -1;
	}



	if (SysPairMap.count("ForkInterval")) 
	{
 		stConfig.iForkInterval= atoi(SysPairMap["ForkInterval"].c_str());
		std::cout << "set ForkInterval to: " << stConfig.iForkInterval<< endl;
	}
	else
	{
		std::cout << "?????????????????????????????????" << endl;
		return -1;
	}

	if (SysPairMap.count("KillInterval")) 
	{
 		stConfig.iKillInterval= atoi(SysPairMap["KillInterval"].c_str());
		std::cout << "set KillInterval to: " << stConfig.iKillInterval<< endl;
	}
	else
	{
		std::cout << "?????????????????????????????????" << endl;
		return -1;
	}

	if (SysPairMap.count("ForkExtraThreshold")) 
	{
 		stConfig.iForkExtraThreshold= atoi(SysPairMap["ForkExtraThreshold"].c_str());
		std::cout << "set ForkExtraThreshold to: " << stConfig.iForkExtraThreshold<< endl;
	}
	else
	{
		std::cout<< "????????????????????????????????????" << endl;
		return -1;
	}

	if (SysPairMap.count("FreeTimeBeforeKill")) 
	{
 		stConfig.iFreeTimeBeKill= atoi(SysPairMap["FreeTimeBeforeKill"].c_str());
		std::cout << "set FreeTimeBeforeKill to: " << stConfig.iFreeTimeBeKill<< endl;
	}
	else
	{
		std::cout << "??????????????????????????????????????????" << endl;
		return -1;
	}

	if (SysPairMap.count("FatherSleepTime")) 
	{
 		stConfig.iFatherSleepTime= atoi(SysPairMap["FatherSleepTime"].c_str());
		std::cout << "set FatherSleepTime to: " << stConfig.iFatherSleepTime<< endl;
	}
	else
	{
		std::cout << "?????????????????????????????????" << endl;
		return -1;
	}

	if (SysPairMap.count("SysStatus")) 
	{
 		stConfig.m_iSysStatus= atoi(SysPairMap["SysStatus"].c_str());
		std::cout << "set SysStatus to: " << stConfig.m_iSysStatus<< endl;
	}
	else
	{
		std::cout << "????????????????????????" << endl;
		return -1;
	}

	if (SysPairMap.count("ConnectionTimeOut")) 
	{
 		stConfig.iConnectionTimeOut= atoi(SysPairMap["ConnectionTimeOut"].c_str());
		std::cout << "set ConnectionTimeOut to: " << stConfig.iConnectionTimeOut<< endl;
	}
	else
	{
		std::cout << "????????????????????????" << endl;
		return -1;
	}

	if (SysPairMap.count("ProcessHandleTimeOut"))
	{
 		stConfig.iProcessHandleTimeOut= atoi(SysPairMap["ProcessHandleTimeOut"].c_str());
		std::cout << "set ProcessHandleTimeOut to: " << stConfig.iProcessHandleTimeOut<< endl;
	}
	else
	{
		std::cout << "????????????????????????????????????" << endl;
		return -1;
	}

	if (SysPairMap.count("MaxLogSize"))
	{
 		stConfig.lMaxLogSize= atol(SysPairMap["MaxLogSize"].c_str());
		std::cout << "set MaxLogSize to: " << stConfig.lMaxLogSize<< endl;
	}
	else
	{
		std::cout << "?????????????????????????????????" << endl;
		return -1;
	}

	if (SysPairMap.count("MaxLogNum"))
	{
 		stConfig.iMaxLogNum= atoi(SysPairMap["MaxLogNum"].c_str());
		std::cout << "set MaxLogNum to: " << stConfig.iMaxLogNum<< endl;
	}
	else
	{
		std::cout << "???????????????????????????????????????" << endl;
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
		std::cout << "pid??????????????????" << endl;
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
		std::cout << "????????????????????????" << endl;
		return -1;
	}

	if (SysPairMap.count("CountShmKey")) 
	{
 		stConfig.iCountShmKey= atoi(SysPairMap["CountShmKey"].c_str());
		std::cout << "set CountShmKey to: " << stConfig.iCountShmKey<< endl;
	}
	else
	{
		std::cout << "????????????????????????key????????????" << endl;
		return -1;
	}

	if (SysPairMap.count("CountSemKey")) 
	{
 		stConfig.iCountSemKey= atoi(SysPairMap["CountSemKey"].c_str());
		std::cout << "set CountSemKey to: " << stConfig.iCountSemKey<< endl;
	}
	else
	{
		std::cout << "?????????????????????key????????????" << endl;
		return -1;
	}

	if (SysPairMap.count("AcceptSemKey")) 
	{
 		stConfig.iAcceptSemKey= atoi(SysPairMap["AcceptSemKey"].c_str());
		std::cout << "set AcceptSemKey to: " << stConfig.iAcceptSemKey<< endl;
	}
	else
	{
		std::cout << "?????????????????????key????????????" << endl;
		return -1;
	}

	if (SysPairMap.count("InitIpcKeyFile"))
	{
		std::cout << "set InitIpcKeyFile  to: " << SysPairMap["InitIpcKeyFile"].c_str() << endl;
	}
	else
	{
		std::cout << "Ipc key ???????????????" << endl;
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
		std::cout << "??????so?????? ????????????" << endl;
		return -1;
	}

	//so??????????????????
	if (SysPairMap.count("SoUpdateShmKey"))
	{
 		stConfig.iSoUpdateShmKey= atoi(SysPairMap["SoUpdateShmKey"].c_str());
		std::cout << "set SoUpdateShmKey to: " << stConfig.iSoUpdateShmKey<< endl;
	}
	else
	{
		std::cout << "so??????????????????key????????????" << endl;
		return -1;
	}

	if (SysPairMap.count("SoUpdateSemKey"))
	{
 		stConfig.iSoUpdateSemKey= atoi(SysPairMap["SoUpdateSemKey"].c_str());
		std::cout << "set SoUpdateSemKey to: " << stConfig.iSoUpdateSemKey<< endl;
	}
	else
	{
		std::cout << "so???????????????key????????????" << endl;
		return -1;
	}

	//so????????????????????????
	if (SysPairMap.count("SoHandleShmKey"))
	{
 		stConfig.iSoHandleShmKey= atoi(SysPairMap["SoHandleShmKey"].c_str());
		std::cout << "set SoHandleShmKey to: " << stConfig.iSoHandleShmKey<< endl;
	}
	else
	{
		std::cout << "so????????????????????????????????????key????????????" << endl;
		return -1;
	}

	if (SysPairMap.count("SoHandleSemKey"))
	{
 		stConfig.iSoHandleSemKey= atoi(SysPairMap["SoHandleSemKey"].c_str());
		std::cout << "set SoHandleSemKey to: " << stConfig.iSoHandleSemKey<< endl;
	}
	else
	{
		std::cout << "so?????????????????????????????????key????????????" << endl;
		return -1;
	}

	if (SysPairMap.count("EnableIpLimit"))
	{
 		stConfig.iIpLimitEnable= atoi(SysPairMap["EnableIpLimit"].c_str());
		std::cout << "set EnableIpLimit to: " << stConfig.iIpLimitEnable<< endl;
	}
	else
	{
		std::cout << "????????????ip??????????????????" << endl;
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
        std::cout << "????????????????????????IP????????????" << endl;
        return -1;
    }

    if (SysPairMap.count("LogReportPort"))
    {
        stConfig.iLogReportUdpPort = atoi(SysPairMap["LogReportPort"].c_str());
        std::cout << "set LogReportPort to: " << stConfig.iLogReportUdpPort<< endl;
    }
    else
    {
        std::cout << "??????????????????????????????????????????" << endl;
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
        std::cout << "??????????????????IP????????????" << endl;
        return -1;
    }

    if (SysPairMap.count("ServerMonitorPort"))
    {
        stConfig.iServerMonitorUdpPort = atoi(SysPairMap["ServerMonitorPort"].c_str());
        std::cout << "set LogReportPort to: " << stConfig.iServerMonitorUdpPort<< endl;
    }
    else
    {
        std::cout << "????????????????????????????????????" << endl;
        return -1;
    }
    if (SysPairMap.count("MaxProcessNump"))
    {
        stConfig.iMaxProcessNump = atoi(SysPairMap["MaxProcessNump"].c_str());
        std::cout << "set MaxProcessNump to: " << stConfig.iMaxProcessNump<< endl;
    }
    else
    {
        std::cout << "????????????????????????????????????" << endl;
        return -1;
    }
    if (SysPairMap.count("MaxProcessDenomp"))
    {
        stConfig.iMaxProcessDenomp = atoi(SysPairMap["MaxProcessDenomp"].c_str());
        std::cout << "set MaxProcessDenomp to: " << stConfig.iMaxProcessDenomp<< endl;
    }
    else
    {
        std::cout << "????????????????????????????????????" << endl;
        return -1;
    }
    if (stConfig.iMaxProcessNump <= 0 || stConfig.iMaxProcessDenomp <= 0){
        std::cout << "??????????????????????????????????????????" << endl;
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
    	  printf("???????????????????????????????????????[%s]!\n",caConfPath);
	  return -1;
    }
    fclose(fp);

    if (InitSysConfig(caConfPath) < 0)
    {
    	  printf("???????????????????????????!\n");
	  SysPairMap.clear();
         return -1;
    }
 

   snprintf(stConfig.sLogFilePath,  sizeof(stConfig.sLogFilePath)-1, "%s",  tmpLogPath);
   snprintf(stConfig.sPidFilePath,  sizeof(stConfig.sLogFilePath)-1, "%s",  tmpPidPath);
   snprintf(stConfig.sBusinessSoPath,  sizeof(stConfig.sBusinessSoPath)-1, "%s",  tmpSoPath);

   InitSOConfig( );  //??????????????????
 
   InitAuthorIpConfig( );  //??????ip
    //TLib_Log_LogMsg("******************************SysConfig Start******************************");

   //zhichenlei ???????????????????????????IpcKey?????????
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
 *?????????????????????????????????
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

/*zhichenlei 20121213 ?????????????????????
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
//??????????????????????????????
	char	*Request;
//end modify
    char          MsgSentByServer[MAX_ANS_LEN+1]   ;	
    int           iRet, iCmdCode = 0,   iSubId=0;	
    int           iReqLength=0;
    struct timeval stBegin, stEnd;
    char       szComment[200];
    char      szCommand[100];

	//  modify by jameszhou in 2012-06-19  
	// char          AfterReturnOparam[MAX_ANS_LEN+1] ; //?????????????????????socket?????????????????????soMyProcessAfterReturn
	
    char	RequestTmp[1024];
	//??????????????????????????????
		char	*AfterReturnOparam;
	//end modify

    //zhichen 20131204 ip??????
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
    	SetGlobeFun("KBMS.ClIENTIP.TAG",(void*)iptag.c_str()); //20131204?????????IP?????? ????????????
    	TLib_Log_LogMsg("Child process=[%d],clientIp=[%s],clientPort=[%d],IpLimitEnable=[%d],IpTag=[%s],OKVALID connection nowtime=[%d]",getpid(),clientIP.c_str(),clientPort,stConfig.iIpLimitEnable,iptag.c_str(),time(NULL));
    }


    memset(&stBegin, 0x00, sizeof(stBegin));
    memset(&stEnd, 0x00, sizeof(stEnd)); 
    BankRecordCommTime(&stBegin, &stEnd);
    /* ???????????? */
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
//???????????????reqMsg????????????????????????????????????????????????so?????????????????????????????????????????????????????????
	AfterReturnOparam = new char[ iLen + 1024 ];
	memset( AfterReturnOparam , 0x00, iLen+ 1024 );
	strcpy( AfterReturnOparam, reqMsg.c_str());
//end modify

    alarm(0);

    char  szCommandType[128]; //input1 ???type???
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
		//zhichen 20120210???type(input1)???????????????
		memset(szCommandType, 0x00, sizeof(szCommandType));
		if( CParam::getParam(RequestTmp, "input1", szCommandType, 128) != 0){
			memset(szCommandType, 0x00, sizeof(szCommandType));
			if( CParam::getParam(RequestTmp, "type", szCommandType, 128) != 0){
				memset(szCommandType, 0x00, sizeof(szCommandType));
			}
		}
		//???input4?????? ???????????????????????????

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
    //zhichen 20120210???type(input1)???????????????
		memset(szCommandType, 0x00, sizeof(szCommandType));
		if( CParam::getParam(Request, "input1", szCommandType, 128) != 0){
			memset(szCommandType, 0x00, sizeof(szCommandType));
			if( CParam::getParam(Request, "type", szCommandType, 128) != 0){
				memset(szCommandType, 0x00, sizeof(szCommandType));
			}
		}
    //???input4?????? ???????????????????????????
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

    //zhichen???????????????so?????????
    string soTmpPath = SysPairMap["BusinessSoPath"] +  "/%s.so";
    sprintf(szComment, soTmpPath.c_str(),szCommand);

    //20111218 zhichen????????????????????????????????????
    SemLock(iPCSoHandleSemID);
	memset(psProcessSoHandleInfo+iProcessID,0x00,sizeof(ProcessHandleInfo));
    psProcessSoHandleInfo[iProcessID].processId = getpid();
    psProcessSoHandleInfo[iProcessID].processBegTime = time(NULL);
	psProcessSoHandleInfo[iProcessID].processEndTime = 0;
	strcpy(psProcessSoHandleInfo[iProcessID].soFileName,szCommand);
	strcpy(psProcessSoHandleInfo[iProcessID].soCommandType,szCommandType);
    SemUnlock(iPCSoHandleSemID);


    //20130318 zhichen ??????so????????????????????????1/3
    int takeProcessCount = 0;//so??????????????????
    int takeProcessCountForType = 0;//so??????????????????????????????
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


	//???KBMS????????????????????????????????????????????????????????????????????????????????????????????????
	//??????so??????????????????????????????
	SetGlobeFun("KBMS.ClIENTIP",(void*)clientIP.c_str()); //20130815?????????IP
	SetGlobeFun("KBMS.COMMAND",(void*)nowSoName.c_str()); //20130924so??????
	SetGlobeFun("KBMS.COMMANDTYPE",(void*)nowTypeName.c_str()); //20130924so????????????
	char takeProcessCountStr[64];
	memset(takeProcessCountStr, 0x00, sizeof(takeProcessCountStr));
	sprintf(takeProcessCountStr, "%d", takeProcessCount);
	SetGlobeFun("KBMS.COMMAND.CURRENTCALLNUM",(void*)takeProcessCountStr); //20130924so???????????????????????????
	char takeProcessCountForTypeStr[64];
	memset(takeProcessCountForTypeStr, 0x00, sizeof(takeProcessCountForTypeStr));
	sprintf(takeProcessCountForTypeStr, "%d", takeProcessCountForType);
	SetGlobeFun("KBMS.COMMANDTYPE.CURRENTCALLNUM",(void*)takeProcessCountForTypeStr); //20130924so???????????????????????????????????????


	//20140104 ???callother????????????
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
    if (takeProcessCount > (stConfig.iMaxProcessNum * stConfig.iMaxProcessNump)/stConfig.iMaxProcessDenomp){//??????????????????1/3
    	TLib_Log_LogMsg("PROCESS:REJECT:Child process=[%d],sn=[%d],so=[%s],Begin call so,Set info to psProcessHandleInfo[%d],begtime[%d],takeProcessCount[%d],useCount[%d]",psProcessSoHandleInfo[iProcessID].processId,processSn,szCommand,iProcessID,psProcessSoHandleInfo[iProcessID].processBegTime,takeProcessCount,useCount);

    	longOparam = "result=-1005&msg=Reject Request";
    	iRet = -1005; //??????????????????-1005??????????????????
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

    //20120210????????????UDP??????,????????????????????????????????????????????????
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


    //20120312 ???????????????????????????so???MyProcessAfterReturn??????
    if ( (NULL != AfterReturnOparam) &&  ( strlen(AfterReturnOparam) > 0 )  ){
    	//???????????????????????????????????????????????????????????????,
    	CallDllFun2(szComment, szCommand, "MyProcessAfterReturn",AfterReturnOparam);
    }

    //20111218 zhichen???????????????????????????
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

//zhichenlei 20111209??????so???????????????
void FreeSoGlobeValue(void *handle,string soname)
{
	if (handle == NULL)
		return;

    string cmd = soname.substr(0,soname.length() - 3);
    //??????????????????SO???????????????
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

	 //??????SO???????????????so??????????????????
	 dlfreefun =(int (*)(string,void* (*)(string)) ) dlsym(handle,"MyFreeGlobeFun");
	 char *dlError = dlerror();
	 if ( dlError)  {
		TLib_Log_LogMsg("Child process[%d] ,so =[%s] ,call MyFreeGlobeFun error[%s]",getpid(), soname.c_str(),dlerror());
	 }else{
		int freeret =(*dlfreefun)("",GetGlobeFun);
		TLib_Log_LogMsg("Child process[%d] ,so =[%s] ,call MyFreeGlobeFun result[%d]",getpid(), soname.c_str(),freeret);
	 }

	 //??????????????????
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
	//???????????????????????????so????????????
	int id = -1;
	string soname (psProcessSoUpdateInfo->soFileName);
	int childpid = getpid();
	for (int i =0; i < stConfig.iMaxProcessNum ; i++){
		  if (psProcessSoUpdateInfo->processId[i] == childpid ){
			  id = i;
			  break;
		  }
	}
	//??????so?????????????????????(???????????????SO????????????)
	if (id >= 0){
		TLib_Log_LogMsg("Child process[%d] ,update so =[%s] begin",getpid(), soname.c_str());
		//?????????????????????SO
		map<string, void *> ::iterator it;
	    it = ProcessSoMap.find(soname);
		if( it != ProcessSoMap.end() ){
			void * handle =  it->second;
			//??????????????????so???????????????
			FreeSoGlobeValue(handle,soname);
			//?????????so
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
	//???????????????????????????
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
        //??????so???????????????
        ChildSoUpdate();
        //20111216zhichen????????????????????????????????????????????????SIGALRM
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

