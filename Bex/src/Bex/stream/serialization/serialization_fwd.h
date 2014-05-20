#ifndef __BEX_STREAM_SERIALIZATION_SERIALIZATION_FWD_HPP__
#define __BEX_STREAM_SERIALIZATION_SERIALIZATION_FWD_HPP__

//////////////////////////////////////////////////////////////////////////
/// stream.serialization��ǰ�������ļ�

#ifndef BEX_STREAM_SERIALIZATION_VERSION_NAME
#define BEX_STREAM_SERIALIZATION_VERSION_NAME bex_serialization_version
#endif //BEX_STREAM_SERIALIZATION_VERSION_NAME

/// ���ڶ���ṹ�����л��汾�ŵ�����ö����
#define BEX_SS_VERSION BEX_STREAM_SERIALIZATION_VERSION_NAME

/// �����ӻ���̳й����İ汾��
#define BEX_SS_UNDEF_BASE_VERSION(Base) private: using Base::BEX_SS_VERSION;

namespace Bex { namespace serialization
{
    //////////////////////////////////////////////////////////////////////////
    /// archive mark
    enum archive_mark_bit
    {
        amb_none = 0x0,
        amb_rollback = 0x1,        ///< ���reback������, ��/дʧ��ʱ, �������ỹԭ������ǰ��״̬��λ��.
    };
    typedef int archive_mark;
    static const archive_mark default_mark = amb_none;

    //////////////////////////////////////////////////////////////////////////
    /// @{ Archive����ö��
    enum BEX_ENUM_CLASS archive_mode_enum
    {
        archive_mode_binary,
        archive_mode_text,
        archive_mode_unkown,
    };
    struct binary_mode_tag {};
    struct text_mode_tag {};
    struct unkown_mode_tag {};
    /// @}
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    /// @{ Archive��������ö��
    enum BEX_ENUM_CLASS archive_oper_enum
    { 
        archive_oper_load, 
        archive_oper_save, 
        archive_oper_unkown,
    };
    struct load_oper_tag {};
    struct save_oper_tag {};
    struct unkown_oper_tag {};
    /// @}
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    /// @{ ���ݰ�װ��
    /// ��������������װ��
    struct binary_wrapper;

    /// �ı���������װ��
    template <typename T>
    struct text_wrapper;
    /// @}
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    /// binary archive base
    struct binary_base {};

    /// text archive base
    struct text_base {};

    /// i/o base
    struct input_archive_base {};
    struct output_archive_base {};

    /// archive traits
    template <class Archive>
    struct archive_traits;

    /// �ж������Ƿ����ػ�Ϊ�����
    template <typename T>
    struct is_adapter_type;
    //////////////////////////////////////////////////////////////////////////

} //namespace serialization

namespace {
    using serialization::archive_mark;
    using serialization::amb_none;
    using serialization::amb_rollback;

    using serialization::load_oper_tag;
    using serialization::save_oper_tag;
    using serialization::unkown_oper_tag;
} //namespace

} //namespace Bex

#endif //__BEX_STREAM_SERIALIZATION_SERIALIZATION_FWD_HPP__