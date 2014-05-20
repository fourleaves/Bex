#ifndef __BEX_TYPE_TRAITS_CLASS_INFO_HPP__
#define __BEX_TYPE_TRAITS_CLASS_INFO_HPP__

//////////////////////////////////////////////////////////////////////////
/// class_info ����ʶ�������Ϣ
#include <Bex/type_traits/class_info/has_const_int.hpp>
#include <Bex/type_traits/class_info/has_member_data.hpp>
#include <Bex/type_traits/class_info/has_member_function.hpp>
#include <Bex/type_traits/class_info/has_static_member_data.hpp>
#include <Bex/type_traits/class_info/has_static_member_function.hpp>
#include <Bex/type_traits/class_info/has_template_function.hpp>
#include <Bex/type_traits/class_info/has_template_type.hpp>
#include <Bex/type_traits/class_info/has_type.hpp>
#include <Bex/type_traits/class_info/has_operator.hpp>
//////////////////////////////////////////////////////////////////////////

//struct X
//{
//    int member_data;                                ///< OK! ����
//    static int static_member_data;                  ///< OK! ����
//    void member_function(int);                      ///< OK! ����
//    static int static_member_function(double);      ///< OK! ����
//
//    /// typedef �� �Զ���������ʱ�޷�ϸ������
//    typedef int _typedef;           ///< OK!
//    struct declare_struct;          ///< OK!
//
//    template <typename T>
//    struct declare_template_struct; ///< ��ָ�������б�, ʵ�ֲ����ɱ����Դﵽ����
//
//    template <typename T>
//    void member_template_function();    ///< ��static��ͬ, �޷�ʶ����ǩ��
//    template <typename T>
//    static void static_member_template_function(); ///< ��static��ͬ, �޷�ʶ����ǩ��
//
//    static const int init_static_const_int_member_data = 0; ///< OK! ��ö����ͬ
//    enum { enum_number, }; ///< OK! ��static const int��ͬ
//};

#endif //__BEX_TYPE_TRAITS_CLASS_INFO_HPP__