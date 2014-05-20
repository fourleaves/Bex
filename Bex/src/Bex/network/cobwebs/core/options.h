#ifndef __BEX_NETWORK_COBWEBS_OPTIONS_H__
#define __BEX_NETWORK_COBWEBS_OPTIONS_H__

//////////////////////////////////////////////////////////////////////////
/// ѡ������

#include <Bex/utility/singleton.hpp>

namespace Bex { namespace cobwebs
{
    struct options
    {
        int sendbufbytes;   ///< ���ͻ�������С
        int recvbufbytes;   ///< ���ջ�������С
        int sendbufcount;   ///< ���ͻ���������
        int recvbufcount;   ///< ���ջ���������
        bool sendbufoverflow_disconnect;  ///< ���ͻ���������Ƿ�Ͽ�����
        bool recvbufoverflow_disconnect;  ///< ���ջ���������Ƿ�Ͽ�����
        int max_packet_size;    ///< �������ݰ���󳤶�
        int listen_count;       ///< ����������

        options()
        {
            sendbufbytes = 64 * 1024;
            recvbufbytes = 64 * 1024;
            sendbufcount = 1;
            recvbufcount = 1;
            sendbufoverflow_disconnect = true;
            recvbufoverflow_disconnect = true;
            max_packet_size = 8 * 1024;
            listen_count = 1;
        }

        /// Todo: read from ini/xml/property_tree
    };

    typedef boost::shared_ptr<options> options_ptr;

} //namespace cobwebs
} //namespace Bex

#endif //__BEX_NETWORK_COBWEBS_OPTIONS_H__