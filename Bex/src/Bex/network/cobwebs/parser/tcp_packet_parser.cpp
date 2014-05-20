#include "tcp_packet_parser.h"
#include <Bex/network/cobwebs/core/core.h>


namespace Bex { namespace cobwebs
{
    class tcp_packet_parser::Impl
    {
    public:
        typedef tcp_packet_parser::OnErrorFunc OnErrorFunc;

    private:
        char * m_buffer;
        std::size_t m_pos;
        const std::size_t m_size;

        ph_wrap_ptr m_pWrapper;
        OnErrorFunc m_onError;

    public:
        Impl( std::size_t max_packet_size , ph_wrap_ptr pWrapper, OnErrorFunc const& onError );
        ~Impl();

        /// ѹ������
        void sputn(char * buf, std::size_t len);

        /// ʣ������ݳ���
        std::size_t overage();

        /// ��ȡʣ������
        std::size_t extract_overage(char * buf, std::size_t len);
    };

    tcp_packet_parser::Impl::Impl( std::size_t max_packet_size
        , ph_wrap_ptr pWrapper, OnErrorFunc const& onError )
        : m_size(max_packet_size), m_pWrapper(pWrapper), m_onError(onError), m_pos(0)
    {
        m_buffer = (char*)core::getInstance().get_pool().ordered_malloc(m_size);
    }

    tcp_packet_parser::Impl::~Impl()
    {
        core::getInstance().get_pool().ordered_free(m_buffer, m_size);
        m_buffer = 0;
    }

    /// ѹ������
    void tcp_packet_parser::Impl::sputn( char * buf, std::size_t len )
    {
        BOOST_ASSERT(m_buffer);
        BOOST_ASSERT(m_pWrapper);

        const std::size_t head_size = m_pWrapper->size_of();
        while (len)
        {
            if (0 == m_pos)
            {
                // ��������������, ֱ�����ϲ㻺�����г������.
                if (len < head_size)
                {
                    // ����һ�����ͷ, д�뻺��������
                    memcpy(m_buffer, buf, len);
                    m_pos += len;
                    return ;
                }
                else
                {
                    // �������ķ��ͷ, ��һ������Ƿ��������ķ��.
                    std::size_t packet_len = m_pWrapper->buf_size(buf) + head_size;
                    if (packet_len > m_size)
                    {
                        // ���������
                        if (m_onError) m_onError();
                        return ;
                    }

                    if (len < packet_len)
                    {
                        // ����һ�����
                        memcpy(m_buffer, buf, len);
                        m_pos += len;
                        return ;
                    }
                    else
                    {
                        // �������ķ��, ֱ�Ӵ���.
                        m_pWrapper->onPacket(buf, packet_len);
                        buf += packet_len;
                        len -= packet_len;
                        continue;
                    }
                }
            }
            else if (m_pos < head_size)
            {
                // ��������������, ������һ�����ͷ, ����ƴ��һ�����ͷ
                std::size_t delta = head_size - m_pos;
                std::size_t cpy_size = (std::min)(delta, len);
                memcpy(m_buffer + m_pos, buf, cpy_size);
                m_pos += cpy_size;
                buf += cpy_size;
                len -= cpy_size;
                continue;
            }
            else
            {
                // ���������������ķ��ͷ
                std::size_t packet_len = m_pWrapper->buf_size(m_buffer) + head_size;
                if (packet_len > m_size)
                {
                    // ���������
                    if (m_onError) m_onError();
                    m_pos = 0;
                    return ;
                }

                std::size_t delta = packet_len - m_pos;
                if (delta > len)
                {
                    // �޷�ƴ��һ�������ķ��
                    memcpy(m_buffer + m_pos, buf, len);
                    m_pos += len;
                    return ;
                }
                else
                {
                    // ����ƴ��һ�������ķ��
                    memcpy(m_buffer + m_pos, buf, delta);
                    m_pWrapper->onPacket(m_buffer, m_pos + delta);
                    m_pos = 0;
                    buf += delta;
                    len -= delta;
                    continue;
                }
            }
        }
    }

    /// ʣ������ݳ���
    std::size_t tcp_packet_parser::Impl::overage()
    {
        return m_pos;
    }

    /// ��ȡʣ������
    std::size_t tcp_packet_parser::Impl::extract_overage( char * buf, std::size_t len )
    {
        len = (std::min)(len, m_pos);
        memcpy(buf, m_buffer, len);
        return len;
    }

    void tcp_packet_parser::initialize( std::size_t max_packet_size , ph_wrap_ptr pWrapper, OnErrorFunc const& onError )
    {
        BOOST_ASSERT(pWrapper);
        m_pImpl = new Impl(max_packet_size, pWrapper, onError);
    }

    tcp_packet_parser::tcp_packet_parser( std::size_t max_packet_size , ph_wrap_ptr pWrapper, OnErrorFunc const& onError )
    {
        initialize(max_packet_size, pWrapper, onError);
    }

    tcp_packet_parser::~tcp_packet_parser()
    {
        if (m_pImpl)
        {
            delete m_pImpl;
            m_pImpl = 0;
        }
    }

    /// ѹ������
    void tcp_packet_parser::sputn( char * buf, std::size_t len )
    {
        m_pImpl->sputn(buf, len);
    }

    /// ʣ������ݳ���
    std::size_t tcp_packet_parser::overage()
    {
        return m_pImpl->overage();
    }

    /// ��ȡʣ������
    std::size_t tcp_packet_parser::extract_overage( char * buf, std::size_t len )
    {
        return m_pImpl->extract_overage(buf, len);
    }

} //namespace cobwebs
} //namespace Bex