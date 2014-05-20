#ifndef __BEX_UTILITY_OPERATORS_H__
#define __BEX_UTILITY_OPERATORS_H__

//////////////////////////////////////////////////////////////////////////
/// ����������ĺ궨��

//////////////////////////////////////////////////////////////////////////
/// �����������

/// ��ѧ����
#define BEX_OPERATOR_ADD                        +       ///< �ӷ�
#define BEX_OPERATOR_UNARY_PLUS                 +       ///< ��������
#define BEX_OPERATOR_INC                        ++      ///< ��1(��ǰ�úͺ�������)
#define BEX_OPERATOR_ADD_ASSIGN                 +=      ///< �ӷ� and ��ֵ
#define BEX_OPERATOR_SUB                        -       ///< ����
#define BEX_OPERATOR_UNARY_NEGATION             -       ///< ��������
#define BEX_OPERATOR_DEC                        --      ///< ��1(��ǰ�úͺ�������)
#define BEX_OPERATOR_SUB_ASSIGN                 -=      ///< ���� and ��ֵ
#define BEX_OPERATOR_MULTI                      *       ///< �˷�
#define BEX_OPERATOR_MULTI_ASSIGN               *=      ///< �˷� and ��ֵ
#define BEX_OPERATOR_DIV                        /       ///< ����
#define BEX_OPERATOR_DIV_ASSIGN                 /=      ///< ���� and ��ֵ
#define BEX_OPERATOR_MODULE                     %       ///< ȡģ
#define BEX_OPERATOR_MODULE_ASSIGN              %=      ///< ȡģ and ��ֵ

/// ����������
#define BEX_OPERATOR_BIT_AND                    &       ///< λ��
#define BEX_OPERATOR_BIT_AND_ASSIGN             &=      ///< λ�� and ��ֵ
#define BEX_OPERATOR_BIT_OR                     |       ///< λ��
#define BEX_OPERATOR_BIT_OR_ASSIGN              |=      ///< λ�� and ��ֵ
#define BEX_OPERATOR_BIT_NOT                    ~       ///< λ��
#define BEX_OPERATOR_EXCLUSIVE_OR               ^       ///< λ���
#define BEX_OPERATOR_EXCLUSIVE_OR_ASSIGN        ^=      ///< λ��� and ��ֵ
#define BEX_OPERATOR_LEFT_SHIFT                 <<      ///< ��λ����
#define BEX_OPERATOR_LEFT_SHIFT_ASSIGN          <<=     ///< ��λ���� and ��ֵ
#define BEX_OPERATOR_RIGHT_SHIFT                >>      ///< ��λ����
#define BEX_OPERATOR_RIGHT_SHIFT_ASSIGN         >>=     ///< ��λ���� and ��ֵ

/// �߼�����
#define BEX_OPERATOR_LOGICAL_NOT                !       ///< �߼���
#define BEX_OPERATOR_LOGICAL_AND                &&      ///< �߼���
#define BEX_OPERATOR_LOGICAL_OR                 ||      ///< �߼���

/// �Ƚ�
#define BEX_OPERATOR_LESS                       <       ///< С��
#define BEX_OPERATOR_LESS_OR_EQUAL              <=      ///< С��or����
#define BEX_OPERATOR_EQUAL                      ==      ///< ����
#define BEX_OPERATOR_GREATER                    >       ///< ����
#define BEX_OPERATOR_GREATER_OR_EQUAL           >=      ///< ����or����
#define BEX_OPERATOR_NOT_EQUAL                  !=      ///< ������

/// ���������
#define BEX_OPERATOR_COMMA                      ,       ///< ���Ų�����
#define BEX_OPERATOR_ADDRESS_OF                 &       ///< ȡ��ַ������
#define BEX_OPERATOR_INVOKE                     ()      ///< ��������
#define BEX_OPERATOR_DEREFERENCE                *       ///< �����ò�����
#define BEX_OPERATOR_MEMBER_SELECT              ->
#define BEX_OPERATOR_POINTER_MEMBER_SELECT      ->*     ///< ���ó�Ա����ָ��
#define BEX_OPERATOR_ASSIGN                     =       ///< ��ֵ
#define BEX_OPERATOR_SUBSCRIPT                  []      ///< �±�ȡֵ
#define BEX_OPERATOR_NEW                        new     ///< �����ڴ�
#define BEX_OPERATOR_DELETE                     delete  ///< �ͷ��ڴ�

//////////////////////////////////////////////////////////////////////////
/// �������������
#define BEX_OPERATOR_MEMBER                     .
#define BEX_OPERATOR_POINT_MEMBER               .*
#define BEX_OPERATOR_SCOPE                      ::
#define BEX_OPERATOR_CONDITION                  ? :
#define BEX_OPERATOR_PERPROCESS_1               #
//#define BEX_OPERATOR_PERPROCESS_2             ##

#endif //__BEX_UTILITY_OPERATORS_H__