#ifndef __BEX_IO_ERROR_HPP__
#define __BEX_IO_ERROR_HPP__

//////////////////////////////////////////////////////////////////////////
/// �Զ���errorcode
#include <Bex/config.hpp>
#include <boost/system/error_code.hpp>

namespace Bex { namespace bexio
{
    enum BEX_ENUM_CLASS bexio_error_em
    {
        initiative_terminate,   ///< ǿ�������Ͽ�����
        initiative_shutdown,    ///< ���ŵ������Ͽ�����
        passive_shutdown,       ///< ���ŵر����Ͽ�����
        sendbuffer_overflow,    ///< ���ͻ��������
        receivebuffer_overflow, ///< ���ջ��������
        connect_overtime,       ///< ���ӳ�ʱ
        parse_error,            ///< ���ݽ�������
        reconnect_error,        ///< ����ʧ��
        miss_ssl_options,       ///< SSLѡ��δ����
    };
    typedef bexio_error_em bee;

    class bexio_error
        : public boost::system::error_category
    {
#if defined(BOOST_NO_CXX11_DEFAULTED_FUNCTIONS)
        bexio_error() {}
#else  // defined(BOOST_NO_CXX11_DEFAULTED_FUNCTIONS)
        bexio_error() = default;
#endif // defined(BOOST_NO_CXX11_DEFAULTED_FUNCTIONS)

    public:
        virtual char const* name() const BOOST_SYSTEM_NOEXCEPT
        {
            return "bexio_error";
        }

        virtual std::string message(int ev) const BOOST_SYSTEM_NOEXCEPT
        {
            switch(ev)
            {
            case (int)bee::initiative_terminate:
                return "initiative terminate";

            case (int)bee::initiative_shutdown:
                return "initiative shutdown";

            case (int)bee::passive_shutdown:
                return "passive shutdown";

            case (int)bee::sendbuffer_overflow:
                return "sendbuffer overflow";

            case (int)bee::receivebuffer_overflow:
                return "receivebuffer overflow";

            case (int)bee::connect_overtime:
                return "connect overtime";

            case (int)bee::parse_error:
                return "data parse error";

            case (int)bee::reconnect_error:
                return "reconnect error";

            case (int)bee::miss_ssl_options:
                return "miss ssl options";

            default:
                return "undefined bexio error";
            }
        }

        friend bexio_error const& get_bexio_category();
    };

    inline bexio_error const& get_bexio_category()
    {
        static bexio_error err;
        return err;
    }

    /// ����bexio_error���͵�error_code
    inline boost::system::error_code generate_error(bexio_error_em ev)
    {
        return boost::system::error_code((int)ev, get_bexio_category());
    }

} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_ERROR_HPP__