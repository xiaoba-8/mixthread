AUTOMAKE_OPTIONS= \
  foreign \
  subdir-objects
  
INCLUDES=-I../include

bin_PROGRAMS=mix_thread_demo 
mix_thread_demo_SOURCES= \
  demo_main.cpp
mix_thread_demo_LDADD=../lib/libmixthread.la
mix_thread_demo_LDFLAGS=-pthread
