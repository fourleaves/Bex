#ifndef __BEX_BIND_FORWARD_BIND_FWD_H__
#define __BEX_BIND_FORWARD_BIND_FWD_H__

#include <tuple>
#include <type_traits>

namespace Bex
{
    namespace forward_bind
    {
        //////////////////////////////////////////////////////////////////////////
        /// ����
        template <typename ... Args>
        using Tuple = std::tuple<Args...>;

        // ռλ��
        template <int I>
        struct placeholder;

        // �ɰ�callable���������ö��
        enum class callable_type : int
        {
            free_function,      // ���ɺ���
            mem_function,       // ���Ա����
            object_function,    // �º�������
            mem_variable,       // ���Ա����
        };

        // ���÷�ʽ(�������ص����Ͷ���)
        template <callable_type Ct>
        using call_t = std::integral_constant<callable_type, Ct>;

        using free_function_calltype = call_t<callable_type::free_function>;
        using mem_function_calltype = call_t<callable_type::mem_function>;
        using object_function_calltype = call_t<callable_type::object_function>;
        using mem_variable_calltype = call_t<callable_type::mem_variable>;

        // bind�����
        template <typename R, typename F, typename ... BArgs>
        class bind_t;

        // callable����������ȡ
        // �ṩ: 1.callable_typeö��ֵ
        //      2.callable_type���÷�ʽ����
        //      3.result_type
        //      4.ʵ�θ���
        template <typename F>
        struct callable_traits;

        // ռλ��������ȡ
        // �ṩ: 1.�Ƿ���ռλ�����ж�
        //      2.ռλ��������
        template <typename T>
        struct placeholder_traits;

        // ѡ��ʵ��
        template <typename BindArgs, typename CallArgs, unsigned int N>
        struct select;

        // ��������ת��Ϊ��������������ʽ
        template <unsigned int ... N> struct seq {};
        template <unsigned int N, unsigned int ... S> struct gen;
        template <unsigned int N, unsigned int ... S> struct gen : gen<N - 1, N - 1, S...> {};
        template <unsigned int ...S> struct gen<0, S...>
        {
            typedef seq<S...> type;
        };

    } //namespace forward_bind
} //namespace Bex

#endif //__BEX_BIND_FORWARD_BIND_FWD_H__