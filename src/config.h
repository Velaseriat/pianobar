#pragma once

/* package name */
#define PACKAGE "pianobar"

#define VERSION "2024.12.21-dev"

/* glibc feature test macros, define _before_ including other files */
#define _POSIX_C_SOURCE 200809L

#if defined(_WIN32)
#define BAR_WINDOWS 1
#endif

#if defined(BAR_WINDOWS)
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define popen _popen
#define pclose _pclose
#define setenv(name, value, overwrite) _putenv_s ((name), (value))
#endif

/* ffmpeg/libav quirks detection
 * ffmpeg’s micro versions always start at 100, that’s how we can distinguish
 * ffmpeg and libav */
#include <libavfilter/version.h>
#include <libavformat/version.h>

/* does graph_send_command exist (ffmpeg >=2.2) */
#if !defined(HAVE_AVFILTER_GRAPH_SEND_COMMAND) && \
		LIBAVFILTER_VERSION_MAJOR >= 4 && \
		LIBAVFILTER_VERSION_MICRO >= 100
#define HAVE_AVFILTER_GRAPH_SEND_COMMAND
#endif

/* explicit init is optional for ffmpeg>=4.0 */
#if !defined(HAVE_AVFORMAT_NETWORK_INIT) && \
		LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 5, 100) && \
		LIBAVFORMAT_VERSION_MICRO >= 100
#define HAVE_AVFORMAT_NETWORK_INIT
#endif

/* dito */
#if !defined(HAVE_AVFILTER_REGISTER_ALL) && \
		LIBAVFILTER_VERSION_INT < AV_VERSION_INT(7, 14, 100) && \
		LIBAVFILTER_VERSION_MICRO >= 100
#define HAVE_AVFILTER_REGISTER_ALL
#endif

/* dito */
#if !defined(HAVE_AV_REGISTER_ALL) && \
		LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 9, 100) && \
		LIBAVFORMAT_VERSION_MICRO >= 100
#define HAVE_AV_REGISTER_ALL
#endif

#ifndef NDEBUG
#define HAVE_DEBUGLOG
#define debug(...) fprintf(stderr, __VA_ARGS__)
#else
#define debug(...)
#endif
