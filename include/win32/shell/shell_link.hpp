#ifndef __WIN32_SHELL_LINK_HPP
#define __WIN32_SHELL_LINK_HPP


#include <windows.h>
#include <shobjidl.h>
#include <ShlGuid.h>
#include <string>
#include <vector>

namespace OS
{
	// ������ݷ�ʽ
	HRESULT CreateShortcut(LPCTSTR lpszPathLink, LPCTSTR lpszPathObj, LPCTSTR lpszDesc = NULL)
	{
		_com_ptr_t<uuid()> * psl = NULL;

		// Get a pointer to the IShellLink interface.
		HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
			IID_IShellLink, (LPVOID*)&psl);
		if (SUCCEEDED(hres))
		{
			// Set the path to the shortcut target and add the description.
			psl->SetPath(lpszPathObj);
			if (lpszDesc)
			{
				psl->SetDescription(lpszDesc);
			}		

			IPersistFile* ppf = NULL;
			// Query IShellLink for the IPersistFile interface for saving the
			// shortcut in persistent storage.
			hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

			if (SUCCEEDED(hres))
			{
				// Save the link by calling IPersistFile::Save.
				hres = ppf->Save(lpszPathLink, TRUE);
				ppf->Release();
			}
			psl->Release();
		}

		return hres;
	}

	// ��ʾϵͳ�Ҽ��˵�
	// @pszPath Ϊ�ļ���Ŀ¼·��
	BOOL ShowContextMenu(HWND hwnd, LPCTSTR pszPath, POINT point);

	// ��ʾ����ļ���Ŀ¼��ϵͳ�Ҽ��˵�
	// ����ļ���Ŀ¼����λ��ͬһĿ¼��
	BOOL ShowContextMenu(HWND hwnd, const std::vector<std::wstring> &vctFilePaths, POINT point);

	// ����ϵͳ�Ҽ��˵�����"����""����"�˵���Ӧ��Ч��ԭ��δ֪��
	BOOL PopContextMenu(HWND hwnd, POINT point, LPCONTEXTMENU pContextMenu);

	// ����ϵͳ�Ҽ��˵�������������"����""����"�����
	BOOL PopContextMenuEx(HWND hwnd, POINT point, LPCONTEXTMENU pContextMenu, LPCTSTR pszFilePath);

	// ��ʾϵͳ�ļ��е��Ҽ��˵������ҵĵ��ԣ�����վ��
	// @nFolder A CSIDL value that identifies the folder����CSIDL_DRIVES��CSIDL_BITBUCKET
	BOOL ShowSysFolerContextMenu(HWND hwnd, int nFolder, POINT point);

    BOOL ShowSysContextMenu(HWND hwnd, wchar_t* Path, POINT point);

    // ���ITEMIDLIST����Ӧ������
    //  lpVerbΪ�գ���ִ�е�һ��Ĭ������
    HRESULT SHInvokeCommand(HWND hwnd, IShellFolder* pShellFolder, LPCITEMIDLIST pidlItem, LPCSTR lpVerb);

    //  ͨ���Ҽ��˵������������
    HRESULT ShellExecute(LPCWSTR FilePath);

    HICON GetIcon(LPCITEMIDLIST pidl);

    HICON LoadIconFromShell(LPCWSTR FilePath);

	BOOL CutOrCopyFiles(LPCWSTR pszFilePath, BOOL bCopy);

	BOOL CutOrCopyFiles(const std::vector<std::wstring> &vctFiles, BOOL bCopy);

    BOOL ReadShortcut(LPCWSTR lpwLnkFile, LPWSTR lpDescFile, int length);

	BOOL GetShellFolderAndRelativeItemidlist(LPCWSTR pwszPath, IShellFolder **ppFolder, ITEMIDLIST **ppItemlist);
};


#endif