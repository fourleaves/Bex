#ifndef __BEX_SERIALIZATION_CONFIG_H__
#define __BEX_SERIALIZATION_CONFIG_H__

//////////////////////////////////////////////////////////////////////////
// config serialization.

// ��δ����serialize��serialize_version���Զ������͵���pod���ʹ���
/**
 * ������������������, ���Ƽ����ô�ѡ��!
 */
#if defined(BEX_SERIALIZATION_POD_EXTEND)
# define BEX_SERIALIZATION_USE_POD_EXTEND 1
#endif //defined(BEX_SERIALIZATIOIN_POD_EXTEND)

#endif //__BEX_SERIALIZATION_CONFIG_H__