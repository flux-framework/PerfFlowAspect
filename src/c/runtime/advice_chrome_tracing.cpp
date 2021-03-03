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
                                           const char *module,
                                           const char *function)
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
    cxa = abi::__cxa_demangle (function, 0, 0, &status);
    demangle = (cxa)? cxa : function;
    std::size_t pos = demangle.find_first_of ('(');
    if (pos != std::string::npos)
        demangle = demangle.substr (0, pos);

    if (!(json = json_pack ("{s:s s:s s:I s:I s:f}",
                                "name", demangle.c_str (),
                                "cat", module,
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
                                           const char *scope,
                                           const char *flow_enclose, int64_t id)
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
    if (flow_enclose) {
        json_t *enclose;
        if (!(enclose = json_string ("e")))
            goto json_memerror;
        if (json_object_set_new (o, "bp", enclose) < 0)
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

int advice_chrome_tracing_t::flush_if (size_t size)
{
    int rc = 0;
    try {
        m_oss.seekp (0, std::ios::end);
        std::stringstream::pos_type offset = m_oss.tellp ();
        if (offset >= size) {
            if (!m_ofs.is_open ()) {
                m_ofs.open (m_fn, std::ofstream::out);
                m_ofs << "[" << std::endl;
            }
            m_ofs << m_oss.str ();
            m_oss.str ("");
            m_oss.clear ();
        }
    } catch (std::ostream::failure &e) {
        if (errno == 0)
            errno = EIO;
        return -1;
    }
    return 0;
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
    m_fn = std::string ("perfflow.") + std::string (hn)
           + std::string (".") + std::to_string (getpid ())
           + std::string (".pfw");
    m_before_counter_mutex = PTHREAD_MUTEX_INITIALIZER;
    m_after_counter_mutex = PTHREAD_MUTEX_INITIALIZER;
}

advice_chrome_tracing_t::~advice_chrome_tracing_t ()
{
    flush_if (1);
    if (m_ofs.is_open ())
        m_ofs.close ();
}

int advice_chrome_tracing_t::with_flow (const char *module,
                                        const char *function,
                                        const char *flow,
                                        int64_t id)
{
    int rc;
    char *json_str;
    json_t *event, *ph;

    if ( (rc = create_event (&event, module, function)) < 0)
        return rc;
    if (std::string ("in") == flow) {
        if ( (rc = encode_event (event, "f", nullptr, "e", -1)) < 0) {
            json_decref (event);
            return rc;
        }
    } else if (std::string ("out") == flow) {
        if ( (rc = encode_event (event, "s", nullptr, "e", -1)) < 0) {
            json_decref (event);
            return rc;
        }
    } else {
        json_decref (event);
        errno = EINVAL;
        return -1;
    }
    json_t *id_o;
    if (!(id_o = json_integer (static_cast<json_int_t> (id)))) {
        json_decref (event);
        errno = ENOMEM;
        return -1;
    }
    if (json_object_set_new (event, "id", id_o) < 0) {
        json_decref (event);
        errno = ENOMEM;
        return -1;
    }
    if (!(json_str = json_dumps (event, JSON_INDENT(0)))) {
        json_decref (event);
        errno = ENOMEM;
        return -1;
    }
    m_oss << json_str << "," << std::endl;
    free (json_str);
    json_decref (event);
    return flush_if (FLUSH_SIZE);
}

int advice_chrome_tracing_t::before (const char *module,
                                     const char *function,
                                     const char *flow)
{
    int rc;
    char *json_str;
    json_t *event, *ph;

    if ( (rc = create_event (&event, module, function)) < 0)
        return rc;
    if ( (rc = encode_event (event, "B", nullptr, nullptr, -1)) < 0) {
        json_decref (event);
        return rc;
    }
    if (!(json_str = json_dumps (event, JSON_INDENT(0)))) {
        json_decref (event);
        errno = ENOMEM;
	return -1;
    }
    m_oss << json_str << "," << std::endl;
    free (json_str);
    json_decref (event);
    if ( (rc = flush_if (FLUSH_SIZE)) < 0)
        return rc;
    if (std::string ("NA") != flow) {
	const char *eff_flow = flow;
        if (std::string ("inout") == flow)
            eff_flow = "in";
	else if (std::string ("outin") == flow)
            eff_flow = "out";
        rc = with_flow ("flow", "flow", eff_flow, 100);
    }
    return rc;
}

int advice_chrome_tracing_t::after (const char *module,
                                    const char *function,
                                    const char *flow)
{
    int rc;
    char *json_str;
    json_t *event, *ph;

    if (std::string ("NA") != flow) {
        const char *eff_flow = flow;
        if (std::string ("inout") == flow)
            eff_flow = "out";
        else if (std::string ("outin") == flow)
            eff_flow = "in";
        if ( (rc = with_flow ("flow", "flow", eff_flow, 100)) < 0)
            return rc;
    }
    if ( (rc = create_event (&event, module, function)) < 0)
        return rc;
    if ( (rc = encode_event (event, "E", nullptr, nullptr, -1)) < 0) {
        json_decref (event);
        return rc;
    }
    if (!(json_str = json_dumps (event, JSON_INDENT(0)))) {
        json_decref (event);
        errno = ENOMEM;
        return -1;
    }
    m_oss << json_str << "," << std::endl;
    free (json_str);
    json_decref (event);
    return flush_if (FLUSH_SIZE);
}

int advice_chrome_tracing_t::before_async (const char *module,
                                           const char *function,
                                           const char *scope,
                                           const char *flow)
{
    int rc;
    char *json_str;
    json_t *event, *ph, *sc;

    if ( (rc = create_event (&event, module, function)) < 0)
        return rc;
    if ( (rc = pthread_mutex_lock (&m_before_counter_mutex)) < 0) {
        json_decref (event);
        return rc;
    }
    if ( (rc = encode_event (event, "b", scope, nullptr,
                             m_before_counter)) < 0) {
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
    m_oss << json_str << "," << std::endl;
    free (json_str);
    json_decref (event);
    if ( (rc = flush_if (FLUSH_SIZE)) < 0)
        return rc;
    if (std::string ("NA") != flow) {
	const char *eff_flow = flow;
        if (std::string ("inout") == flow)
            eff_flow = "in";
        else if (std::string ("outin") == flow)
            eff_flow = "out";
        rc = with_flow ("flow", "flow", flow, 100);
    }
    return rc;
}

int advice_chrome_tracing_t::after_async (const char *module,
                                          const char *function,
                                          const char *scope,
                                          const char *flow)
{
    int rc;
    char *json_str;
    json_t *event, *ph, *sc;

    if (std::string ("NA") != flow) {
        const char *eff_flow = flow;
        if (std::string ("inout") == flow)
            eff_flow = "out";
        else if (std::string ("outin") == flow)
            eff_flow = "in";
        if ( (rc = with_flow ("flow", "flow", eff_flow, 100)) < 0)
            return rc;
    }
    if ( (rc = create_event (&event, module, function)) < 0)
        return rc;
    if ( (rc = pthread_mutex_lock (&m_after_counter_mutex)) < 0) {
        json_decref (event);
        return rc;
    }
    if ( (rc = encode_event (event, "e", scope,
                             nullptr, m_after_counter)) < 0) {
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
    m_oss << json_str << "," << std::endl;
    free (json_str);
    json_decref (event);
    return flush_if (FLUSH_SIZE);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
