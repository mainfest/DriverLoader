
// DriverLoaderDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "DriverLoader.h"
#include "DriverLoaderDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDriverLoaderDlg 对话框



CDriverLoaderDlg::CDriverLoaderDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DRIVERLOADER_DIALOG, pParent)
	, m_Edit_Path(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDriverLoaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MFCEDITBROWSE1, m_Edit_Path);
	DDX_Control(pDX, IDC_MFCEDITBROWSE1, m_edit_browse);
}

BEGIN_MESSAGE_MAP(CDriverLoaderDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_INSTALL, &CDriverLoaderDlg::OnBnClickedButtonInstall)
	ON_BN_CLICKED(IDC_BUTTON_START, &CDriverLoaderDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CDriverLoaderDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_UNINSTALL, &CDriverLoaderDlg::OnBnClickedButtonUninstall)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CDriverLoaderDlg 消息处理程序

BOOL CDriverLoaderDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	GetDlgItem(IDC_STATIC_tip)->SetWindowText(_T(""));//设置控件文本内容  

																			   //调用ChangeWindowMessageFilter函数，放行WM_DROPFILES消息和WM_COPYGLOBALDATA消息，以解决Win7系统中文件拖放失效的问题
	DragAcceptFiles(TRUE);
	ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
	ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);       // 0x0049 == WM_COPYGLOBALDATA

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CDriverLoaderDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CDriverLoaderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//安装
void CDriverLoaderDlg::OnBnClickedButtonInstall()
{
	// TODO: 在此添加控件通知处理程序代码

	//1. 使用OpenSCManager函数打开SCM  
	m_hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (m_hServiceMgr == NULL)
	{
		MessageBox(_T("OpenSCManager Error"), _T("Error"));
		CloseServiceHandle(m_hServiceMgr);
		return;
	}
	//2.使用CreateService函数利用SCM句柄创建一个服务  
	m_hServiceDDK = CreateService(
		m_hServiceMgr,//SMC句柄  
		_T("HadesService"),//驱动服务名称(驱动程序的在注册表中的名字)  
		_T("HadesService"),//驱动服务显示名称(注册表驱动程序的DisplayName值)  
		SERVICE_ALL_ACCESS,//权限(所有访问权限)  
		SERVICE_KERNEL_DRIVER,//服务类型(驱动程序)  
		SERVICE_DEMAND_START,//启动方式(需要时启动,注册表驱动程序的Start值)  
		SERVICE_ERROR_IGNORE,//错误控制(忽略,注册表驱动程序的ErrorControl值)  
		m_Edit_Path,//服务的二进制文件路径(驱动程序文件路径, 注册表驱动程序的ImagePath值)  
		NULL,//加载组命令  
		NULL,//TagId(指向一个加载顺序的标签值)  
		NULL,//依存关系  
		NULL,//服务启动名  
		NULL);//密码  
	if (m_hServiceDDK == NULL)
	{
		//如果创建错误,关闭句柄  
		CloseServiceHandle(m_hServiceDDK);
		CloseServiceHandle(m_hServiceMgr);
		MessageBox(_T("Install Drive Fail"), _T("Error"));
		return;
	}
	CloseServiceHandle(m_hServiceDDK);
	CloseServiceHandle(m_hServiceMgr);
	GetDlgItem(IDC_STATIC_tip)->SetWindowText(_T("Driver has been installed"));//设置控件文本内容  
}

//启动
void CDriverLoaderDlg::OnBnClickedButtonStart()
{
	// TODO: 在此添加控件通知处理程序代码

	//使用OpenSCManager函数打开SCM  
	m_hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (m_hServiceMgr == NULL)
	{
		MessageBox(_T("OpenSCManager Error"), _T("Error"));
		CloseServiceHandle(m_hServiceMgr);
		return;
	}
	//打开服务获得句柄  
	m_hServiceDDK = OpenService(m_hServiceMgr, _T("HadesService"), SERVICE_START);

	//启动刚刚创建的服务  
	BOOL bRet = StartService(m_hServiceDDK, NULL, NULL);
	if (bRet == FALSE)
	{
		CloseServiceHandle(m_hServiceDDK);
		CloseServiceHandle(m_hServiceMgr);
		MessageBox(_T("Start Service Fail"), _T("Error"));
		return;
	}
	CloseServiceHandle(m_hServiceDDK);
	CloseServiceHandle(m_hServiceMgr);
	GetDlgItem(IDC_STATIC_tip)->SetWindowText(_T("The driver has started"));//设置控件文本内容  
}

//停止
void CDriverLoaderDlg::OnBnClickedButtonStop()
{
	// TODO: 在此添加控件通知处理程序代码

	//使用OpenSCManager函数打开SCM  
	m_hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (m_hServiceMgr == NULL)
	{
		MessageBox(_T("OpenSCManager Error"), _T("Error"));
		CloseServiceHandle(m_hServiceMgr);
		return;
	}
	//打开服务获得句柄  
	m_hServiceDDK = OpenService(m_hServiceMgr, _T("HadesService"), SERVICE_STOP);

	//停止驱动服务  
	SERVICE_STATUS svcsta = { 0 };
	BOOL bRet = ControlService(m_hServiceDDK, SERVICE_CONTROL_STOP, &svcsta);
	if (bRet == FALSE)
	{
		CloseServiceHandle(m_hServiceDDK);
		CloseServiceHandle(m_hServiceMgr);
		MessageBox(_T("Stop Service Fail"), _T("Error"));
		return;
	}
	CloseServiceHandle(m_hServiceDDK);
	CloseServiceHandle(m_hServiceMgr);
	GetDlgItem(IDC_STATIC_tip)->SetWindowText(_T("The drive has stopped"));//设置控件文本内容  
}

//卸载
void CDriverLoaderDlg::OnBnClickedButtonUninstall()
{
	// TODO: 在此添加控件通知处理程序代码

	//使用OpenSCManager函数打开SCM  
	m_hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (m_hServiceMgr == NULL)
	{
		MessageBox(_T("OpenSCManager Error"), _T("Error"));
		CloseServiceHandle(m_hServiceMgr);
		return;
	}
	//打开服务获得句柄  
	m_hServiceDDK = OpenService(m_hServiceMgr, _T("HadesService"), SERVICE_STOP | DELETE);
	//删除驱动服务  
	BOOL bRet = DeleteService(m_hServiceDDK);
	if (bRet == FALSE)
	{
		MessageBox(_T("UnInstall Service Fail"), _T("Error"));
		CloseServiceHandle(m_hServiceDDK);
		CloseServiceHandle(m_hServiceMgr);
		return;
	}
	GetDlgItem(IDC_STATIC_tip)->SetWindowText(_T("Driver has been uninstalled"));//设置控件文本内容  
	CloseServiceHandle(m_hServiceDDK);
	CloseServiceHandle(m_hServiceMgr);
}



void CDriverLoaderDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnDropFiles(hDropInfo);

	TCHAR szPath[MAX_PATH] = {};
	//获取拖拽文件的路径
	DragQueryFile(hDropInfo, 0, szPath, MAX_PATH);
	m_Edit_Path = szPath;
	//将路径添加到文件框中
	m_edit_browse.SetWindowText(m_Edit_Path);
	//拖放结束后，释放内存
	DragFinish(hDropInfo);
	UpdateData(TRUE);
}
