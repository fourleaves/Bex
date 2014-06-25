#ifndef __BEX_IO_SESSION_HPP__
#define __BEX_IO_SESSION_HPP__

#include "intrusive_list.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>

namespace Bex { namespace bexio
{
    using namespace boost::asio;
    using boost::system::error_code;

    template <typename Protocol, typename SocketPolicy, typename SessionMgr>
    class session
        : public SessionMgr::hook
        , public SocketPolicy::storage
    {
    public:
        typedef typename SocketPolicy::socket socket_t;
        typedef boost::shared_ptr<socket_t> socket_ptr;

    public:
        explicit session(socket_ptr socket);

        // actor����session����ɻص�
        void run();

        // ��������
        template <typename ConstBufferSequence>
        void send(ConstBufferSequence const& const_buffer_sequence);

        // ���ŵعر�����
        void shutdown();

        // ǿ�Ƶعر�����
        void terminate();

    private:
        // �����첽��������
        void post_send();

        // �첽���ͻص�
        void on_async_send(error_code ec, std::size_t bytes);

        // �����첽��������
        void post_receive();

        // �첽���ջص�
        void on_async_receive(error_code ec, std::size_t bytes, char const* data);

    private:
        // �������ݻص�
        void on_receive() BEX_FINAL;

    protected:
        // ���ӻص�
        virtual void on_connect(error_code ec);

        // �������ݻص�
        virtual void on_receive(const_buffer data);
        
        // �Ͽ����ӻص�
        virtual void on_disconnect(error_code ec);

    private:
        socket_ptr socket_;
    };

} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_SESSION_HPP__