#ifndef __BEX_IO_TCP_PROTOCOL_HPP__
#define __BEX_IO_TCP_PROTOCOL_HPP__

//////////////////////////////////////////////////////////////////////////
/// tcp socket policy
#include "bexio_fwd.hpp"
#include "buffered_socket.hpp"
#include "nonblocking_circularbuffer.hpp"
#include "allocator.hpp"

namespace Bex { namespace bexio
{
    template <typename Buffer = nonblocking_circularbuffer,
        typename Allocator = ::Bex::bexio::allocator<int> >
    struct tcp_protocol
    {
        typedef Buffer buffer_type;
        typedef Allocator allocator;

        typedef ip::tcp::endpoint endpoint;
        typedef ip::tcp::acceptor acceptor;
        typedef ip::tcp::resolver resolver;
        typedef ip::tcp::socket native_socket_type;
        typedef buffered_socket<native_socket_type, buffer_type, allocator> socket;
        typedef boost::shared_ptr<socket> socket_ptr;

        // callback functions
        typedef boost::function<void(char const*, std::size_t)> OnReceiveF;

        static socket_ptr alloc_socket(io_service & ios, std::size_t rbsize, std::size_t wbsize)
        {
            return make_shared_ptr<socket, allocator>(ios, rbsize, wbsize);
        }

    protected:
        /// �������û��Զ���session��д�Ľ������ݺ���
        virtual void on_receive(char const* /*data*/, std::size_t /*size*/) {}


        //////////////////////////////////////////////////////////////////////////
        /// @{ ֻ��session���Ե�������ӿ�
    public:
        /// ��ʼ��
        template <typename F, typename Id>
        void initialize(shared_ptr<options> const& opts, F const& f, Id const& id)
        {
            opts_ = opts;
            if (f)
                global_receiver_ = BEX_IO_BIND(f, id, _1, _2);
        }

    protected:
        /// ���ڴ��ݽ��յ���������������
        inline void parse(const_buffer const& buffer)
        {
            if (opts_->mlpe_ == mlpe::mlp_derived || opts_->mlpe_ == mlpe::mlp_both)
                on_receive((char const*)buffer_cast_helper(buffer),
                    buffer_size_helper(buffer));
            
            if (opts_->mlpe_ == mlpe::mlp_callback || opts_->mlpe_ == mlpe::mlp_both)
                if (global_receiver_)
                    global_receiver_((char const*)buffer_cast_helper(buffer),
                        buffer_size_helper(buffer));
        }
        /// @}
        //////////////////////////////////////////////////////////////////////////

    private:
        shared_ptr<options> opts_;
        OnReceiveF global_receiver_;
    };
    
} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_TCP_PROTOCOL_HPP__