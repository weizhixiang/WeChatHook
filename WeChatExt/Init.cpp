#include "Init.h"
#include "Command.h"
#include "WSClient.h"
#include "Util.h"
#include "Sql.h"
#include "ChatRecord.h"

bool isHook = false;
/**
 * 初始化 Hook
 * @param
 * @return void
*/
void InitWeChat() {
	neb::CJsonObject result;

	if (isHook) {
		Send(Cmd_Init, result, "", 1, L"已初始化。");
		return;
	}
	//检查当前微信版本
	try
	{
		if (IsWxVersionValid()) {

			HookDB();

			HookChatRecord();

			isHook = true;

			Send(Cmd_Init, result, "", 0, L"初始化成功。");
		}
		else
		{
			isHook = false;
			Send(Cmd_Init, result, "", 2, L"当前微信版本不匹配，请下载WeChat2.9.5.56。");
		}
	}
	catch (...)
	{
		isHook = false;
		Send(Cmd_Init, result, "", 3, L"初始化异常。");

	}
}
