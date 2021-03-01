/*
 * Copyright (c) 2020 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "logger.hpp"

#include <string.h>
namespace logger_nm
{
  logger *logger::instance = nullptr;
  logger *logger::get_instance(const char *const log_file_name, int8_t level)
      {
        if (nullptr == instance)
        {
          instance = new logger(log_file_name, level);
        }
        return instance;
      }
  logger::logger(const char *const log_file_name, int8_t level)
  {
#if defined(UNIT_TEST) && !defined(EMBEDDED_UNIT_TEST)
    int8_t fRet = -1;
    if (NULL != log_file_name)
    {
      log_file = fopen(log_file_name, "w+");
      if (NULL != log_file)
      {
        fRet = log_add_fp(log_file, level);
      }
    }
#endif
  }

  logger::~logger()
  {
    if (NULL != log_file)
    {
      fflush(log_file);
      fclose(log_file);
    }
    instance = nullptr;
  }

} // namespace logger_nm

namespace logger_nm
{
  static const char *level_strings[] = {
      "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

  static const char *level_colors[] = {
      "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m", "\0"};

  /**
 * @brief file_callback: Write logs to the file
 *
 * @param ev log container
 */
  void logger::file_callback(log_Event *ev)
  {
    char buf[64]{0};
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ev->time)] = '\0';
    fprintf((FILE *)ev->udata, "%s [%-5s]: ",
            buf, level_strings[ev->level]);

    vfprintf((FILE *)ev->udata, ev->fmt, ev->ap);
    fprintf((FILE *)ev->udata, "\n");
    fflush((FILE *)ev->udata);
  }

  void logger::lock(void)
  {
    if (lock_var.lock)
    {
      lock_var.lock(true, lock_var.udata);
    }
  }

  void logger::unlock(void)
  {
    if (lock_var.lock)
    {
      lock_var.lock(false, lock_var.udata);
    }
  }

  const char *logger::log_level_string(int level)
  {
    return level_strings[level];
  }

  void logger::log_set_lock(log_LockFn fn, void *udata)
  {
    lock_var.lock = fn;
    lock_var.udata = udata;
  }

  void logger::log_set_level(int level)
  {
    lock_var.level = level;
  }

  void logger::log_set_quiet(bool enable)
  {
    lock_var.quiet = enable;
  }
  /**
 * @brief log_add_callback: add desired callback to the logging system.
 *
 * @param fn callback function
 * @param udata log output destination
 * @param level log level
 * @return int : 0 on success, -1 on failure
 */
  int logger::log_add_callback(log_LogFn fn, void *udata, int level)
  {
    for (int i = 0; i < MAX_CALLBACKS; i++)
    {
      if (!lock_var.callbacks[i].fn)
      {
        lock_var.callbacks[i] = (Callback){fn, udata, level};
        return 0;
      }
    }
    return -1;
  }

  /**
 * @brief log_add_fp: add file handler to the logging system.
 *
 * @param fp[in] file pointer
 * @param level[in] minimum logging level for the file.
 * @return int return file write operation result
 */
  int logger::log_add_fp(FILE *fp, int level)
  {
    return log_add_callback(file_callback, fp, level);
  }

  void logger::log_simple(const int level, const char *file, const int line, const char *msg)
  {
    // char buf[64]{0};
    // time_t time_var = time(NULL);
    // struct tm *ltime = localtime(&time_var);
    // msg_buf = msg;
    // buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ltime)] = '\0';

    // lock();

    // if ((!lock_var.quiet) && (level >= lock_var.level))
    // {

    //   fprintf(stderr, "%s [%s%-5s\x1b[0m]:\x1b[0m ",
    //           buf, level_colors[level], level_strings[level]);
    //   fprintf(stderr, "%s\n", msg);
    //   fflush(stderr);
    // }

    // for (int i = 0; (i < MAX_CALLBACKS) && (lock_var.callbacks[i].fn); i++)
    // {
    //   Callback *cb = &lock_var.callbacks[i];
    //   if (level >= cb->level)
    //   {
    //     fprintf((FILE *)cb->udata, "%s [%-5s]:",
    //             buf, level_strings[level]);
    //     fprintf((FILE *)cb->udata, "%s\n", msg);
    //     fflush((FILE *)cb->udata);
    //   }
    // }

    // unlock();
  }

#if defined(UNIT_TEST) && !defined(EMBEDDED_UNIT_TEST)

  // /**
  //  * @brief init_event: Assign log time to the time variable of ev variables
  //  *
  //  * @param ev log event container.
  //  * @param udata log target (file or stderr)
  //  */
  // void logger::init_event(log_Event *ev, void *udata)
  // {
  //   if (!ev->time)
  //   {
  //     time_t time_var = time(NULL);
  //     ev->time = localtime(&time_var);
  //   }
  //   ev->udata = udata;
  // }
  /**
   * @brief log_console: send syslog to the console (stderr).
   *
   * @param[in] level logging level.
   * @param[in] fmt printf like format.
   * @param[in] ... varidic arguments.
   */
  void logger::log_console(int level, const char *fmt, ...)
  {
    va_list ap;
    char buf[64]{0};
    time_t time_var = time(NULL);
    struct tm *ltime = localtime(&time_var);

    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ltime)] = '\0';

    lock();

    if ((!lock_var.quiet) && (level >= lock_var.level))
    {
      va_start(ap, fmt);

      fprintf(stderr, "%s [%s%-5s\x1b[0m]:\x1b[0m ", buf, level_colors[level], level_strings[level]);
      vfprintf(stderr, fmt, ap);
      fprintf(stderr, "\n");
      fflush(stderr);
      va_end(ap);
    }

    for (int i = 0; (i < MAX_CALLBACKS) && (lock_var.callbacks[i].fn); i++)
    {
      Callback *cb = &lock_var.callbacks[i];
      if (level >= cb->level)
      {
        va_start(ap, fmt);

        fprintf((FILE *)cb->udata, "%s [%-5s]:", buf, level_strings[level]);
        vfprintf((FILE *)cb->udata, fmt, ap);
        fprintf((FILE *)cb->udata, "\n");
        fflush((FILE *)cb->udata);
        va_end(ap);
      }
    }

    unlock();
  }

#else
  /**
   * @brief log_serial: send syslog to the serial port.
   *
   * @param[in] level logging level.
   * @param[in] fmt printf like format.
   * @param[in] ... varidic arguments.
   */
  void logger::log_serial(int level, const char *fmt, ...)
  {
    char buf_raw[256]{0};
    char buf_msg[384]{0};
    // char time_str[32]{0};
    log_Event ev;

    // time_t t = time(NULL);
    // ev.time = localtime(&t);

    // time_str[strftime(time_str, sizeof(time_str), "[%F %H:%M:%S]", ev.time)] = '\0';

    va_start(ev.ap, fmt);
    vsprintf(buf_raw, fmt, ev.ap);

    sprintf(buf_msg, " [%-5s]: %s", level_strings[level], buf_raw);
    // // Javad: Hardware specific UART.
    // Serial.print(time_str);
    Serial.println(buf_msg);

    va_end(ev.ap);
  }
#endif

} // namespace logger_nm
