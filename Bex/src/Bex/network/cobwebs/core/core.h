#ifndef __BEX_NETWORK_COBWEBS_CORE_H__
#define __BEX_NETWORK_COBWEBS_CORE_H__

//////////////////////////////////////////////////////////////////////////
// cobwebs, �ײ������.
/* core:
* \�������, �ṩio�������̳߳ط���.
*/

#if defined(_MSC_VER) || defined(_WIN32)
#include <sdkddkver.h>
#pragma warning(disable:4996)
#endif

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <string>

#include <Bex/signals/signal_fwd.h>
#include <Bex/utility/singleton.hpp>
#include <Bex/platform.hpp>
#include <Bex/thread.hpp>

#include "id.h"
#include "error.hpp"
#include "pool.hpp"

namespace Bex { namespace cobwebs
{
    using namespace boost::asio;
    using namespace boost::asio::ip;

    class core
        : public singleton<core>
    {
        class impl;
        boost::shared_ptr<impl> m_impl;

        Pool m_pool;

    public:
        typedef boost::function<void()> RunFunction;
        typedef boost::function<void()> PostHandler;

    public:
        core();

        // ����
        bool startup(unsigned int threads = 0);

        // �Ƿ�����
        bool is_running() const;

        // �ر�
        void shutdown();

        // ȡ��io_service
        io_service& get_io_service();

        // ���ݽ����߳��ƽ�
        void run();

        // ��ȡ�ڴ��
        Pool& get_pool();

        // Ͷ��һ����������ѭ�������
        void post(PostHandler const& handler);

        // ע��run
        signal_scoped register_run(RunFunction const& run_func, int group = 0);
    };

} //namespace cobwebs
} //namespace Bex

#endif //__BEX_NETWORK_COBWEBS_CORE_H__