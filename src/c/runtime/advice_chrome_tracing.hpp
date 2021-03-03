/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#include <jansson.h>
#include <pthread.h>
#include <fstream>
#include <sstream>
#include "advice_base.hpp"

class advice_chrome_tracing_t : public advice_base_t
{
public:
    advice_chrome_tracing_t ();
    virtual ~advice_chrome_tracing_t ();
    virtual int before (const char *module,
                        const char *function, const char *flow);
    virtual int after (const char *module,
                       const char *function, const char *flow);
    virtual int before_async (const char *module,
                              const char *function,
                              const char *scope, const char *flow);
    virtual int after_async (const char *module,
                             const char *function,
                             const char *scope,  const char *flow);
private:
    int create_event (json_t **o, const char *module, const char *function);
    int get_timestamp (double &tstamp);
    int encode_event (json_t *o, const char *ph,
                      const char *scope, const char *enclose, int64_t id);
    int with_flow (const char *module, const char *function,
                   const char *flow, int64_t id);
    int flush_if (size_t size);

    const int FLUSH_SIZE = 16777216;
    std::ostringstream m_oss;
    std::ofstream m_ofs;
    std::string m_fn = "";
    int m_before_counter = 0;
    int m_after_counter = 0;
    pthread_mutex_t m_before_counter_mutex;
    pthread_mutex_t m_after_counter_mutex;
};

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
