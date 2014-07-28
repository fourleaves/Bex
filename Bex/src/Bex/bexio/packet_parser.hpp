#ifndef __BEX_IO_PACKET_PARSER_HPP__
#define __BEX_IO_PACKET_PARSER_HPP__

//////////////////////////////////////////////////////////////////////////
/// pod���ͷʽ�����ݽ�����

/*
* @Concept:
*   Parser
*/

#include "bexio_fwd.hpp"

namespace Bex { namespace bexio 
{
    // ����std::size_t size(); �ӿڵķ��ͷ���õ�Sizer
    struct sizer_packet_head
    {
        template <typename PacketHead>
        inline std::size_t operator()(PacketHead & ph) const
        {
            return ph.size();
        }
    };

    // ������ǰ4���ֽڵķ��ͷ���õ�Sizer
    struct first4_packet_head
    {
        template <typename PacketHead>
        inline std::size_t operator()(PacketHead & ph) const
        {
            return *(boost::uint32_t*)boost::addressof(ph);
        }
    };

    /// ���������
    template <typename PacketHead = boost::uint32_t,    ///< ���ͷ���Ͷ���
        typename Sizer = first4_packet_head,            ///< ���ݷ��ͷ��������Ľӿ�(�����������ͷ�ĳ���)
        typename Allocator = ::Bex::bexio::allocator<int> >
    class packet_parser
    {
    public:
        typedef PacketHead packet_head_type;
        typedef typename Allocator::template rebind<char>::other allocator;
        typedef boost::function<void(error_code const&, PacketHead*, char const*, std::size_t)> Callback;

        // ��ͷ����
        static const std::size_t head_size = sizeof(PacketHead);

        ~packet_parser()
        {
            if (init_.reset())
                allocator().deallocate(buf_);
        }

        // ��ʼ��
        void initialize(std::size_t max_packet_size, Callback callback)
        {
            if (!init_.set())
                return ;

            size_ = (std::max)(max_packet_size, head_size);
            buf_ = allocator().allocate(size_);
            pos_ = 0;
            callback_ = callback;
        }

        // ѹ�����������
        void parse(char const* data, std::size_t size)
        {
            char const* buf = data;
            std::size_t len = size;

            while (len)
            {
                if (0 == pos_)
                {
                    // ��������������, ֱ�����ϲ㻺�����г������.
                    if (len < head_size)
                    {
                        // ����һ�����ͷ, д�뻺��������
                        write_buffer(buf, len);
                        return ;
                    }
                    else
                    {
                        // �������ķ��ͷ, ��һ������Ƿ��������ķ��.
                        std::size_t packet_len = Sizer()(packethead(buf)) + head_size;
                        if (packet_len > size_)
                        {
                            // ���������
                            invoke_callback(generate_error(bee::parse_error), 0, 0, 0);
                            return ;
                        }

                        if (len < packet_len)
                        {
                            // ����һ�����, ���뻺������, �ȴ���������.
                            write_buffer(buf, len);
                            return ;
                        }
                        else
                        {
                            // �������ķ��, ֱ�Ӵ���.
                            invoke_callback(error_code(), &packethead(buf), buf + head_size, packet_len - head_size);
                            buf += packet_len;
                            len -= packet_len;
                            continue;
                        }
                    }
                }
                else if (pos_ < head_size)
                {
                    // ��������������, ������һ�����ͷ, ����ƴ��һ�����ͷ
                    std::size_t delta = head_size - pos_;
                    std::size_t cpy_size = (std::min)(delta, len);
                    write_buffer(buf, cpy_size);
                    buf += cpy_size;
                    len -= cpy_size;
                    continue;
                }
                else
                {
                    // ���������������ķ��ͷ
                    std::size_t packet_len = Sizer()(packethead(buf_)) + head_size;
                    if (packet_len > size_)
                    {
                        // ���������
                        invoke_callback(generate_error(bee::parse_error), 0, 0, 0);
                        return ;
                    }

                    std::size_t delta = packet_len - pos_;
                    if (delta > len)
                    {
                        // �޷�ƴ��һ�������ķ��
                        write_buffer(buf, len);
                        return ;
                    }
                    else
                    {
                        // ����ƴ��һ�������ķ��
                        write_buffer(buf, delta);
                        invoke_callback(error_code(), &packethead(buf_), buf_ + head_size, pos_ - head_size);
                        pos_ = 0;
                        buf += delta;
                        len -= delta;
                        continue;
                    }
                }
            }
        }

    private:
        // ���ûص�
        inline void invoke_callback(error_code const& ec, PacketHead * ph, char const* data, std::size_t size)
        {
            if (callback_)
                callback_(ec, ph, data, size);
        }

        // д��������������
        bool write_buffer(char const* data, std::size_t size)
        {
            if (size + pos_ > size_) 
                return false;

            memcpy(buf_ + pos_, data, size);
            pos_ += size;
            return true;
        }

        // ������ת��Ϊ��ͷ
        inline PacketHead & packethead(char const* data)
        {
            return (*(PacketHead*)data);
        }

    private:
        // �Ƿ��ʼ��
        sentry<inter_lock> init_;

        // ������
        char * buf_;
        std::size_t size_;
        std::size_t pos_;

        // ��������ص�
        Callback callback_;
    };


} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_PACKET_PARSER_HPP__