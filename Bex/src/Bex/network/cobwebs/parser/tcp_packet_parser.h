#ifndef __BEX_NETWORK_COBWEBS_PARSER_TCP_PACKET_PARSER_H__
#define __BEX_NETWORK_COBWEBS_PARSER_TCP_PACKET_PARSER_H__

//////////////////////////////////////////////////////////////////////////
/// Tcp���������

/* PacketHead������Լ��:
*    1.������pod����.
*       (���PacketHead����Ҫ���캯��, ���Զ����BEX_COBWEBS_CHECK_PACKET_HEAD_POD�������������Զ����.)
*    2.�г�Ա����std::size_t size() const;, ���ط���г���ͷ������ݳ���.
*/

#include "packet_head_wrapper.hpp"

namespace Bex { namespace cobwebs
{
    class tcp_packet_parser
    {
        class Impl;
        Impl * m_pImpl;

    public:
        typedef boost::function<void()> OnErrorFunc;

    private:
        void initialize(std::size_t max_packet_size
            , ph_wrap_ptr pWrapper, OnErrorFunc const& onError);

    public:
        tcp_packet_parser(std::size_t max_packet_size
            , ph_wrap_ptr pWrapper, OnErrorFunc const& onError);

        ~tcp_packet_parser();

        /// ѹ������
        void sputn(char * buf, std::size_t len);

        /// ʣ������ݳ���
        std::size_t overage();

        /// ��ȡʣ������
        std::size_t extract_overage(char * buf, std::size_t len);
    };

} //namespace cobwebs
} //namespace Bex

#endif //__BEX_NETWORK_COBWEBS_PARSER_TCP_PACKET_PARSER_H__