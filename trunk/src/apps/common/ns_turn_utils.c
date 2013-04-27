/*
 * Copyright (C) 2011, 2012 Citrix Systems
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "ns_turn_utils.h"
#include "ns_turn_ioalib.h"

#include <time.h>

#if !defined(TURN_NO_THREADS)
#include <pthread.h>
#endif

#include <stdlib.h>
#include <stdio.h>

////////// MUTEXES /////////////

#define MAGIC_CODE (0xEFCD1983)

int turn_mutex_lock(const turn_mutex *mutex) {
  if(mutex && mutex->mutex && (mutex->data == MAGIC_CODE)) {
    int ret = 0;
#if !defined(TURN_NO_THREADS)
    ret = pthread_mutex_lock((pthread_mutex_t*)mutex->mutex);
#endif
    if(ret<0) {
      perror("Mutex lock");
    }
    return ret;
  } else {
    printf("Uninitialized mutex\n");
    return -1;
  }
}

int turn_mutex_unlock(const turn_mutex *mutex) {
  if(mutex && mutex->mutex && (mutex->data == MAGIC_CODE)) {
    int ret = 0;
#if !defined(TURN_NO_THREADS)
    ret = pthread_mutex_unlock((pthread_mutex_t*)mutex->mutex);
#endif
    if(ret<0) {
      perror("Mutex unlock");
    }
    return ret;
  } else {
    printf("Uninitialized mutex\n");
    return -1;
  }
}

int turn_mutex_init(turn_mutex* mutex) {
  if(mutex) {
    mutex->mutex=malloc(sizeof(pthread_mutex_t));
    mutex->data=MAGIC_CODE;
    int ret = 0;
#if !defined(TURN_NO_THREADS)
    pthread_mutex_init((pthread_mutex_t*)mutex->mutex,NULL);
#endif
    if(ret<0) {
      perror("Mutex init");
      mutex->data=0;
      free(mutex->mutex);
      mutex->mutex=NULL;
    }
    return ret;
  } else {
    return -1;
  }
}

int turn_mutex_init_recursive(turn_mutex* mutex) {
  int ret=-1;
  if(mutex) {
#if !defined(TURN_NO_THREADS)
    pthread_mutexattr_t attr;
    if(pthread_mutexattr_init(&attr)<0) {
      perror("Cannot init mutex attr");
    } else {
      if(pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE)<0) {
	perror("Cannot set type on mutex attr");
      } else {
	mutex->mutex=malloc(sizeof(pthread_mutex_t));
	mutex->data=MAGIC_CODE;
	if((ret=pthread_mutex_init((pthread_mutex_t*)mutex->mutex,&attr))<0) {
	  perror("Cannot init mutex");
	  mutex->data=0;
	  free(mutex->mutex);
	  mutex->mutex=NULL;
	}
      }
      pthread_mutexattr_destroy(&attr);
    }
#else
    mutex->mutex=malloc(1);
    mutex->data=MAGIC_CODE;
    ret = 0;
#endif
  }
  return ret;
}

int turn_mutex_destroy(turn_mutex* mutex) {
  if(mutex && mutex->mutex && mutex->data == MAGIC_CODE) {
    int ret = 0;
#if !defined(TURN_NO_THREADS)
    ret = pthread_mutex_destroy((pthread_mutex_t*)(mutex->mutex));
#endif
    free(mutex->mutex);
    mutex->mutex=NULL;
    mutex->data=0;
    return ret;
  } else {
    return 0;
  }
}

///////////////////////// LOG ///////////////////////////////////

#if defined(TURN_LOG_FUNC_IMPL)
extern void TURN_LOG_FUNC_IMPL(TURN_LOG_LEVEL level, const s08bits* format, va_list args);
#endif

static int no_stdout_log = 0;

void set_no_stdout_log(int val)
{
	no_stdout_log = val;
}

void turn_log_func_default(TURN_LOG_LEVEL level, const s08bits* format, ...)
{
#if !defined(TURN_LOG_FUNC_IMPL)
	{
		va_list args;
		va_start(args,format);
		vrtpprintf(format, args);
		va_end(args);
	}
#endif

	{
		va_list args;
		va_start(args,format);
#if defined(TURN_LOG_FUNC_IMPL)
		TURN_LOG_FUNC_IMPL(level,format,args);
#else
		if (level == TURN_LOG_LEVEL_ERROR) {
			printf("%lu: ERROR: ",(unsigned long)turn_time());
			vprintf(format, args);
		} else if(!no_stdout_log) {
			printf("%lu: ",(unsigned long)turn_time());
			vprintf(format, args);
		}
#endif
		va_end(args);
	}
}

void addr_debug_print(int verbose, const ioa_addr *addr, const s08bits* s)
{
	if (verbose) {
		if (!addr) {
			TURN_LOG_FUNC(TURN_LOG_LEVEL_INFO, "%s: EMPTY\n", s);
		} else {
			s08bits addrbuf[INET6_ADDRSTRLEN];
			if (!s)
				s = "";
			if (addr->ss.ss_family == AF_INET) {
				TURN_LOG_FUNC(TURN_LOG_LEVEL_INFO, "IPv4. %s: %s:%d\n", s, inet_ntop(AF_INET,
								&addr->s4.sin_addr, addrbuf, INET6_ADDRSTRLEN),
								nswap16(addr->s4.sin_port));
			} else if (addr->ss.ss_family == AF_INET6) {
				TURN_LOG_FUNC(TURN_LOG_LEVEL_INFO, "IPv6. %s: %s:%d\n", s, inet_ntop(AF_INET6,
								&addr->s6.sin6_addr, addrbuf, INET6_ADDRSTRLEN),
								nswap16(addr->s6.sin6_port));
			} else {
				if (addr_any_no_port(addr)) {
					TURN_LOG_FUNC(
									TURN_LOG_LEVEL_INFO,
									"IP. %s: 0.0.0.0:%d\n",
									s,
									nswap16(addr->s4.sin_port));
				} else {
					TURN_LOG_FUNC(TURN_LOG_LEVEL_INFO, "%s: wrong IP address family: %d\n", s,
									(int) (addr->ss.ss_family));
				}
			}
		}
	}
}

/******* Log ************/

static FILE* _rtpfile = NULL;
static char log_fn[1025]="\0";
static char log_fn_base[1025]="\0";

static turn_mutex log_mutex;
static int log_mutex_inited = 0;

static void log_lock(void) {
	if(!log_mutex_inited) {
		log_mutex_inited=1;
		turn_mutex_init_recursive(&log_mutex);
	}
	turn_mutex_lock(&log_mutex);
}

static void log_unlock(void) {
	turn_mutex_unlock(&log_mutex);
}

static void get_date(char *s, size_t sz) {
	time_t curtm;
    struct tm* tm_info;

    curtm = time(NULL);
    tm_info = localtime(&curtm);

    strftime(s, sz, "%M", tm_info);
}

void set_logfile(const char *fn)
{
	if(fn) {
		log_lock();
		if(strcmp(fn,log_fn_base)) {
			reset_rtpprintf();
			STRCPY(log_fn_base,fn);
		}
		log_unlock();
	}
}

void reset_rtpprintf(void)
{
	log_lock();
	if(_rtpfile) {
		if(_rtpfile != stdout)
			fclose(_rtpfile);
		_rtpfile = NULL;
	}
	log_unlock();
}

static void set_log_file_name(char *base, char *f)
{
	char logdate[125];

	get_date(logdate,sizeof(logdate));

	sprintf(f, "%s%s.log", base,logdate);
}

static void set_rtpfile(void)
{
	if (!_rtpfile) {
		if(log_fn_base[0]) {
			if(!strcmp(log_fn_base,"stdout")|| !strcmp(log_fn_base,"-")) {
				_rtpfile = stdout;
				no_stdout_log = 1;
			} else {
				set_log_file_name(log_fn_base,log_fn);
				_rtpfile = fopen(log_fn, "w");
			}
			if (!_rtpfile) {
				fprintf(stderr,"ERROR: Cannot open log file for writing: %s\n",log_fn);
			} else {
				return;
			}
		}
	}

	if(!_rtpfile) {
		char logbase[1025];
		char logtail[125];
		char logf[1025];
		sprintf(logtail, "turn_%d_", (int)getpid());

		sprintf(logbase, "/var/log/%s", logtail);
		set_log_file_name(logbase,logf);
		_rtpfile = fopen(logf, "w");
		if(!_rtpfile) {
			sprintf(logbase, "/var/tmp/%s", logtail);
			set_log_file_name(logbase,logf);
			_rtpfile = fopen(logf, "w");
			if(!_rtpfile) {
				sprintf(logbase, "/tmp/%s", logtail);
				set_log_file_name(logbase,logf);
				_rtpfile = fopen(logf, "w");
				if(!_rtpfile) {
					sprintf(logbase, "%s", logtail);
					set_log_file_name(logbase,logf);
					_rtpfile = fopen(logf, "w");
					if(!_rtpfile) {
						_rtpfile = stdout;
						return;
					}
				}
			}
		}

		STRCPY(log_fn_base,logbase);
		STRCPY(log_fn,logf);
	}
}

#define Q(x) #x
#define QUOTE(x) Q(x)

void rollover_logfile(void)
{
	log_lock();
	if(_rtpfile && log_fn[0] && (_rtpfile != stdout)) {
		char logf[1025];

		set_log_file_name(log_fn_base,logf);
		if(strcmp(log_fn,logf)) {
			fclose(_rtpfile);
			log_fn[0]=0;
			_rtpfile = fopen(logf, "w");
			if(!_rtpfile) {
				_rtpfile = stdout;
			} else {
				STRCPY(log_fn,logf);
			}
		}
	}
	log_unlock();
}

void rtpprintf(const char *format, ...)
{
	log_lock();
	set_rtpfile();
	fprintf(_rtpfile,"%lu: ",(unsigned long)turn_time());
	va_list args;
	va_start (args, format);
	vfprintf(_rtpfile,format, args);
	fflush(_rtpfile);
	va_end (args);
	log_unlock();
}

int vrtpprintf(const char *format, va_list args)
{
	log_lock();
	set_rtpfile();
	fprintf(_rtpfile,"%lu: ",(unsigned long)turn_time());
	vfprintf(_rtpfile,format, args);
	fflush(_rtpfile);
	log_unlock();
	return 0;
}

//////////////////////////////////////////////////////////////////
