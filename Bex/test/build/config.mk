GCC=g++
INCLUDEPATH=-I../main -I../../src -I../../../boost/include
CXXFLAGS=-std=c++11 -g -O0 -DDEBUG -D_DEBUG ${INCLUDEPATH}
DYNAMIC_LINK=-L../../../boost/lib -lpthread -L/usr/lib/i386-linux-gnu -lgnutls-openssl -lssl -lcrypto
STATIC_LINK=../../../boost/lib/libboost_date_time.a ../../../boost/lib/libboost_filesystem.a ../../../boost/lib/libboost_regex.a ../../../boost/lib/libboost_system.a ../../../boost/lib/libboost_thread.a ../../../boost/lib/libboost_unit_test_framework.a ../../../boost/lib/libboost_locale.a
objects+= TestThreadPool.o
objects+= TestLock.o
objects+= TestCharsetConvert.o
objects+= TestCompressNumeric.o
objects+= TestBit.o
objects+= Test_noblocking_circle.o
objects+= TestLayerMetaFunctions.o
objects+= Test_signature_traits.o
objects+= Test_class_info.o
objects+= Test_remove_all.o
objects+= Test_has_operator.o
objects+= TestPCH.o
objects+= TestMain.o
objects+= TestSignals.o
objects+= TestRSAStream.o
objects+= TestTextArchive.o
objects+= TestSerializationProperty.o
objects+= TestStream.o
objects+= test_while.o
objects+= TestLockPtr.o
objects+= Test_type_name.o
objects+= TestFormat.o
objects+= TestRange.o
objects+= Test_lexical_cast.o
objects+= TestForwardBind.o
VPATH+=../base:
VPATH+=../bexio:
VPATH+=../bind:
VPATH+=../build:
VPATH+=../crypto:
VPATH+=../main:
VPATH+=../math:
VPATH+=../mpl:
VPATH+=../platform:
VPATH+=../signals:
VPATH+=../stream:
VPATH+=../thread:
VPATH+=../type_traits:
VPATH+=../utility:
