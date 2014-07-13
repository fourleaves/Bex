#pragma warning(disable:4996)
#define _WIN32_WINNT 0x501
#include <iostream>
#include <string>
#define Dump(x) do { std::cout << x << std::endl; } while(0)

#include <Bex/bexio/bexio.hpp>
#include <Bex/bexio/ssl_protocol.hpp>
#include <Bex/auto_link.h>
#include <Bex/utility/format.hpp>
using namespace Bex::bexio;


enum {
    t_simple = 0,       // �򵥲���
    t_pingpong = 1,     // pingpong����������
    t_multiconn = 2,    // ������������
    t_packet = 3,       // �������
    t_tcp_shutdown = 4, // �������ŵعر�����(���ͺ�����shutdown, Ҫ��֤�Զ˿��Խ�������, ��������.)
    t_ssl_shutdown = 5, // �������ŵعر�����(���ͺ�����shutdown, Ҫ��֤�Զ˿��Խ�������, ��������.)
};

std::string remote_ip = "127.0.0.1";
volatile long s_count = 0;
volatile long s_obj_count = 0;
io_service ios;
options opt = options::test();

template <typename Protocol>
class simple_session
    : public basic_session<Protocol>
{
public:
    virtual void on_connect() BEX_OVERRIDE
    {
        Dump(boost::this_thread::get_id() << " on connect " << remote_endpoint());
        char buf[] = "Hello, I'm bexio!";
        send(buf, sizeof(buf));
    }

    virtual void on_disconnect(error_code const& ec) BEX_OVERRIDE
    {
        Dump(boost::this_thread::get_id() << " on disconnect, error:" << ec.message());
    }

    virtual void on_receive(char const* data, std::size_t size) BEX_OVERRIDE
    {
        Dump(boost::this_thread::get_id() << " size = " << size << " data:" << std::string(data, size));
    }
};

template <typename Protocol>
class pingpong_session
    : public simple_session<Protocol>
{
public:
    static pingpong_session * p;

    virtual void on_connect() BEX_OVERRIDE
    {
        Dump(boost::this_thread::get_id() << " on connect " << remote_endpoint());
        char buf[8000] = {1};
        send(buf, sizeof(buf));
        p = this;
    }

    virtual void on_receive(char const* data, std::size_t size) BEX_OVERRIDE
    {
        //Dump(boost::this_thread::get_id() << " size = " << size << " data:" << std::string(data, size));
        reply(data, size);
    }

    virtual void on_disconnect(error_code const& ec) BEX_OVERRIDE
    {
        Dump(boost::this_thread::get_id() << " on disconnect, error:" << ec.message());
    }

    bool reply(char const* data, std::size_t size)
    {
        bool first = send(data, size);
        //bool second = send(data, size);
        return first;// || second;
    }
};
template <typename Protocol>
pingpong_session<Protocol> * pingpong_session<Protocol>::p = 0;

template <typename Protocol>
class multi_session
    : public basic_session<Protocol>
{
public:
    multi_session()
    {
        ::BOOST_INTERLOCKED_INCREMENT(&s_obj_count);
    }

    ~multi_session()
    {
        ::BOOST_INTERLOCKED_DECREMENT(&s_obj_count);
    }

    virtual void on_connect() BEX_OVERRIDE
    {
        ::BOOST_INTERLOCKED_INCREMENT(&s_count);
    }

    virtual void on_disconnect(error_code const& ec) BEX_OVERRIDE
    {
        ::BOOST_INTERLOCKED_DECREMENT(&s_count);
    }
};

template <typename Protocol>
class packet_session
    : public basic_session<Protocol>
{
public:
    virtual void on_connect() BEX_OVERRIDE
    {
        Dump(boost::this_thread::get_id() << " on connect " << remote_endpoint());
        ::BOOST_INTERLOCKED_INCREMENT(&s_count);

        char p1[4] = {};
        char p2[8] = {};
        char p3[138] = {};
        char p4[4099] = {};
        *(boost::uint32_t*)p2 = 4;
        *(boost::uint32_t*)p3 = 134;
        *(boost::uint32_t*)p4 = 4095;

        send(p1, sizeof(p1));
        send(p2, sizeof(p2));
        send(p3, sizeof(p3));
        send(p4, sizeof(p4));
    }

    virtual void on_disconnect(error_code const& ec) BEX_OVERRIDE
    {
        Dump(boost::this_thread::get_id() << " on disconnect");
        ::BOOST_INTERLOCKED_DECREMENT(&s_count);
    }

    virtual void on_receive(error_code const& ec, boost::uint32_t * ph
        , char const* data, std::size_t size) BEX_OVERRIDE
    {
        //Dump(boost::this_thread::get_id() << " size = " << size << " data:" << std::string(data, size));
        if (ec)
        {
            Dump("error: " << ec.message());
            terminate();
            return ;
        }

        send((char const*)ph, size + sizeof(boost::uint32_t));
    }
};

template <typename Protocol>
class shutdown_session
    : public basic_session<Protocol>
{
public:
    shutdown_session()
    {
        ::BOOST_INTERLOCKED_INCREMENT(&s_obj_count);
    }

    ~shutdown_session()
    {
        ::BOOST_INTERLOCKED_DECREMENT(&s_obj_count);
    }

    virtual void on_connect() BEX_OVERRIDE
    {
        ::BOOST_INTERLOCKED_INCREMENT(&s_count);
        Dump(boost::this_thread::get_id() << " on connect " << remote_endpoint());
        char buf[] = "I will shutdown session!";
        bool ok = send(buf, sizeof(buf));
        shutdown();
        if (!ok)
            Dump("Send failed!");
    }

    virtual void on_receive(char const* data, std::size_t size) BEX_OVERRIDE
    {
        Dump("recv " << size << " bytes: " << std::string(data, size));
    }

    virtual void on_disconnect(error_code const& ec) BEX_OVERRIDE
    {
        ::BOOST_INTERLOCKED_DECREMENT(&s_count);
        Dump(boost::this_thread::get_id() << " on disconnect! error:" << ec.message());
    }
};

template <typename EndPoint>
void on_handshake_error(error_code const& ec, EndPoint const& addr)
{
    Dump("handshake error: {" << ec.value() << ", " << ec.message() << "} addr:{" << addr << "}");
}

template <class Session>
void start_server()
{
    typedef basic_server<Session> server;
    opt.ssl_opts = ssl_options::server();

    server s(ios, opt);
    s.set_handshake_error_callbcak(boost::BOOST_BIND(&on_handshake_error<server::endpoint>, _1, _2));
    bool ok = s.startup(server::endpoint(ip::address::from_string("0.0.0.0"), 28087), 10);
    if (!ok)
        Dump("server startup error: " << s.get_error_code().message());

    server::mstrand_service_type & core = use_service<server::mstrand_service_type >(ios);
    io_service::work worker(core.actor());
    core.run();
}

template <class Session>
void start_client()
{
    typedef basic_client<Session> client;
    opt.ssl_opts = ssl_options::client();

    client c(ios, opt);
    c.set_handshake_error_callbcak(boost::BOOST_BIND(&on_handshake_error<client::endpoint>, _1, _2));
    bool ok = c.connect(client::endpoint(ip::address::from_string(remote_ip), 28087));
    if (!ok)
        Dump("connect error: " << c.get_error_code().message());

    client::mstrand_service_type & core = use_service<client::mstrand_service_type >(ios);
    io_service::work worker(core.actor());
    core.run();
}

void on_connect_callback(error_code const& ec)
{
    Dump("connected!");
}

template <class Session>
void start_multi_client(int count = 100)
{
    typedef basic_client<Session> client;
    opt.ssl_opts = ssl_options::client();

    std::list<boost::shared_ptr<client> > clients;

    for (int i = 0; i < count; ++i)
    {
        shared_ptr<client> c(new client(ios, opt));
        clients.push_back(c);
        c->set_async_connect_callback(&on_connect_callback);
        c->set_handshake_error_callbcak(boost::BOOST_BIND(&on_handshake_error<client::endpoint>, _1, _2));
        bool ok = c->async_connect(client::endpoint(ip::address::from_string(remote_ip), 28087));
        if (!ok)
            Dump("connect error: " << c->get_error_code().message());
    }

    client::mstrand_service_type & core = use_service<client::mstrand_service_type >(ios);
    io_service::work worker(core.actor());
    core.run();
}

void handle_ctrl_c(error_code, int, signal_set * ss)
{
    ss->async_wait(boost::BOOST_BIND(&handle_ctrl_c, _1, _2, ss));
    Dump("ctrl-c");
    char buf[10];
    if (pingpong_session<tcp_protocol<> >::p)
    {
        bool ok = pingpong_session<tcp_protocol<> >::p->send(buf, sizeof(buf));
        Dump("Send " << ok);
    }
}

int main()
{
    signal_set signal_proc(ios, SIGINT);
    signal_proc.async_wait(boost::BOOST_BIND(&handle_ctrl_c, _1, _2, &signal_proc));

    int input = 0;
    do 
    {
        std::cout << "�����������"
            "\n\t(0:tcp, 2:ssl_tcp)"
            "\n\t(0:simple, 1:pingpong, 2:multiconn, 3:packet, 4:tcp_shutdown)"
            "\n\t(0:server, 1:client):" << std::endl;
        std::cin >> input;

        int proto = input / 100;
        int type = (input % 100) / 10;
        int point = input % 10;

#if defined(_MSC_VER) && (_MSC_VER >= 1800)
        boost::thread th([point] {
            for (;;)
            {
                long c = s_count;
                long oc = s_obj_count;
                ::SetConsoleTitleA(Bex::format("%s (%d)(%d)", ((point == 0) ? "server" : "client"), oc, c).c_str());
                boost::this_thread::sleep(boost::posix_time::millisec(100));
            }
        });
#endif

        typedef tcp_protocol<> tcp_proto;
        typedef tcp_packet_protocol<> tcp_packet_proto;
        typedef ssl_protocol<tcp_protocol<> > ssl_proto;
        typedef ssl_protocol<tcp_packet_protocol<> > ssl_packet_proto;

        switch (type)
        {
        case t_simple:
            if (point == 0)
                if (proto == 0)
                    start_server<simple_session<tcp_proto> >();
                else
                    start_server<simple_session<ssl_proto> >();
            else
                if (proto == 0)
                    start_client<simple_session<tcp_proto> >();
                else
                    start_client<simple_session<ssl_proto> >();
            break;

        case t_pingpong:
            if (point == 0)
                if (proto == 0)
                    start_server<pingpong_session<tcp_proto> >();
                else
                    start_server<pingpong_session<ssl_proto> >();
            else
                if (proto == 0)
                    start_client<pingpong_session<tcp_proto> >();
                else
                    start_client<pingpong_session<ssl_proto> >();
            break;

        case t_multiconn:
            {
                opt.send_buffer_size = opt.receive_buffer_size = 64;

                if (point == 0)
                    if (proto == 0)
                        start_server<multi_session<tcp_proto> >();
                    else
                        start_server<multi_session<ssl_proto> >();
                else
                {
                    int count = 1;
                    std::cout << "������ͻ�������:" << std::endl;
                    std::cin >> count;
                    if (proto == 0)
                        start_multi_client<multi_session<tcp_proto> >(count);
                    else
                        start_multi_client<multi_session<ssl_proto> >(count);
                }
            }
            break;

        case t_packet:
            {
                if (point == 0)
                    if (proto == 0)
                        start_server<packet_session<tcp_packet_proto> >();
                    else
                        start_server<packet_session<ssl_packet_proto> >();
                else
                {
                    int count = 1;
                    std::cout << "������ͻ�������:" << std::endl;
                    std::cin >> count;
                    if (proto == 0)
                        start_multi_client<packet_session<tcp_packet_proto> >(count);
                    else
                        start_multi_client<packet_session<ssl_packet_proto> >(count);
                }
            }
            break;

        case t_tcp_shutdown:
            if (point == 0)
                if (proto == 0)
                    start_server<shutdown_session<tcp_proto> >();
                else
                    start_server<shutdown_session<ssl_proto> >();
            else
                if (proto == 0)
                    start_client<shutdown_session<tcp_proto> >();
                else
                    start_client<shutdown_session<ssl_proto> >();
            break;

        default:
            break;
        }
    } while (true);

    std::cin.get();
    return 0;
}
