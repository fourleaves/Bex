#ifndef __BEX_IO_SESSION_HPP__
#define __BEX_IO_SESSION_HPP__

#include "bexio_fwd.hpp"

namespace Bex { namespace bexio
{
    using namespace boost::asio;
    using boost::system::error_code;

    template <typename Protocol, typename SessionMgr>
    class session
        : public SessionMgr::hook
        , public Protocol
    {
    public:
        struct id {};

        /// ����Protocol::F���id����, �Ƶ���F
        typedef boost::function<void(error_code)> OnConnectF;
        typedef boost::function<void(error_code)> OnDisconnectF;
        typedef boost::function<void(char const*, std::size_t)> OnReceiveF;

    public:
        explicit session(socket_ptr socket)
            : socket_(socket)
        {
        }

        // actor����session����ɻص�
        void run();

        // ��������
        void send(char const* data, std::size_t size)
        {
            socket_->sputn(data, size);
        }

        template <typename ConstBufferSequence>
        void send(ConstBufferSequence const& buffers)
        {
            socket_->sputn(buffers);
        }

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
        void on_receive();

    private:
        socket_ptr socket_;
    };

} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_SESSION_HPP__