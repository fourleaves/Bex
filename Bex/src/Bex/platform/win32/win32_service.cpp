#include "win32_service.h"
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <Windows.h>

namespace Bex
{
    struct win_svc_base::WinStructImpl
    {
        SERVICE_STATUS          m_ssStatus;
        SERVICE_STATUS_HANDLE   m_ssh;
    };

    win_svc_base::win_svc_base()
        : m_pWinStructImpl( new WinStructImpl )
    {
        memset(&m_pWinStructImpl->m_ssStatus, 0, sizeof(m_pWinStructImpl->m_ssStatus));
        m_pWinStructImpl->m_ssh = SERVICE_STATUS_HANDLE();
    }

    win_svc_base::~win_svc_base()
    {

    }

    /// ��װ�ɹ��ص�
    void win_svc_base::OnInstall( bool bAutoStart )
    {
        if ( bAutoStart )
        {
            SC_HANDLE schSCMgr = ::OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
            if (!schSCMgr)
            {
                std::cout << "����������ݿ��ʧ��!" << std::endl;
                return ;
            }

            SC_HANDLE schService = ::OpenServiceA(schSCMgr, GetServiceName(), SERVICE_ALL_ACCESS);
            if (!schService)
            {
                std::cout << "�����ʧ��!" << std::endl;
                ::CloseServiceHandle(schSCMgr);
                return ;
            }

            ::StartServiceA(schService, 0, NULL);
            ::CloseServiceHandle(schService);
            ::CloseServiceHandle(schSCMgr);
        }
    }

    int win_svc_base::Main( int argc, char * argv[] )
    {
        //////////////////////////////////////////////////////////////////////////
        /// �����в�������
        boost::program_options::options_description opts;
        opts.add_options()
            ("help", "options: --install --uninstall")
            ("install", "")
            ("uninstall", "")
            ("auto-start", "");

        boost::program_options::store(
            boost::program_options::parse_command_line(argc, argv, opts)
            , m_vm );

        //////////////////////////////////////////////////////////////////////////
        /// ����SERVICE_TABLE_ENTRY DispatchTable[] �ṹ
        SERVICE_TABLE_ENTRYA DispatchTable[2] = 
        {
            {(char*)GetServiceName(), (LPSERVICE_MAIN_FUNCTIONA)GetServiceStartFunction()},                  
            {NULL, NULL}
        };

        if ( m_vm.count("help") )
        {
            /// ����
            std::cout << opts << std::endl;
        }
        else if ( m_vm.count("install") )
        {
            bool bAutoStart = (m_vm.count("auto-start") > 0);
            /// ��װ����
            if ( Install(bAutoStart) )
            {
                std::cout << "��װ�ɹ�!" << std::endl;
                OnInstall(bAutoStart);
            }
            else
                std::cout << "��װʧ��!" << std::endl;
        }
        else if ( m_vm.count("uninstall") )
        {
            /// ж�ط���
            if ( Uninstall() )
                std::cout << "ж�سɹ�!" << std::endl;
            else
                std::cout << "ж��ʧ��!" << std::endl;
        }
        else
        {
            /// OS�����ó���
            BOOL bSuccessed = ::StartServiceCtrlDispatcherA(DispatchTable);
            //if (bSuccessed)
            //    LOG << "������ں���ע��ɹ�!" << std::endl;
            //else
            //    LOG << "������ں���ע��ʧ��!" << std::endl;
        }
        return 0;
    }

    win_svc_base::PServiceStart win_svc_base::GetServiceStartFunction()
    {
        return (&win_svc_base::service_start);
    }

    win_svc_base::PServiceCtrl win_svc_base::GetServiceCtrlFunction()
    {
        return (&win_svc_base::service_ctrl);
    }

    bool win_svc_base::Install(bool bAutoStart)
    {
        SC_HANDLE schSCMgr, schService;
        char szPath[MAX_PATH] = {};
        if (!::GetModuleFileNameA(NULL, szPath, sizeof(szPath)))
            return false;

        /// �򿪷���������ݿ�
        schSCMgr = ::OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!schSCMgr)
        {
            std::cout << "����������ݿ��ʧ��!" << std::endl;
            return false;
        }

        schService = ::CreateServiceA(
            schSCMgr,                           ///< ����������ݿ���
            GetServiceName(),                   ///< ������
            GetShowName(),                      ///< ������ʾ����ı�ʶ
            SERVICE_ALL_ACCESS,                 ///< ��Ӧ���еķ�������
            SERVICE_WIN32_OWN_PROCESS,          ///< ��������(��ռһ������)
            bAutoStart ? SERVICE_AUTO_START
                : SERVICE_DEMAND_START,         ///< ��������(�ֶ�����)
            SERVICE_ERROR_NORMAL,               ///< �����������
            szPath,                             ///< �������·��
            NULL,                               ///< ���������κ���
            NULL,                               ///< û��tag��ʶ��
            NULL,                               ///< ���������������ķ���������,���������һ�����ַ���
            NULL,                               ///< LocalSystem �ʺ�
            NULL );

        if (!schService)
        {
            std::cout << "���񴴽�ʧ��!" << std::endl;
            ::CloseServiceHandle(schSCMgr);
            return false;
        }

        ::CloseServiceHandle(schService);
        ::CloseServiceHandle(schSCMgr);
        return true;
    }

    bool win_svc_base::Uninstall()
    {
        SC_HANDLE schSCMgr = ::OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!schSCMgr)
        {
            std::cout << "����������ݿ��ʧ��!" << std::endl;
            return false;
        }

        SC_HANDLE schService = ::OpenServiceA(schSCMgr, GetServiceName(), SERVICE_ALL_ACCESS);
        if (!schService)
        {
            std::cout << "�����ʧ��!" << std::endl;
            ::CloseServiceHandle(schSCMgr);
            return false;
        }

        BOOL bDeleted = ::DeleteService(schService);
        ::CloseServiceHandle(schService);
        ::CloseServiceHandle(schSCMgr);
        return (bDeleted ? true : false);
    }

    /// ������ڵ㺯��
    void __stdcall win_svc_base::service_start( unsigned long argc, char ** argv )
    {
        WinService::ServicePtr pSvc = WinService::getInstance().GetServicePtr();

        //LOG << "call " << __FUNCTION__ << std::endl;

        /// ע��Ctrl����
        pSvc->m_pWinStructImpl->m_ssh = ::RegisterServiceCtrlHandlerA(
            pSvc->GetServiceName(), pSvc->GetServiceCtrlFunction());
        if (!pSvc->m_pWinStructImpl->m_ssh)
        {
            //LOG << "Ctrl����ע��ʧ��!" << std::endl;
            return ;
        }

        /// ��ʼ������״̬
        pSvc->ReportStatus(SERVICE_START_PENDING, 0, 0, 1000);

        /// ���������߳�
        pSvc->m_taskThread = boost::thread(boost::bind(&win_svc_base::Run, pSvc));

        pSvc->ReportStatus(SERVICE_RUNNING);
    }

    /// �ر�����״̬
    bool win_svc_base::ReportStatus(unsigned long dwCurrentState, unsigned long dwExitCode
        , unsigned long dwCheckPoint, unsigned long dwWaitHint)
    {
        if (dwCurrentState == SERVICE_START_PENDING)
            m_pWinStructImpl->m_ssStatus.dwControlsAccepted = 0;
        else
            m_pWinStructImpl->m_ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

        // Initialize ss structure.
        m_pWinStructImpl->m_ssStatus.dwServiceType             = SERVICE_WIN32_OWN_PROCESS;
        m_pWinStructImpl->m_ssStatus.dwServiceSpecificExitCode = 0;
        m_pWinStructImpl->m_ssStatus.dwCurrentState            = dwCurrentState;
        m_pWinStructImpl->m_ssStatus.dwWin32ExitCode           = dwExitCode;
        m_pWinStructImpl->m_ssStatus.dwCheckPoint              = dwCheckPoint;
        m_pWinStructImpl->m_ssStatus.dwWaitHint                = dwWaitHint;

        bool bRet = (SetServiceStatus(m_pWinStructImpl->m_ssh, &m_pWinStructImpl->m_ssStatus) ? true : false);
        //LOG << "call ReportStatus dwCurrentState=" << dwCurrentState << " ret=" << bRet << std::endl;
        return bRet;
    }

    void __stdcall win_svc_base::service_ctrl( unsigned long ulCtrlCode )
    {
        WinService::ServicePtr pSvc = WinService::getInstance().GetServicePtr();

        //LOG << "call " << __FUNCTION__ << " uiCtrlCode=" << ulCtrlCode << std::endl;

        //�������������
        switch(ulCtrlCode)
        {
            //�ȸ��·���״̬Ϊ SERVICDE_STOP_PENDING,��ֹͣ����
        case SERVICE_CONTROL_STOP:
            pSvc->ReportStatus(SERVICE_STOP_PENDING,NO_ERROR,0,500);
            pSvc->ServiceStop();     //�ɾ���ķ������ʵ��
            pSvc->m_pWinStructImpl->m_ssStatus.dwCurrentState = SERVICE_STOPPED;
            break;

            //��ͣ����
        case SERVICE_CONTROL_PAUSE:
            pSvc->ReportStatus(SERVICE_PAUSE_PENDING,NO_ERROR,0,500);
            pSvc->ServicePause();    //�ɾ���ķ������ʵ��
            pSvc->m_pWinStructImpl->m_ssStatus.dwCurrentState = SERVICE_PAUSED;
            break;

            //��������
        case SERVICE_CONTROL_CONTINUE:
            pSvc->ReportStatus(SERVICE_CONTINUE_PENDING,NO_ERROR,0,500);
            pSvc->ServiceContinue(); //�ɾ���ķ������ʵ��
            pSvc->m_pWinStructImpl->m_ssStatus.dwCurrentState = SERVICE_RUNNING;
            break;

            //���·���״̬
        case SERVICE_CONTROL_INTERROGATE:
            pSvc->OnServiceUpdateStatus();
            break;

            //��Ч���Զ��������
        default:
            pSvc->OnCtrlCode(ulCtrlCode);
            break;
        }

        pSvc->ReportStatus(pSvc->m_pWinStructImpl->m_ssStatus.dwCurrentState);
    }

}