#ifndef __BEX_IO_NONBLOCKING_CIRCULARBUFFER_HPP__
#define __BEX_IO_NONBLOCKING_CIRCULARBUFFER_HPP__

//////////////////////////////////////////////////////////////////////////
// ���λ�����
/*
* һ����һ��дʱ, �����������̰߳�ȫ�Ļ��λ�����.
*/

#include <cstdio>
#include <algorithm>
#include "bexio_fwd.hpp"

namespace Bex { namespace bexio
{
    class nonblocking_circularbuffer
        : boost::noncopyable
    {
    public:
        typedef char value_type;
        typedef value_type* pointer;
        typedef value_type const* const_pointer;
        typedef std::size_t size_type;
        typedef std::size_t difference_type;
        typedef value_type& reference_type;

        nonblocking_circularbuffer(pointer buffer, size_type capacity)
        {
            reset(buffer, capacity);
        }

        // ��������ʼ��ַ
        const_pointer address() const
        {
            return buffer_;
        }

        // �������ܴ�С
        size_type capacity() const
        {
            return capacity_;
        }

        //////////////////////////////////////////////////////////////////////////
        // @{ only one thread methods
        // ���ö�дָ��
        void reset()
        {
            get_ = buffer_, put_ = buffer_ + 1;
        }

        // ���軺��������дָ��
        void reset(pointer buffer, size_type capacity)
        {
            buffer_ = buffer, capacity_ = capacity;
            reset();
        }

        // �Ƿ��
        bool empty() const
        {
            return (gbegin() == gend());
        }

        // �Ƿ���
        bool full() const
        {
            return (pbegin() == pend());
        }
        // @}
        //////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        // @{ put thread methods
        // дָ��
        pointer pptr() const
        {
            return put_;
        }

        // �����Ŀ�д����
        size_type pcount() const
        {
            pointer pb = pbegin(), pe = pend();
            return (pb > pe) ? distance(pb, end()) : distance(pb, pe);
        }

        // дָ������ƶ�offset��λ��
        void pbump(difference_type offset)
        {
            put_ = advance(put_, offset);
        }

        // �ܼƵĿ�д����
        size_type psize() const
        {
            return distance(pbegin(), pend());
        }

        // д������
        size_type sputn(const_pointer data, size_type size)
        {
            const_pointer pos = data;
            size_type put_size = size;
            while (put_size)
            {
                size_type once = (std::min)(pcount(), put_size);
                if (!once)
                    break;

                memcpy(pptr(), pos, once);
                pbump(once);
                pos += once, put_size -= once;
            }

            return (size - put_size);
        }

        // д������
        template <typename ConstBufferSequence>
        size_type sputn_to_buffers(ConstBufferSequence const& buffers)
        {
            size_type count = 0;
            typename ConstBufferSequence::iterator it = buffers.begin();
            for (; it != buffers.end(); ++it)
            {
                size_type once = sputn(detail::buffer_cast_helper(*it),
                    detail::buffer_size_helper(*it));
                count += once;
                if (once < detail::buffer_size_helper(*it))
                    break;
            }
            return count;
        }

        // ��д��������ȡ��MutableBufferSequence concept
        template <typename MutableBuffer>
        size_type put_buffers(boost::array<MutableBuffer, 2> & out) const
        {
            size_type ps = psize(), pc = pcount();
            size_type c1 = pc; 
            size_type c2 = (ps > pc) ? (ps - pc) : 0;

            out[0] = MutableBuffer(pptr(), c1);
            out[1] = MutableBuffer(buffer_, c2);
            return c2 ? 2 : (c1 ? 1 : 0);
        }

        // @}
        ////////////////////////////////////////////.ipp///////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        // @{ get thread methods
        // ��ָ��
        const_pointer gptr() const
        {
            return increment(get_);
        }

        // �����Ŀɶ�����
        size_type gcount() const
        {
            const_pointer gb = increment(gbegin()), ge = increment(gend());
            return (gb > ge) ? distance(gb, end()) : distance(gb, ge);
        }

        // ��ָ������ƶ�offset��λ��
        void gbump(difference_type offset)
        {
            get_ = advance(get_, offset);
        }

        // �ܼƵĿɶ�����
        size_type gsize() const
        {
            return distance(gbegin(), gend());
        }

        // ��ȡ����
        size_type sgetn(pointer data, size_type size)
        {
            pointer pos = data;
            size_type get_size = size;
            while (get_size)
            {
                size_type once = (std::min)(gcount(), get_size);
                if (!once)
                    break;

                memcpy(pos, gptr(), once);
                gbump(once);
                pos += once, get_size -= once;
            }

            return (size - get_size);
        }

        // ��ȡ����
        template <typename MutableBufferSequence>
        size_type sgetn_to_buffers(MutableBufferSequence & buffers)
        {
            size_type count = 0;
            typename MutableBufferSequence::iterator it = buffers.begin();
            for (; it != buffers.end(); ++it)
            {
                size_type once = sgetn(detail::buffer_cast_helper(*it),
                    detail::buffer_size_helper(*it));
                count += once;
                if (once < detail::buffer_size_helper(*it))
                    break;
            }
            return count;
        }

        // ��д��������ȡ��ConstBufferSequence concept
        template <typename ConstBuffer>
        size_type get_buffers(boost::array<ConstBuffer, 2> & out) const
        {
            size_type gs = gsize(), gc = gcount();
            size_type c1 = gc; 
            size_type c2 = (gs > gc) ? (gs - gc) : 0;

            out[0] = ConstBuffer(gptr(), c1);
            out[1] = ConstBuffer(buffer_, c2);
            return c2 ? 2 : (c1 ? 1 : 0);
        }

        // @}
        //////////////////////////////////////////////////////////////////////////

    private:
        pointer pbegin() const
        {
            return put_;
        }
        pointer pend() const
        {
            return get_;
        }

        const_pointer gbegin() const
        {
            return get_;
        }
        const_pointer gend() const
        {
            return decrement(put_);
        }

        const_pointer end() const
        {
            return buffer_ + capacity_;
        }
        
        difference_type distance(const_pointer const lhs, const_pointer const rhs) const
        {
            return (rhs >= lhs) ? (rhs - lhs) : (rhs + capacity_ - lhs);
        }

        template <typename Pointer>
        Pointer advance(Pointer const pos, difference_type offset) const
        {
            Pointer result = pos + offset;
            return (result >= end()) ? (result - capacity_) : result;
        }

        template <typename Pointer>
        Pointer retreat(Pointer const pos, difference_type offset) const
        {
            Pointer result = pos - offset;
            return (pos < buffer_ + offset) ? (result + capacity_) : result;
        }

        template <typename Pointer>
        Pointer increment(Pointer const pos) const
        {
            return advance(pos, 1);
        }

        template <typename Pointer>
        Pointer decrement(Pointer const pos) const
        {
            return retreat(pos, 1);
        }

    private:
        // �������׵�ַ
        pointer buffer_;

        // ����������
        size_type capacity_;

        // дָ�� [pbegin, pend)
        pointer volatile put_;

        // ��ָ�� [gbegin, gend)
        pointer volatile get_;
    };

} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_NONBLOCKING_CIRCULARBUFFER_HPP__
