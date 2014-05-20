#include <Bex/network/cobwebs.h>

class my_session
    : public Bex::tcp_session
{
    __int64 m_receive_bytes;
    __int64 m_send_bytes;
    __int64 m_packet_count;
    Bex::signal_scoped m_sc;
    Bex::tcp_packet_parser m_parser;

public:
    my_session(socket_ptr pSock, Bex::session_initialized const& si);

    ~my_session();

public:
    /// �յ����
    void on_packet(_stPacketHead * pHead, Bex::static_streambuf & buf);

    /// �����������
    void on_error_packet();

    static bool static_send(boost::weak_ptr<Bex::tcp_session> wp, char const* buf, std::size_t bytes);

protected:
    /// ���ӳɹ��ص�
    virtual void on_connected();

    /// ��������
    virtual bool send(char const* buf, std::size_t bytes);

    /// �������ݳɹ��ص�
    virtual void on_recv(char * buf, std::size_t bytes);

    /// �Ͽ����ӻص�
    virtual void on_disconnect(boost::system::error_code const& ec);

protected:
    /// ���ջص�
    virtual void recv_handler(boost::system::error_code const& ec, std::size_t bytes);

    /// ���ͻص�
    virtual void send_handler(boost::system::error_code const& ec, std::size_t bytes);
};