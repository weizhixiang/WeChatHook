#include <Windows.h>
#include "offset.h"
#include "Util.h"

/**
 * 检查微信登录状态
 * @param
 * @return int
 */
bool CheckLogin()
{
	int* islogin=(int*)(GetWeChatWinBase() + ISLOGIN);
	return *islogin == 1;
}


/**
 * 退出微信
 * @param
 * @return void
 */
void LogoutWeChat()
{
	DWORD dwCallAddress = GetWeChatWinBase() + LOGOUT;
	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)dwCallAddress, 0, NULL, 0);
	if (hThread != 0)
	{
		CloseHandle(hThread);
	}
}


/**
 * 跳转到二维码
 * @param
 * @return void
 */
VOID GotoQrCode()
{
	DWORD dwCallAddr1 = GetWeChatWinBase() + GOTOQRCODE1;
	DWORD dwCallAddr2 = GetWeChatWinBase() + GOTOQRCODE2;

	__asm {
		call dwCallAddr1;
		mov ecx, eax;
		call dwCallAddr2;
	}

}
