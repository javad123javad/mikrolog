/**
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H


#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#define UNIT_TEST

namespace logger_nm
{
#define MAX_CALLBACKS 32
#define MAX_MSG_SIZE 512

  typedef struct
  {
    va_list ap;
    const char *fmt;
    struct tm *time;
    void *udata;
    int level;
  } log_Event;

  enum
    {
      LOG_TRACE,
      LOG_DEBUG,
      LOG_INFO,
      LOG_WARN,
      LOG_ERROR,
      LOG_FATAL
    };

  class logger
  {
  private:
    FILE *log_file;
    const char *msg_buf; //[MAX_MSG_SIZE]{0};


    typedef void (*log_LogFn)(log_Event *ev);
    typedef void (*log_LockFn)(bool lock, void *udata);

    typedef struct
    {
      log_LogFn fn;
      void *udata;
      int level;
    } Callback;

    struct
    {
      void *udata;
      log_LockFn lock;
      int level;
      bool quiet;
      Callback callbacks[MAX_CALLBACKS];
    } lock_var;

  private:
    static logger *instance;
    logger(const char *const log_file_name = NULL, int8_t level = LOG_TRACE);
    ~logger();

    static void file_callback(log_Event *ev);
    void lock(void);
    void unlock(void);
    // void init_event(log_Event *ev, void *udata);

#if defined(UNIT_TEST) && !defined(EMBEDDED_UNIT_TEST)
    virtual void log_console(int level, const char *fmt, ...);
#else
    virtual void log_serial(int level, const char *fmt, ...);
#endif

  public:
    static logger *get_instance(const char *const log_file_name = NULL, int8_t level = LOG_TRACE);
    const char *log_level_string(int level);
    void log_set_lock(log_LockFn fn, void *udata);
    void log_set_level(int level);
    void log_set_quiet(bool enable);
    int log_add_callback(log_LogFn fn, void *udata, int level);
    int log_add_fp(FILE *fp, int level);
    void log_simple(const int level, const char *file, const int line, const char *msg);
    template <typename... Args>
    void log_log(int level, char const *const format, Args const &...args)
    {
#if defined(UNIT_TEST) && !defined(EMBEDDED_UNIT_TEST)

      log_console(level, format, args...);
#else
      log_serial(level, format, args...);

#endif
    }

    template <typename... Args>
    void log_trace(char const *const format, Args const &...args)
    {
      log_log(LOG_TRACE, format, args...);
    }

    template <typename... Args>
    void log_info(char const *const format, Args const &...args)
    {

      log_log(LOG_INFO, format, args...);
    }

    template <typename... Args>
    void log_warn(char const *const format, Args const &...args)
    {
      log_log(LOG_WARN, format, args...);
    }

    template <typename... Args>
    void log_error(char const *const format, Args const &...args)
    {
      log_log(LOG_ERROR, format, args...);
    }

    template <typename... Args>
    void log_fatal(char const *const format, Args const &...args)
    {
      log_log(LOG_FATAL, format, args...);
    }
  };

} // namespace logger_nm

#endif
