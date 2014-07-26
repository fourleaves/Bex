#ifndef __BEX_IO_BEXIO_CORE_HPP__
#define __BEX_IO_BEXIO_CORE_HPP__

//////////////////////////////////////////////////////////////////////////
/// ���Ĳ�(��io_service�ķ�װ)
#include "bexio_fwd.hpp"
#include "multithread_strand_service.hpp"

namespace Bex { namespace bexio
{
    struct core_default_tag {};

    template <class Allocator = ::Bex::bexio::allocator<int>
        , class Tag = ::Bex::bexio::core_default_tag
        >
    class core
        : public singleton<core<Allocator, Tag> >
    {
        friend class singleton<core<Allocator, Tag> >;

    public:
        typedef multithread_strand_service<Allocator> ServiceType;

        // ��ѭ���ƽ�
        std::pair<std::size_t, error_code> run()
        {
            return mts_srv_.run();
        }

        // ��ú��io_service. 
        // * �����ʹ�õ�, 
        // * ��ʼ��server/clientʱ, �����ʹ������ӿڷ��ص�io_service, ��Allocator����һ��!
        io_service & backend()
        {
            return ios_;
        }

        // ���ǰ��io_service. 
        // * Ӧ�ò�ʹ�õ�
        io_service & backfront()
        {
            return mts_srv_.actor();
        }

        // ���ط���
        ServiceType & get_service()
        {
            return mts_srv_;
        }

    private:
        core()
            : mts_srv_(use_service<ServiceType>(ios_))
        {
        }

        ~core() {}

    private:
        io_service ios_;
        ServiceType & mts_srv_;
    };

} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_BEXIO_CORE_HPP__