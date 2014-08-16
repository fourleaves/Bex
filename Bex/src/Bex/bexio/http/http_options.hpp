#ifndef __BEX_IO_HTTP_HTTP_OPTIONS_HPP__
#define __BEX_IO_HTTP_HTTP_OPTIONS_HPP__

#include <Bex/bexio/bexio_fwd.hpp>
#include <boost/algorithm/string.hpp>

namespace Bex { 
namespace bexio { 
namespace http { 
    
// ���û�ж�������ض������, ��Ĭ��Ϊ5������ض���.
#ifndef BEXIO_HTTP_MAX_REDIRECTS
#define BEXIO_HTTP_MAX_REDIRECTS 5
#endif

enum BEX_ENUM_CLASS http_proxy_type
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
	static const std::string accept_language("Accept-Language");
	static const std::string transfer_encoding("Transfer-Encoding");
	static const std::string content_encoding("Content-Encoding");

    // �ָ���
    static const std::string line_end("\r\n"); // ����
    static const std::string header_end("\r\n\r\n"); // header������

    // Э��Ĭ�϶˿ں�
    static const int http_port = 80;
    static const int https_port = 443;
    static const int ftp_port = 21;

    // request method
    static const std::string rm_get("GET");
    static const std::string rm_post("POST");

    // http versions
    static const std::string http_1_0("HTTP/1.0");
    static const std::string http_1_1("HTTP/1.1");

    // ����Ƿ�������ѡ��
    inline bool is_builtin_option(std::string const& key)
    {
        return !key.empty() && key[0] == '_';
    }
} // namespace option


// http��ѡ��洢.
class http_header
{
public:
    typedef std::string opt_key_type;
    typedef std::string opt_value_type;
    typedef std::pair<opt_key_type, opt_value_type> opt_type;
    typedef std::vector<opt_type> opts;
    typedef opts::iterator iterator;
    typedef opts::const_iterator const_iterator;

    struct setter
    {
        http_header& ref_;
        explicit setter(http_header& ref) : ref_(ref) {}
        inline setter operator()(opt_key_type const& _key, opt_value_type const& value)
        {
            return ref_.set(_key, value);
        }
    };

    // @Todo: getter.
    // @Todo: httpͷ��, ͷ���������ظ�, ��Ҫǿ��ת���Сд!

    http_header() {}

    // ��ȡָ��ѡ���ֵ
    opt_value_type get(opt_key_type const& _key) const
    {
        opt_key_type key = boost::to_lower_copy(_key);
        const_iterator cit = std::find_if(opts_.begin(), opts_.end(), [&key](opt_type const& opt) {
            return opt.first == key;
        });
        return (opts_.end() == cit) ? opt_value_type() : (cit->second);
    }

    // ����ָ��ѡ���ֵ
    setter set(opt_key_type const& _key, opt_value_type const& value)
    {
        opt_key_type key = boost::to_lower_copy(_key);
        iterator it = std::find_if(opts_.begin(), opts_.end(), [&key](opt_type const& opt) {
            return opt.first == key;
        });
        if (opts_.end() == it)
            opts_.push_back(opt_type(key, value));
        else
            it->second = value;

        return setter(*this);
    }

    std::string to_string() const
    {
        using namespace option;

        std::string s;
        opt_value_type v_method = get(request_method);
        opt_value_type v_version = get(http_version);
        if (v_method.empty())
        {
            // response
            opt_value_type v_status_code = get(status_code);
            opt_value_type v_status_str = get(status_str);
            s += v_version, s += " ";
            s += v_status_code, s += " ";
            s += v_status_str, s += line_end;
        }
        else
        {
            // request
            opt_value_type v_path = get(path);
            s += v_method, s += " ";
            s += v_path, s += " ";
            s += v_version, s += line_end;
        }

        for (const_iterator cit = opts_.begin(); cit != opts_.end(); ++cit)
        {
            opt_key_type const& key = cit->first;
            opt_value_type const& value = cit->second;
            if (is_builtin_option(key))
                continue;

            s += key, s += ":", s += value, s += line_end;
        }

        s += line_end;
        return s;
    }

    /// The http header looks like:
    //GET /123456 HTTP/1.1
    //Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, application/vnd.ms-excel, application/vnd.ms-powerpoint, application/msword, */*
    //Accept-Language: zh-cn
    //Accept-Encoding: gzip, deflate
    //User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; Maxthon; .NET CLR 1.1.4322)
    //Host: 10.1.1.238:8000
    //Connection: Keep-Alive
    //
    std::size_t from_string(const char* s)
    {
        using namespace option;

        static boost::regex re(regex::ignore_white(re_httpheader));

        const char* end = strstr(s, header_end.c_str());
        if (!end || !*end)
            return 0;

        std::string match_s(s, end + header_end.size());

        boost::cmatch mrs;
        if (!boost::regex_search(match_s.c_str(), mrs, re
              , boost::regex_constants::match_single_line | boost::regex_constants::format_all))
            return 0;

        if (mrs.size() < 4)
            return 0;

        bool is_response = boost::all(mrs[2].str(), boost::is_digit());
        if (is_response)
        {
            // response
            set(http_version, mrs[1].str());
            set(status_code, mrs[2].str());
            set(status_str, mrs[3].str());
        }
        else
        {
            // request
            set(request_method, mrs[1].str());
            set(path, mrs[2].str());
            set(http_version, mrs[3].str());
        }

        for ( std::size_t ui = 4; ui + 1 < mrs.size(); ui += 2 )
        {
            if (mrs[ui].str().empty())
                continue;

            set(mrs[ui].str(), mrs[ui + 1].str());
        }

        return mrs[0].str().length();
    }

    /// http�������ݳ���
    std::size_t body_size() const
    {
        opt_value_type clength = get(option::content_length);
        return (std::size_t)std::atoi(clength.c_str());
    }

protected:
	// ѡ���б�.
	opts opts_;

	//// �Ƿ����ü�100 continue��Ϣ, �������, ���ڷ������http request head
	//// ֮��, ����һ��fake continue��Ϣ.
	//bool fake_continue_;
};

// ����ʱ��httpѡ��.
// _http_version, ȡֵ "HTTP/1.0" / "HTTP/1.1", Ĭ��Ϊ"HTTP/1.1".
// _request_method, ȡֵ "GET/POST/HEAD", Ĭ��Ϊ"GET".
// _request_body, �����е�body����, ȡֵ����, Ĭ��Ϊ��.
// Host, ȡֵΪhttp������, Ĭ��Ϊhttp������.
// Accept, ȡֵ����, Ĭ��Ϊ"*/*".
// ��Щ�Ƚϳ��õ�ѡ�������http_options��.
typedef http_header request_header;

// http���������ص�httpѡ��.
// һ���������¼���ѡ��:
// _http_version, ȡֵ "HTTP/1.0" / "HTTP/1.1", Ĭ��Ϊ"HTTP/1.1".
// _status_code, http����״̬.
// Server, ����������.
// Content-Length, �������ݳ���.
// Connection, ����״̬��ʶ.
typedef http_header response_header;



// Http����Ĵ�������.
struct proxy_options
{
	proxy_options()
        : type(http_proxy_type::none), port(option::http_port)
	{}

	std::string hostname;
	int port;

	std::string username;
	std::string password;

	http_proxy_type type;
};


//// һЩĬ�ϵ�ֵ.
//static const int default_request_piece_num = 10;
//static const int default_time_out = 11;
//static const int default_connections_limit = 5;
//static const int default_buffer_size = 1024;
//
//// multi_download��������.
//
//struct settings
//{
//	settings ()
//		: download_rate_limit(-1)
//		, connections_limit(default_connections_limit)
//		, piece_size(-1)
//		, time_out(default_time_out)
//		, request_piece_num(default_request_piece_num)
//		, allow_use_meta_url(true)
//		, disable_multi_download(false)
//		, check_certificate(true)
//		, storage(NULL)
//	{}
//
//	// ������������, -1Ϊ������, ��λΪ: byte/s.
//	int download_rate_limit;
//
//	// ����������, -1ΪĬ��.
//	int connections_limit;
//
//	// �ֿ��С, Ĭ�ϸ����ļ���С�Զ�����.
//	int piece_size;
//
//	// ��ʱ�Ͽ�, Ĭ��Ϊ11��.
//	int time_out;
//
//	// ÿ������ķ�Ƭ��, Ĭ��Ϊ10.
//	int request_piece_num;
//
//	// meta_file·��, Ĭ��Ϊ��ǰ·����ͬ�ļ�����.meta�ļ�.
//	fs::path meta_file;
//
//	// ����ʹ��meta�б����url, Ĭ��Ϊ����. ���һЩ�䶯��url, ����Ӧ�ý���.
//	bool allow_use_meta_url;
//
//	// ��ֹʹ�ò�������.
//	// NOTE: �������ڶ�̬ҳ������, ��Ϊ��̬ҳ�治��ʹ�ò�������, ����������ʹ��
//	// multi_download��������, ����Ҫ�����������, ���򲢷����ض�̬���ݵ���Ϊ, ��
//	// δ�����, ����������Ϊ���ݳ��Ȳ�һ�����¶���, Ҳ�������ݴ���.
//	// NOTE: ���Ƽ�ʹ��multi_download��������, ��Ӧ��ʹ��http_stream��������,
//	// multi_download��ҪӦ���ڴ��ļ�, ��̬ҳ������!
//	bool disable_multi_download;
//
//	// �����ļ�·��, Ĭ��Ϊ��ǰĿ¼.
//	fs::path save_path;
//
//	// �����Ƿ���֤��, Ĭ�ϼ��֤��.
//	bool check_certificate;
//
//	// �洢�ӿڴ�������ָ��, Ĭ��Ϊmulti_download�ṩ��file.hppʵ��.
//	storage_constructor_type storage;
//
//	// ����ѡ��.
//	request_opts opts;
//
//	// ��������.
//	proxy_settings proxy;
//};

} //namespace http
} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_HTTP_HTTP_OPTIONS_HPP__