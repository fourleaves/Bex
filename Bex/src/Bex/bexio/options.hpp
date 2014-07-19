#ifndef __BEX_IO_OPTIONS_HPP__
#define __BEX_IO_OPTIONS_HPP__

//////////////////////////////////////////////////////////////////////////
/// ��ѡ������ options
#include <Bex/config.hpp>
#include <boost/shared_ptr.hpp>

namespace Bex { namespace bexio
{
    using namespace boost::asio;

    //////////////////////////////////////////////////////////////////////////
    /// @{ ѡ��
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
        //sbo_extend,         ///< ��չ���ͻ�����(�ݲ�֧��)
    };
    typedef send_buffer_overflow_em sboe;

    // ���ջ��������������
    enum BEX_ENUM_CLASS receive_buffer_overflow_em
    {
        rbo_interrupt,      ///< �Ͽ�����
        rbo_wait,           ///< �߼���ȴ�
        //rbo_extend,         ///< ��չ���ջ�����(�ݲ�֧��)
    };
    typedef receive_buffer_overflow_em rboe;
    /// @}
    //////////////////////////////////////////////////////////////////////////

    struct ssl_options;

    struct options
    {
        // ���ͻ��������������
        send_buffer_overflow_em sboe_;

        // ���ջ��������������
        receive_buffer_overflow_em rboe_;

        // ���ӳɹ�\�Ͽ�����\�յ����� ������Ϣ�߼��̵߳���Ӧ��ʽ
        message_logic_process_em mlpe_;

        // �����߳�����(0��ʾ�����������߳���)
        std::size_t workthread_count;

        // ���ͻ�����
        std::size_t send_buffer_size;
        static const std::size_t default_sbsize = 1024 * 8;
        static const std::size_t large_sbsize = 1024 * 1024 * 8;

        // ���ͻ�����
        std::size_t receive_buffer_size;
        static const std::size_t default_rbsize = 1024 * 8;
        static const std::size_t large_rbsize = 1024 * 8;

        // ÿ�������󳤶�(Ŀǰ��tcp packet/ssl packetЭ����Ч)
        std::size_t max_packet_size;
        static const std::size_t default_max_packet_size = 1024 * 8;

        // ���ŵعر����ӿɵȴ����ʱ��(���� ms)
        unsigned int shutdown_timeout;

        /// ssl����
        boost::shared_ptr<ssl_options> ssl_opts;

        /// �������÷���(Test)
        static options test()
        {
            static options const opts = {
                sboe::sbo_wait,
                rboe::rbo_wait,
                mlpe::mlp_derived,
                0,
                default_sbsize,
                default_rbsize,
                default_max_packet_size,
                30 * 1000
                };
            return opts;
        }

        /// �߲�������������������Ƽ����÷���
        static options multi_session_server()
        {
            static options const opts = {
                sboe::sbo_interrupt,
                rboe::rbo_interrupt,
                mlpe::mlp_derived,
                0,
                default_sbsize,
                default_rbsize,
                default_max_packet_size,
                30 * 1000
                };
            return opts;
        }

        ///// �ȶ������ȵ��Ƽ����÷���(�ݲ�֧��)
        //static options stability()
        //{
        //    static options const opts = {
        //        sboe::sbo_extend,
        //        rboe::rbo_extend,
        //        mlpe::mlp_derived,
        //        0,
        //        default_sbsize,
        //        default_rbsize,
        //        default_max_packet_size,
        //        30 * 1000
        //        };
        //    return opts;
        //}
    }; //struct options

} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_OPTIONS_HPP__
