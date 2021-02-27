/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#include <string>
#include <fstream>
#include <system_error>
#include <unistd.h>
#include <cxxabi.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/syscall.h>
// TODO: only works for Linux
#define my_gettid() ((pid_t)syscall(SYS_gettid))
#include "advice_chrome_tracing.hpp"


/******************************************************************************
 *                                                                            *
 *           Private Methods of advice_chrome_tracing_t Class                 *
 *                                                                            *
 ******************************************************************************/

int advice_chrome_tracing_t::get_timestamp (double &tstamp)
{
    int rc;
    struct timeval ts;
    if ( (rc = gettimeofday (&ts, NULL)) < 0)
        return rc;
    tstamp = (double)ts.tv_sec*1000000.f + (double)ts.tv_usec;
    return 0;
}

int advice_chrome_tracing_t::create_event (json_t **o,
                                           const char *mn, const char *fn)
{
    int rc, status;
    char *cxa;
    double ts;
    json_t *json;
    pid_t pid, tid;
    std::string demangle;

    if ( (rc = get_timestamp (ts)) < 0)
        return rc;

    pid = getpid ();
    tid = my_gettid ();
    cxa = abi::__cxa_demangle (fn, 0, 0, &status);
    demangle = (cxa)? cxa : fn;
    std::size_t pos = demangle.find_first_of ('(');
    if (pos != std::string::npos)
        demangle = demangle.substr (0, pos);

    if (!(json = json_pack ("{s:s s:s s:I s:I s:f}",
                                "name", demangle.c_str (),
                                "cat", mn,
                                "pid", static_cast<int64_t> (pid),
                                "tid", static_cast<int64_t> (tid),
                                "ts", ts))) {
        free (cxa);
        errno = ENOMEM;
        return -1;
    }
    *o = json;
    free (cxa);
    return 0;
}

int advice_chrome_tracing_t::encode_event (json_t *o, const char *ph,
                                           const char *scope, int64_t id)
{
    if (!o || !ph) {
        errno = EINVAL;
        return -1;
    }

    json_t *ph_o;
    if (!(ph_o = json_string (ph)))
        goto json_memerror;
    if (json_object_set_new (o, "ph", ph_o) < 0)
        goto json_memerror;
    if (scope) {
        json_t *sc;
        if (!(sc = json_string (scope)))
            goto json_memerror;
        if (json_object_set_new (o, "scope", sc) < 0)
            goto json_memerror;
    }
    if (id >= 0) {
        json_t *id_o;
        if (!(id_o = json_integer (static_cast<json_int_t> (id))))
            goto json_memerror;
        if (json_object_set_new (o, "id", id_o) < 0)
            goto json_memerror;
    }
    return 0;

json_memerror:
    errno = ENOMEM;
    return -1;
}


/******************************************************************************
 *                                                                            *
 *             Public Methods of advice_chrome_tracing_t Class                *
 *                                                                            *
 ******************************************************************************/

// Can throw std:::system_error or std::ofstream::failure exceptions
advice_chrome_tracing_t::advice_chrome_tracing_t ()
{
    // TODO: add support for PERFLOW_OPTIONS
    // TODO: especially PERFLOW_OPTIONS="log_file=my_name.log"
    // TODO: support for TOML config
    char hn[128];
    if (gethostname (hn, 128) < 0)
        throw std::system_error (errno,
                                 std::system_category (),
                                 "gethostname failed");
    std::string fn = std::string ("perfflow.out.")
                         + hn + std::string (".")
                         + std::to_string (getpid ());
    pid_t tid = my_gettid ();
    m_before_counter_mutex = PTHREAD_MUTEX_INITIALIZER;
    m_after_counter_mutex = PTHREAD_MUTEX_INITIALIZER;
    m_ofs.open (fn, std::ofstream::out);
    m_ofs << "[" << std::endl;
}

advice_chrome_tracing_t::~advice_chrome_tracing_t ()
{
}

int advice_chrome_tracing_t::before (const char *mn, const char *fn)
{
    int rc;
    char *json_str;
    json_t *event, *ph;

    if ( (rc = create_event (&event, mn, fn)) < 0)
        return rc;
    if ( (rc = encode_event (event, "B", nullptr, -1)) < 0) {
        json_decref (event);
        return rc;
    }
    if (!(json_str = json_dumps (event, JSON_INDENT(0)))) {
        json_decref (event);
        errno = ENOMEM;
	return -1;
    }
    m_ofs << json_str << "," << std::endl;
    free (json_str);
    json_decref (event);
    return 0;
}

int advice_chrome_tracing_t::after (const char *mn, const char *fn)
{
    int rc;
    char *json_str;
    json_t *event, *ph;

    if ( (rc = create_event (&event, mn, fn)) < 0)
        return rc;
    if ( (rc = encode_event (event, "E", nullptr, -1)) < 0) {
        json_decref (event);
        return rc;
    }
    if (!(json_str = json_dumps (event, JSON_INDENT(0)))) {
        json_decref (event);
        errno = ENOMEM;
        return -1;
    }
    m_ofs << json_str << "," << std::endl;
    free (json_str);
    json_decref (event);
    return 0;
}

int advice_chrome_tracing_t::before_async (const char *mn,
                                           const char *fn, const char *scope)
{
    int rc;
    char *json_str;
    json_t *event, *ph, *sc;

    if ( (rc = create_event (&event, mn, fn)) < 0)
        return rc;
    if ( (rc = pthread_mutex_lock (&m_before_counter_mutex)) < 0) {
        json_decref (event);
        return rc;
    }
    if ( (rc = encode_event (event, "b", scope, m_before_counter)) < 0) {
        json_decref (event);
        return rc;
    }
    m_before_counter++;
    if ( (rc = pthread_mutex_unlock (&m_before_counter_mutex)) < 0) {
        json_decref (event);
        return rc;
    }
    if (!(json_str = json_dumps (event, JSON_INDENT(0)))) {
        json_decref (event);
        errno = ENOMEM;
        return -1;
    }
    m_ofs << json_str << "," << std::endl;
    free (json_str);
    json_decref (event);
    return 0;
}

int advice_chrome_tracing_t::after_async (const char *mn,
                                          const char *fn, const char *scope)
{
    int rc;
    char *json_str;
    json_t *event, *ph, *sc;

    if ( (rc = create_event (&event, mn, fn)) < 0)
        return rc;
    if ( (rc = pthread_mutex_lock (&m_after_counter_mutex)) < 0) {
        json_decref (event);
        return rc;
    }
    if ( (rc = encode_event (event, "e", scope, m_after_counter)) < 0) {
        json_decref (event);
        return rc;
    }
    m_after_counter++;
    if ( (rc = pthread_mutex_unlock (&m_before_counter_mutex)) < 0) {
        json_decref (event);
        return rc;
    }
    if (!(json_str = json_dumps (event, JSON_INDENT(0)))) {
        json_decref (event);
        errno = ENOMEM;
        return -1;
    }
    m_ofs << json_str << "," << std::endl;
    free (json_str);
    json_decref (event);
    return 0;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
