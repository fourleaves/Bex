#ifndef __BEX_IO_HTTP_HTTP_OPTIONS_HPP__
#define __BEX_IO_HTTP_HTTP_OPTIONS_HPP__

#include <Bex/bexio/bexio_fwd.hpp>
#include <boost/algorithm/string.hpp>

namespace Bex { namespace bexio { namespace http
{ 
    
// ���û�ж�������ض������, ��Ĭ��Ϊ5������ض���.
#ifndef BEXIO_HTTP_MAX_REDIRECTS
#define BEXIO_HTTP_MAX_REDIRECTS 5
#endif

// ����httpѡ��.
namespace option
{
	// ����Ϊbexio����ѡ��(������"_"��ͷ).
	static const std::string request_method("_request_method"); // ����ʽ(GET/POST)
	static const std::string http_version("_http_version");		// HTTP/1.0|HTTP/1.1
	static const std::string request_body("_request_body");		// һ������POSTһЩ�������֮��ʱʹ��.
	static const std::string status_code("_status_code");	    // HTTP״̬��.
	static const std::string status_str("_status_str");	        // HTTP״̬�ַ���(����״̬������, ����:OK).
	static const std::string path("_path");		// �����path, ��http://abc.ed/v2/cma.txt�е�/v2/cma.txt.
	static const std::string url("_url");		// ������keep-alive��ʱ��, ����host�ϲ�ͬ��urlʱʹ��.

	// �����ǳ��õı�׼http headѡ��.
	static const std::string host("Host");
	static const std::string accept("Accept");
	static const std::string range("Range");
	static const std::string cookie("Cookie");
	static const std::string referer("Referer");
	static const std::string user_agent("User-Agent");
	static const std::string content_type("Content-Type");
	static const std::string content_length("Content-Length");
	static const std::string content_range("Content-Range");
	static const std::string connection("Connection");
	static const std::string proxy_connection("Proxy-Connection");
	static const std::string accept_encoding("Accept-Encoding");
	static const std::string transfer_encoding("Transfer-Encoding");
	static const std::string content_encoding("Content-Encoding");

    // �ָ���
    static const std::string line_end("\r\n");
    static const std::string opt_end("\r\n\r\n");

    // ����Ƿ�������ѡ��
    inline bool is_builtin_option(std::string const& key)
    {
        return !key.empty() && key[0] == '_';
    }
} // namespace option


// http��ѡ��洢.
class options
{
public:
    typedef std::string opt_key_type;
    typedef std::string opt_value_type;
    typedef std::pair<opt_key_type, opt_value_type> opt_type;
    typedef std::vector<opt_type> opts;
    typedef opts::iterator iterator;
    typedef opts::const_iterator const_iterator;

    options() : fake_continue_(false) {}

    // ��ȡָ��ѡ���ֵ
    opt_value_type get(opt_key_type const& _key) const
    {
        opt_key_type key = boost::to_lower_copy(_key);
        const_iterator cit = std::find_if(opts_.begin(), opts_.end(), [](opt_type const& opt, opt_key_type const& key) {
            return opt.first == key;
        });
        return (opts_.end() == cit) ? opt_value_type() : (cit->second);
    }

    // ����ָ��ѡ���ֵ
    void set(opt_key_type const& _key, opt_value_type const& value)
    {
        opt_key_type key = boost::to_lower_copy(_key);
        iterator it = std::find_if(opts_.begin(), opts_.end(), [](opt_type const& opt, opt_key_type const& key) {
            return opt.first == key;
        });
        if (opts_.end() == it)
            opts_.push_back(opt_type(key, value));
        else
            it->second = value;
    }

    std::string to_string() const
    {
        using namespace option;

        std::string s;
        opt_value_type v_method = get(request_method);
        opt_value_type v_version = get(http_version);
        opt_value_type v_path = get(path);
        opt_value_type v_status_code = get(status_code);
        opt_value_type v_status_str = get(status_str);
        if (v_method.empty())
        {
            // response
            s += v_version, s += " ";
            s += v_status_code, s += " ";
            s += v_status_str, s += line_end;
        }
        else
        {
            // request
            s += v_method, s += " ";
            s += v_path, s += " ";
            s += v_version, s += line_end;
        }
    }

protected:
	// ѡ���б�.
	opts opts_;

	// �Ƿ����ü�100 continue��Ϣ, �������, ���ڷ������http request head
	// ֮��, ����һ��fake continue��Ϣ.
	bool fake_continue_;
};

// ����ʱ��httpѡ��.
// _http_version, ȡֵ "HTTP/1.0" / "HTTP/1.1", Ĭ��Ϊ"HTTP/1.1".
// _request_method, ȡֵ "GET/POST/HEAD", Ĭ��Ϊ"GET".
// _request_body, �����е�body����, ȡֵ����, Ĭ��Ϊ��.
// Host, ȡֵΪhttp������, Ĭ��Ϊhttp������.
// Accept, ȡֵ����, Ĭ��Ϊ"*/*".
// ��Щ�Ƚϳ��õ�ѡ�������http_options��.
typedef option request_opts;

// http���������ص�httpѡ��.
// һ���������¼���ѡ��:
// _http_version, ȡֵ "HTTP/1.0" / "HTTP/1.1", Ĭ��Ϊ"HTTP/1.1".
// _status_code, http����״̬.
// Server, ����������.
// Content-Length, �������ݳ���.
// Connection, ����״̬��ʶ.
typedef option response_opts;



// Http����Ĵ�������.

struct proxy_settings
{
	proxy_settings()
		: type (none)
	{}

	std::string hostname;
	int port;

	std::string username;
	std::string password;

	enum proxy_type
	{
		// û�����ô���.
		none,
		// socks4����, ��Ҫusername.
		socks4,
		// ����Ҫ�û������socks5����.
		socks5,
		// ��Ҫ�û�������֤��socks5����.
		socks5_pw,
		// http����, ����Ҫ��֤.
		http,
		// http����, ��Ҫ��֤.
		http_pw,
	};

	proxy_type type;
};


// һЩĬ�ϵ�ֵ.
static const int default_request_piece_num = 10;
static const int default_time_out = 11;
static const int default_connections_limit = 5;
static const int default_buffer_size = 1024;

// multi_download��������.

struct settings
{
	settings ()
		: download_rate_limit(-1)
		, connections_limit(default_connections_limit)
		, piece_size(-1)
		, time_out(default_time_out)
		, request_piece_num(default_request_piece_num)
		, allow_use_meta_url(true)
		, disable_multi_download(false)
		, check_certificate(true)
		, storage(NULL)
	{}

	// ������������, -1Ϊ������, ��λΪ: byte/s.
	int download_rate_limit;

	// ����������, -1ΪĬ��.
	int connections_limit;

	// �ֿ��С, Ĭ�ϸ����ļ���С�Զ�����.
	int piece_size;

	// ��ʱ�Ͽ�, Ĭ��Ϊ11��.
	int time_out;

	// ÿ������ķ�Ƭ��, Ĭ��Ϊ10.
	int request_piece_num;

	// meta_file·��, Ĭ��Ϊ��ǰ·����ͬ�ļ�����.meta�ļ�.
	fs::path meta_file;

	// ����ʹ��meta�б����url, Ĭ��Ϊ����. ���һЩ�䶯��url, ����Ӧ�ý���.
	bool allow_use_meta_url;

	// ��ֹʹ�ò�������.
	// NOTE: �������ڶ�̬ҳ������, ��Ϊ��̬ҳ�治��ʹ�ò�������, ����������ʹ��
	// multi_download��������, ����Ҫ�����������, ���򲢷����ض�̬���ݵ���Ϊ, ��
	// δ�����, ����������Ϊ���ݳ��Ȳ�һ�����¶���, Ҳ�������ݴ���.
	// NOTE: ���Ƽ�ʹ��multi_download��������, ��Ӧ��ʹ��http_stream��������,
	// multi_download��ҪӦ���ڴ��ļ�, ��̬ҳ������!
	bool disable_multi_download;

	// �����ļ�·��, Ĭ��Ϊ��ǰĿ¼.
	fs::path save_path;

	// �����Ƿ���֤��, Ĭ�ϼ��֤��.
	bool check_certificate;

	// �洢�ӿڴ�������ָ��, Ĭ��Ϊmulti_download�ṩ��file.hppʵ��.
	storage_constructor_type storage;

	// ����ѡ��.
	request_opts opts;

	// ��������.
	proxy_settings proxy;
};

} //namespace http
} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_HTTP_HTTP_OPTIONS_HPP__