#include "tcp_connector.h"
#include <Bex/utility/lexical_cast.hpp>
#include <boost/timer.hpp>

namespace Bex { namespace cobwebs
{
    void tcp_connector::async_connect( int port, std::string const& hostname
        , FactoryFunc fact_func, ConnectHandler const& handler, options const& opts /*= options()*/ )
    {
        async_connect(port, hostname, fact_func, handler, options_ptr(new options(opts)));
    }

    void tcp_connector::async_connect( int port, std::string const& hostname
        , FactoryFunc fact_func, ConnectHandler const& handler, options_ptr opts )
    {
        tcp::resolver::iterator resolver_it;
        tcp::endpoint endpoint;
        boost::system::error_code ec;

        // hostname���Ե���IP��ַʹ��
        address addr = address::from_string(hostname, ec);
        if (!ec)
            endpoint = tcp::endpoint(addr, port);
        else
        {
            // hostname����IP, ������������
            ec.clear();
            tcp::resolver::query query(hostname, lexical_cast_noexcept_d<std::string>(port));
            tcp::resolver rsl(m_ios);
            resolver_it = rsl.resolve(query, ec);
            if (ec || tcp::resolver::iterator() == resolver_it)
            {
                // ����ʧ��
                handler(tcp_session_ptr(), ec);
                return ;
            }

            // �����ɹ�, �ڵ�ַ�б��������������
            endpoint = *resolver_it++;
        }

        socket_ptr pSock(new socket(m_ios));
        pSock->async_connect(endpoint
            , boost::bind(&tcp_connector::async_connect_handler, this, placeholders::error
                , pSock, fact_func, handler, opts, resolver_it ));
    }
    
    /// ���ӻص�
    void tcp_connector::async_connect_handler( boost::system::error_code const& ec
        , socket_ptr pSock, FactoryFunc fact_func, ConnectHandler const& handler
        , options_ptr opts, tcp::resolver::iterator resolver_it )
    {
        tcp_session_ptr ptr;
        boost::system::error_code remote_ec, local_ec;
        if (!ec && pSock->remote_endpoint(remote_ec) != pSock->local_endpoint(local_ec)
            && !remote_ec && !local_ec)
            ptr = fact_func(pSock, session_initialized(session_id(), group_id(), opts));
        else if (tcp::resolver::iterator() != resolver_it)
        {
            boost::system::error_code close_ec;
            pSock->close(close_ec);
            if (close_ec)
                pSock.reset(new socket(m_ios));

            tcp::endpoint endpoint = *resolver_it++;
            pSock->async_connect(endpoint
                , boost::bind(&tcp_connector::async_connect_handler, this, placeholders::error
                , pSock, fact_func, handler, opts, resolver_it ));
            return ;
        }
        
        handler(ptr, ec);
    }

    /// ͬ������
    tcp_session_ptr tcp_connector::connect( FactoryFunc fact_func, int port, 
        std::string const& hostname, boost::system::error_code & ec, options const& opts /*= options()*/ )
    {
        return connect(fact_func, port, hostname, ec, options_ptr(new options(opts)));
    }

    tcp_session_ptr tcp_connector::connect( FactoryFunc fact_func, int port, 
        std::string const& hostname, boost::system::error_code & ec, options_ptr opts)
    {
        address addr = address::from_string(hostname, ec);
        if (ec)
        {
            // hostname����IP��ַ, ���Ե�����������
            ec.clear();
            tcp::resolver::query query(hostname, lexical_cast_noexcept_d<std::string>(port));
            tcp::resolver rsl(m_ios);
            tcp::resolver::iterator resolver_it = rsl.resolve(query, ec);
            if (ec)
                return tcp_session_ptr();

            ec = error::host_not_found;
            socket_ptr pSock(new socket(m_ios));
            while (ec && tcp::resolver::iterator() != resolver_it)
            {
                tcp::endpoint endpoint = *resolver_it++;
                std::cout << endpoint << std::endl;
                boost::system::error_code close_ec;
                pSock->close(close_ec);
                if (close_ec)
                    pSock.reset(new socket(m_ios));
                pSock->connect(endpoint, ec);
            }

            boost::system::error_code remote_ec, local_ec;
            if (!ec && pSock->remote_endpoint(remote_ec) != pSock->local_endpoint(local_ec)
                && !remote_ec && !local_ec)
                return fact_func(pSock, session_initialized(session_id(), group_id(), opts));
            else
                return tcp_session_ptr();
        }

        // hostname��IP��ַ
        tcp::endpoint endpoint(addr, port);
        socket_ptr pSock(new socket(m_ios));
        pSock->connect(endpoint, ec);
        
        boost::system::error_code remote_ec, local_ec;
        if (!ec && pSock->remote_endpoint(remote_ec) != pSock->local_endpoint(local_ec)
            && !remote_ec && !local_ec)
            return fact_func(pSock, session_initialized(session_id(), group_id(), opts));
        else
            return tcp_session_ptr();
    }

} //namespace cobwebs
} //namespace Bex