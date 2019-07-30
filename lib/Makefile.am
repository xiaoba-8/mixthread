AUTOMAKE_OPTIONS= \
  foreign \
  subdir-objects
  
INCLUDES=-I../include
  
#noinst_LIBRARIES=libmixthread.a     #static library
lib_LTLIBRARIES=libmixthread.la 
libmixthread_la_SOURCES= \
  mutex/thread_mutex.cpp \
  thread/mix_thread.cpp \
  thread/thread_proxy.cpp \
  thread/mix_thread_container.cpp \
  thread/i_monitor.cpp \
  thread/i_runnable.cpp \
  thread/thread_pool.cpp \
  utils/thread_exception.cpp \
  utils/stringutil.cpp \
  utils/time-tzset.c 

mixdir=$(includedir)/mix
mix_HEADERS=../include/mix/thread_errno.h

mutexdir=$(includedir)/mix/mutex
mutex_HEADERS=../include/mix/mutex/thread_mutex.h

threaddir=$(includedir)/mix/thread
thread_HEADERS=../include/mix/thread/mix_thread.h \
  ../include/mix/thread/thread_proxy.h \
  ../include/mix/thread/i_runnable.h \
  ../include/mix/thread/mix_thread_container.h \
  ../include/mix/thread/i_monitor.h \
  ../include/mix/thread/thread_pool.h
  
utilsdir=$(includedir)/mix/utils
utils_HEADERS=../include/mix/utils/thread_exception.h \
  ../include/mix/utils/stringutil.h \
  ../include/mix/utils/time-tzset.h 
