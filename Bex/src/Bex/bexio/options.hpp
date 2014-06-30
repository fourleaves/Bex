#ifndef __BEX_IO_OPTIONS_HPP__
#define __BEX_IO_OPTIONS_HPP__

//////////////////////////////////////////////////////////////////////////
/// ��ѡ������
#include <Bex/config.hpp>

namespace Bex { namespace bexio
{
    //////////////////////////////////////////////////////////////////////////
    /// @{ ѡ��
    // ���ӳɹ�\�Ͽ�����\�յ����� ������Ϣ֪ͨ�߼��̵߳ķ�ʽ
    enum BEX_ENUM_CLASS notify_logic_thread_em
    {
        nlt_reactor,        ///< reactorģʽ, ����Ϣpost��ָ����ɶ�����.
        nlt_loop,           ///< �߼��߳���ѯsession
    };
    typedef notify_logic_thread_em nlte;

    // ���ӳɹ�\�Ͽ�����\�յ����� ������Ϣ�߼��̵߳���Ӧ��ʽ
    enum BEX_ENUM_CLASS message_logic_process_em
    {
        mlp_callback,       ///< ע��ص�����
        mlp_derived,        ///< �̳�sessioin����д��Ϣ��Ӧ����
        mlp_both,           ///< ���з�ʽһ����Ч
    };
    typedef message_logic_process_em mlpe;

    // ���ͻ��������������
    enum BEX_ENUM_CLASS send_buffer_overflow_em
    {
        sbo_interrupt,      ///< �Ͽ�����
        sbo_wait,           ///< �߼���ȴ�
        sbo_extend,         ///< ��չ���ͻ�����(�ݲ�֧��)
    };
    typedef send_buffer_overflow_em sboe;

    // ���ջ��������������
    enum BEX_ENUM_CLASS receive_buffer_overflow_em
    {
        rbo_interrupt,      ///< �Ͽ�����
        rbo_wait,           ///< ���ղ�ȴ�
        rbo_extend,         ///< ��չ���ջ�����(�ݲ�֧��)
    };
    typedef receive_buffer_overflow_em rboe;
    /// @}
    //////////////////////////////////////////////////////////////////////////

    struct options
    {
        // ���ӳɹ�\�Ͽ�����\�յ����� ������Ϣ֪ͨ�߼��̵߳ķ�ʽ
        notify_logic_thread_em nlte_;

        // ���ͻ��������������
        send_buffer_overflow_em sboe_;

        // ���ջ��������������
        receive_buffer_overflow_em rboe_;

        // ���ӳɹ�\�Ͽ�����\�յ����� ������Ϣ�߼��̵߳���Ӧ��ʽ
        message_logic_process_em mlpe_;

        /// �������÷���(Test)
        static options test()
        {
            static options const opts = {
                nlte::nlt_reactor,
                sboe::sbo_wait,
                rboe::rbo_wait,
                mlpe::mlp_derived
                };
            return opts;
        }

        /// �߲�������������������Ƽ����÷���
        static options multi_session_server()
        {
            static options const opts = {
                nlte::nlt_reactor,
                sboe::sbo_interrupt,
                rboe::rbo_interrupt,
                mlpe::mlp_derived
                };
            return opts;
        }

        /// �ȶ������ȵ��Ƽ����÷���
        static options stability()
        {
            static options const opts = {
                nlte::nlt_reactor,
                sboe::sbo_extend,
                rboe::rbo_extend,
                mlpe::mlp_derived
                };
            return opts;
        }

        /// ���������������������Ƽ����÷���(�ͻ��˺��ʺ�ʹ�����)
        static options throughput()
        {
            static options const opts = {
                nlte::nlt_loop,
                sboe::sbo_extend,
                rboe::rbo_extend,
                mlpe::mlp_derived
                };
            return opts;
        }
    };

} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_OPTIONS_HPP__
