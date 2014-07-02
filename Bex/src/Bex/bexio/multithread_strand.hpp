#ifndef __BEX_IO_MULTITHREAD_STRAND_HPP__
#define __BEX_IO_MULTITHREAD_STRAND_HPP__

//////////////////////////////////////////////////////////////////////////
// ���߳�->���̴߳�����
/*
* @par concept:
*   io_service, Strand
*/

#include "bexio_fwd.hpp"
#include "multithread_strand_fwd.hpp"
#include "allocator.hpp"

namespace Bex { namespace bexio 
{
    using namespace boost::asio;

    template <class Strand, class Allocator>
    class multithread_strand
    {
    public:
        // next layer type
        typedef typename boost::remove_reference<Strand>::type next_layer_type;

        // lowest layer type
        typedef typename next_layer_t<Strand>::type lowest_layer_type;

    public:
        template <typename Arg>
        explicit multithread_strand(BEX_IO_MOVE_ARG(Arg) arg)
            : next_layer_(BEX_IO_MOVE_CAST(Arg)(arg))
        {
        }

        // ��ȡ�²�strand����
        next_layer_type & next_layer();

        // ��ȡ��ײ�strand����
        lowest_layer_type & lowest_layer();

        // ��ȡ��ǰ���io_service
        io_service & actor();

        // post��ɻص�
        template <typename Handler>
        void post(BEX_IO_MOVE_ARG(Handler) handler);

        // ������ɻص�
        std::size_t run(error_code & ec);

        // ������ɻص�(noexcept)
        std::pair<std::size_t, error_code> run();

        // �����̳߳ع���
        // @thread_count: �����߳�����, ��thread_count==0ʱ, ��thread_count��дΪ��ǰcpu�߳���.
        // @remarks: �����ǰ���й����߳��ڹ���, �ҹ����߳�����С��thread_count, �����ӹ����߳�����thread_count.
        void startup(unsigned int thread_count);

        // ֹͣ�̳߳ع���
        void stop();

        // �����̳߳�
        void reset();

        // ���ŵعر�
        void shutdown();

        // ǿ����ֹ
        void terminate();

        // �̳߳��Ƿ���ֹͣ����
        bool stopped();

    private:
        // �²�strand
        Strand next_layer_;

        // ���ж�������̵߳Ļص�
        io_service actor_;

        // �����߳�����
        unsigned int thread_count_;

        // �������������̵߳�worker
        boost::shared_ptr<io_service::work> worker_;

        // �����߳��б�
        typedef boost::shared_ptr<boost::thread> thread_ptr;
        typedef std::list<thread_ptr, Allocator> thread_list;
        thread_list threads_;

        // �����߳��б���
        boost::recursive_mutex threads_mutex_;
    };

} //namespace bexio
} //namespace Bex

#include "detail/multithread_strand.ipp"

#endif //__BEX_IO_MULTITHREAD_STRAND_HPP__
