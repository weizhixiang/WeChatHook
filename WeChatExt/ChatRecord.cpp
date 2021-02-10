#include "ChatRecord.h"
#include <Windows.h>
#include <string>
#include "Util.h"
#include "offset.h"
#include "struct.h"
#include "WSClient.h"
#include "EVString.h"
#include "Command.h"
#include "Loger.h"
#include <Shlwapi.h>

DWORD recieveMsgCall, recieveMsgJmpAddr;
BOOL g_AutoChat = FALSE;	//是否自动聊天
BOOL isSendTuLing = FALSE;	//是否已经发给了图灵机器人
BOOL isText = TRUE;			//是否是文字消息
DWORD r_esp = 0;
DWORD r_eax = 0;
std::wstring GetMsgByAddress(DWORD memAddress);
string Wstr2Str(wstring wstr)
{
	if (wstr.length() == 0)
		return "";

	std::string str;
	str.assign(wstr.begin(), wstr.end());
	return str;
}

void ReceiveMsgProc(LPVOID Context)
{
	recieveMsgStruct* msg = (recieveMsgStruct*)Context;

	WriteInfo("收到消息1");
	//todo:处理自动功能(自动收款、自动加名片等)
	neb::CJsonObject data;
	//todo:fromWxid、senderWxid某些特殊消息有异常

	WriteInfo("收到消息2");
	data.Add("Type", EVString::w2a(msg->type));
	data.Add("Source", EVString::w2a(msg->source));
	data.Add("Wxid", EVString::w2a(msg->wxid));
	data.Add("MsgSender", EVString::w2a(msg->msgSender));
	data.Add("Content", EVString::w2a(msg->content));
	data.Add("IsMoney",msg->isMoney);
	delete msg;
	WriteInfo("收到消息3");
	Send(Cmd_ReceiveMessage, data);
}

/**
 * 处理拦截到的消息内容
 */
void RecieveMessageJump()
{
	recieveMsgStruct *msg = new recieveMsgStruct;
	msg->isMoney = FALSE;
	//信息块的位置
	DWORD** msgAddress = (DWORD**)r_esp;
	//消息类型
	DWORD msgType = *((DWORD*)(**msgAddress + 0x30));

	BOOL isFriendMsg = FALSE;		//是否是好友消息
	BOOL isImageMessage = FALSE;	//是否是图片消息
	BOOL isRadioMessage = FALSE;	//是否是视频消息
	BOOL isVoiceMessage = FALSE;	//是否是语音消息
	BOOL isBusinessCardMessage = FALSE;	//是否是名片消息
	BOOL isExpressionMessage = FALSE;	//是否是名片消息
	BOOL isLocationMessage = FALSE;	//是否是位置消息
	BOOL isSystemMessage = FALSE;	//是否是系统或红包消息
	BOOL isFriendRequestMessage = FALSE;	//是否是好友请求消息
	BOOL isOther = FALSE;	//是否是其他消息


	switch (msgType)
	{
	case 0x01:
		memcpy(msg->type, L"文字", sizeof(L"文字"));
		break;
	case 0x03:
		memcpy(msg->type, L"图片", sizeof(L"图片"));
		isImageMessage = TRUE;
		break;
	case 0x22:
		memcpy(msg->type, L"语音", sizeof(L"语音"));
		isVoiceMessage = TRUE;
		break;
	case 0x25:
		memcpy(msg->type, L"好友确认", sizeof(L"好友确认"));
		isFriendRequestMessage = TRUE;
		break;
	case 0x28:
		memcpy(msg->type, L"POSSIBLEFRIEND_MSG", sizeof(L"POSSIBLEFRIEND_MSG"));
		isOther = TRUE;
		break;
	case 0x2A:
		memcpy(msg->type, L"名片", sizeof(L"名片"));
		isBusinessCardMessage = TRUE;
		break;
	case 0x2B:
		memcpy(msg->type, L"视频", sizeof(L"视频"));
		isRadioMessage = TRUE;
		break;
	case 0x2F:
		//石头剪刀布
		memcpy(msg->type, L"表情", sizeof(L"表情"));
		isExpressionMessage = TRUE;
		break;
	case 0x30:
		memcpy(msg->type, L"位置", sizeof(L"位置"));
		isLocationMessage = TRUE;
		break;
	case 0x31:
		//共享实时位置
		//文件
		//转账
		//链接
		//收款
		memcpy(msg->type, L"共享实时位置、文件、转账、链接", sizeof(L"共享实时位置、文件、转账、链接"));
		isOther = TRUE;
		break;
	case 0x32:
		memcpy(msg->type, L"VOIPMSG", sizeof(L"VOIPMSG"));
		isOther = TRUE;
		break;
	case 0x33:
		memcpy(msg->type, L"微信初始化", sizeof(L"微信初始化"));
		isOther = TRUE;
		break;
	case 0x34:
		memcpy(msg->type, L"VOIPNOTIFY", sizeof(L"VOIPNOTIFY"));
		isOther = TRUE;
		break;
	case 0x35:
		memcpy(msg->type, L"VOIPINVITE", sizeof(L"VOIPINVITE"));
		isOther = TRUE;
		break;
	case 0x3E:
		memcpy(msg->type, L"小视频", sizeof(L"小视频"));
		isRadioMessage = TRUE;
		break;
	case 0x270F:
		memcpy(msg->type, L"SYSNOTICE", sizeof(L"SYSNOTICE"));
		isOther = TRUE;
		break;
	case 0x2710:
		//系统消息
		//红包
		memcpy(msg->type, L"红包、系统消息", sizeof(L"红包、系统消息"));
		isSystemMessage = TRUE;
		break;
	default:
		isOther = TRUE;
		break;
	}
	//消息内容
	wstring fullmessgaedata = GetMsgByAddress(**msgAddress + 0x68);	//完整的消息内容
	//判断消息来源是群消息还是好友消息
	wstring msgSource2 = L"<msgsource />\n";
	wstring msgSource = L"";
	msgSource.append(GetMsgByAddress(**msgAddress + 0x198));
	//好友消息
	if (msgSource.length() <= msgSource2.length())
	{
		memcpy(msg->source, L"好友消息", sizeof(L"好友消息"));
		isFriendMsg = TRUE;
	}
	else
	{
		//群消息
		memcpy(msg->source, L"群消息", sizeof(L"群消息"));
	}
	//发送消息
	if ((int)*((DWORD*)(**msgAddress + 0x34))) {
		//memcpy(msg->type, L"发送消息", sizeof(L"发送消息"));
		memcpy(msg->source, L"自己", sizeof(L"自己"));
	}
	//显示微信ID/群ID
	LPVOID pWxid = *((LPVOID*)(**msgAddress + 0x40));
	swprintf_s(msg->wxid, L"%s", (wchar_t*)pWxid);

	//如果是群消息
	if (isFriendMsg == FALSE)
	{
		//显示消息发送者
		LPVOID pSender = *((LPVOID*)(**msgAddress + 0x144));
		swprintf_s(msg->msgSender, L"%s", (wchar_t*)pSender);
	}
	else
	{
		memcpy(msg->msgSender, L"NULL", sizeof(L"NULL"));
	}


	//显示消息内容  过滤无法显示的消息 防止奔溃
	if (StrStrW(msg->wxid, L"gh"))
	{
		//如果是图灵机器人发来的消息 并且消息已经发送给图灵机器人
		if ((StrCmpW(msg->wxid, L"gh_ab370b2e4b62") == 0) && isSendTuLing == TRUE)
		{
			wchar_t tempcontent[0x200] = { 0 };
			//首先判断机器人回复的消息类型 如果不是文字 直接回复
			if (msgType != 0x01)
			{
				//SendTextMessage(tempwxid, (wchar_t*)L"啦啦啦");
				isSendTuLing = FALSE;
			}
			//再次判断发送给机器人的消息类型
			else if (isText == FALSE)
			{
				//SendTextMessage(tempwxid, (wchar_t*)L"亲 不支持此类消息哦 请发文字 么么哒");
				isSendTuLing = FALSE;
				isText = TRUE;
			}
			else   //如果是文字 再次判断长度
			{
				//接着拿到消息内容 发送给好友
				LPVOID pContent = *((LPVOID*)(**msgAddress + 0x68));
				swprintf_s(tempcontent, L"%s", (wchar_t*)pContent);
				//判断返回的消息是否过长
				if (wcslen(tempcontent) > 0x100)
				{
					wcscpy_s(tempcontent, wcslen(L"啦啦啦"), L"啦啦啦");
				}

				//SendTextMessage(tempwxid, tempcontent);
				isSendTuLing = FALSE;
			}
		}
		//如果微信ID为gh_3dfda90e39d6 说明是收款消息
		else if ((StrCmpW(msg->wxid, L"gh_3dfda90e39d6") == 0))
		{
			swprintf_s(msg->content, L"%s", L"微信收款到账");
			msg->isMoney = TRUE;
		}
		else
		{
			//如果微信ID中带有gh 说明是公众号
			swprintf_s(msg->content, L"%s", L"公众号发来推文,请在手机上查看");
		}
	}
	//过滤图片消息 
	else if (isImageMessage == TRUE)
	{
		swprintf_s(msg->content, L"%s", L"收到图片消息,请在手机上查看");
	}
	else if (isRadioMessage == TRUE)
	{
		swprintf_s(msg->content, L"%s", L"收到视频消息,请在手机上查看");
	}
	else if (isVoiceMessage == TRUE)
	{
		swprintf_s(msg->content, L"%s", L"收到语音消息,请在手机上查看");
	}
	else if (isBusinessCardMessage == TRUE)
	{
		swprintf_s(msg->content, L"%s", L"收到名片消息,已自动添加好友");
		//自动添加好友
		//AutoAddCardUser(fullmessgaedata);
	}
	else if (isExpressionMessage == TRUE)
	{
		swprintf_s(msg->content, L"%s", L"收到表情消息,请在手机上查看");
	}
	//自动通过好友请求
	else if (isFriendRequestMessage == TRUE)
	{
		swprintf_s(msg->content, L"%s", L"收到好友请求,已自动通过");
		//自动通过好友请求
		//AutoAgreeUserRequest(fullmessgaedata);
	}
	else if (isOther == TRUE)
	{
		//取出消息内容
		wchar_t tempcontent[0x10000] = { 0 };
		LPVOID pContent = *((LPVOID*)(**msgAddress + 0x68));
		swprintf_s(tempcontent, L"%s", (wchar_t*)pContent);
		//判断是否是转账消息
		if (StrStrW(tempcontent, L"微信转账"))
		{
			swprintf_s(msg->content, L"%s", L"收到转账消息,已自动收款");

			//自动收款
			//AutoCllectMoney(fullmessgaedata, msg->wxid);	
		}
		else
		{
			//判断消息长度 如果长度超过就不显示
			if (wcslen(tempcontent) > 200)
			{
				swprintf_s(msg->content, L"%s", L"消息内容过长 已经过滤");
			}
			else
			{
				//判断是否是转账消息
				swprintf_s(msg->content, L"%s", L"收到共享实时位置、文件、链接等其他消息,请在手机上查看");
			}

		}
	}
	else if (isLocationMessage == TRUE)
	{
		swprintf_s(msg->content, L"%s", L"收到位置消息,请在手机上查看");
	}
	else if (isSystemMessage == TRUE)
	{
		wchar_t tempbuff[0x1000];
		LPVOID pContent = *((LPVOID*)(**msgAddress + 0x68));
		swprintf_s(tempbuff, L"%s", (wchar_t*)pContent);

		//在这里处理加入群聊消息
		if ((StrStrW(tempbuff, L"移出了群聊") || StrStrW(tempbuff, L"加入了群聊")))
		{
			wcscpy_s(msg->content, wcslen(tempbuff) + 1, tempbuff);
		}
		else
		{
			swprintf_s(msg->content, L"%s", L"收到红包或系统消息,请在手机上查看");
		}

	}
	//过滤完所有消息之后
	else
	{
		wchar_t tempbuff[0x1000];
		LPVOID pContent = *((LPVOID*)(**msgAddress + 0x68));
		swprintf_s(tempbuff, L"%s", (wchar_t*)pContent);
		//判断消息长度 如果长度超过就不显示
		if (wcslen(tempbuff) > 200)
		{
			swprintf_s(msg->content, L"%s", L"消息内容过长 已经过滤");
		}
		else
		{
			swprintf_s(msg->content, L"%s", (wchar_t*)pContent);
		}

	}
	//MessageBox(NULL, msg->content, L"错误", MB_OK);
	//wchar_t msgOut[0x500];
	//swprintf_s(msgOut, L"%s,%s,%s,%s,%s,%d\r\n", msg->type, msg->source, msg->wxid, msg->msgSender, msg->content, msg->isMoney);
	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReceiveMsgProc, msg, 0, NULL);
}


/**
 * 被写入到hook点的接收消息裸函数
 */
__declspec(naked) void RecieveMsgDeclspec()
{
	//保存现场
	__asm
	{
		//补充被覆盖的代码
		mov ecx, recieveMsgCall

		//提取esp寄存器内容，放在一个变量中
		mov r_esp, esp
		mov r_eax, eax

		pushad
		pushfd
	}
	RecieveMessageJump();
	//恢复现场
	__asm
	{
		popfd
		popad
		//跳回被HOOK指令的下一条指令
		jmp recieveMsgJmpAddr
	}
}

void HookChatRecord() {
	//HOOK接收消息
	DWORD recieveMsgHookAddr = GetWeChatWinBase() + RECIEVEHOOKADDR-5;
	recieveMsgCall = GetWeChatWinBase() + RECIEVEHOOKCALL;
	recieveMsgJmpAddr = recieveMsgHookAddr + 5;
	BYTE msgJmpCode[5] = { 0xE9 };
	*(DWORD*)&msgJmpCode[1] = (DWORD)RecieveMsgDeclspec - recieveMsgHookAddr - 5;
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)recieveMsgHookAddr, msgJmpCode, 5, NULL);
}
//************************************************************
// 函数名称: GetMsgByAddress
// 函数说明: 从地址中获取消息内容
// 作    者: GuiShou
// 时    间: 2019/7/6
// 参    数: DWORD memAddress  目标地址
// 返 回 值: LPCWSTR	消息内容
//************************************************************
std::wstring GetMsgByAddress(DWORD memAddress)
{
	wstring tmp;
	DWORD msgLength = *(DWORD*)(memAddress + 4);
	if (msgLength > 0) {
		WCHAR* msg = new WCHAR[msgLength + 1];
		wmemcpy_s(msg, msgLength + 1, (WCHAR*)(*(DWORD*)memAddress), msgLength + 1);
		tmp = msg;
		delete[]msg;
	}
	return  tmp;
}