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
#include "advice_base.hpp"

class advice_chrome_tracing_t : public advice_base_t
{
public:
    advice_chrome_tracing_t ();
    ~advice_chrome_tracing_t ();
    virtual int before (const char *mn, const char *fn);
    virtual int after (const char *mn, const char *fn);
    virtual int before_async (const char *mn, const char *fn,
                              const char *scope);
    virtual int after_async (const char *mn, const char *fn,
                             const char *scope);
private:
    int create_event (json_t **o, const char *mn, const char *fn);
    int get_timestamp (double &tstamp);
    int encode_event (json_t *o, const char *ph,
                      const char *scope, int64_t id);

    std::ofstream m_ofs;
    int m_before_counter = 0;
    int m_after_counter = 0;
    pthread_mutex_t m_before_counter_mutex;
    pthread_mutex_t m_after_counter_mutex;
};

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
