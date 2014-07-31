#ifndef __BEX_WIN32_SERVICE__
#define __BEX_WIN32_SERVICE__

#include <Bex/config/windows.h>

#if defined(BEX_WINDOWS_API)

#include <boost/program_options.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <Bex/utility/singleton.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <iostream>

//////////////////////////////////////////////////////////////////////////
/// Win32���������
namespace Bex
{
    class WinService;
    class win_svc_base
    {
        friend class WinService;

    protected:
        typedef void (__stdcall * PServiceStart)(unsigned long, char **);
        typedef void (__stdcall * PServiceCtrl)(unsigned long);

    public:
        win_svc_base();
        virtual ~win_svc_base();

        /// ��ȡ������
        virtual const char* GetServiceName() = 0;

        /// ��ȡ������(������ʾ)
        virtual const char* GetShowName() = 0;

    protected:
        /// ����
        virtual void Run() = 0;

    protected:
        //////////////////////////////////////////////////////////////////////////
        /// ------ �ص�����

        /// ��װ�ɹ��ص�
        virtual void OnInstall(bool bAutoStart);

        /// ����ֹͣ�ص�
        virtual void ServiceStop() {}

        /// ������ͣ�ص�
        virtual void ServicePause() {}

        /// ��������ص�
        virtual void ServiceContinue() {}

        /// ���·���״̬�ص�
        virtual void OnServiceUpdateStatus() {}

        /// ���������봦��
        virtual void OnCtrlCode(unsigned long ulCtrlCode) { ulCtrlCode; }
        //////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        /// ------ ���º������û����������, ����������.
    protected:
        /// ��ȡ������ڵ㺯��ָ��
        virtual PServiceStart GetServiceStartFunction();

        /// ��ȡ������ƺ���ָ��
        virtual PServiceCtrl GetServiceCtrlFunction();

        /// ��װ����
        // @param bAutoStart �Ƿ��Զ�����
        virtual bool Install(bool bAutoStart);

        /// ж�ط���
        virtual bool Uninstall();

        /// �ر�����״̬
        bool ReportStatus(unsigned long dwCurrentState, unsigned long dwExitCode = 0
            , unsigned long dwCheckPoint = 0, unsigned long dwWaitHint = 0);
        //////////////////////////////////////////////////////////////////////////

    private:
        /// ��ں���
        int Main(int argc, char * argv[]);

        /// Ĭ�Ϸ�����ڵ㺯��
        static void __stdcall service_start(unsigned long argc, char ** argv);

        /// Ĭ�Ϸ�����ƺ���
        static void __stdcall service_ctrl(unsigned long ulCtrlCode);

    private:
        boost::program_options::variables_map m_vm;         ///< �����в���map
        boost::thread           m_taskThread;

        struct WinStructImpl;
        boost::shared_ptr<WinStructImpl> m_pWinStructImpl;
    };

    class WinService
        : public singleton<WinService>
    {
    public:
        typedef boost::shared_ptr<win_svc_base> ServicePtr;

        /// �󶨷������
        // @param pSvc ָ����ڴ��ָ��, ����WinService����������ʱ�ͷ��ڴ�.
        bool BindService(win_svc_base * pSvc)
        {
            BOOST_ASSERT( !m_pService );
            if (m_pService) 
                return false;

            m_pService.reset(pSvc);
            return true;
        }

        /// ��ں���
        int Main(int argc, char * argv[])
        {
            if (m_pService)
                return m_pService->Main(argc, argv);

            return 0;
        }

        /// ��ȡ�������ָ��
        inline ServicePtr GetServicePtr()
        {
            return m_pService;
        }

    private:
        ServicePtr m_pService;
    };
}

#include "win32_service.ipp"

#endif //defined(BEX_WINDOWS_API)

#endif //__BEX_WIN32_SERVICE__