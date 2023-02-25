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
#include <map>
#include <string>
#include "advice_base.hpp"

class advice_chrome_tracing_t : public advice_base_t
{
public:
    advice_chrome_tracing_t ();
    virtual ~advice_chrome_tracing_t ();
    virtual int before (const char *module,
                        const char *function, const char *flow, const char *pcut);
    virtual int after (const char *module,
                       const char *function, const char *flow, const char *pcut);
    virtual int before_async (const char *module,
                              const char *function,
                              const char *scope, const char *flow);
    virtual int after_async (const char *module,
                             const char *function,
                             const char *scope,  const char *flow);
private:
    int create_event (json_t **o, const char *module, const char *function, double &my_ts);
    int get_timestamp (double &tstamp);
    int encode_event (json_t *o, const char *ph,
                      const char *scope, const char *enclose, int64_t id, double cpu_usage, long mem_usage);
    int with_flow (const char *module, const char *function,
                   const char *flow, int64_t id);
    int flush_if (size_t size);
    int write_to_sstream (const char *str);
    double get_wall_time();
    double get_cpu_time();
    long get_memory_usage();

    int cannonicalize_perfflow_options ();
    int parse_perfflow_options ();
    const std::string get_foreign_wm ();
    const std::string get_uniq_id_from_foreign_wm ();
    const std::string get_perfflow_instance_path ();
    int set_perfflow_instance_path (const std::string &path);

    const int FLUSH_SIZE = 16777216;
    std::ostringstream m_oss;
    std::ofstream m_ofs;
    std::string m_fn = "";
    int m_enable_logging = 1; /* Default should be true */
    int m_before_counter = 0;
    int m_after_counter = 0;
    int m_cpu_mem_usage_enable = 0;
    pthread_mutex_t m_before_counter_mutex;
    pthread_mutex_t m_after_counter_mutex;
    pthread_mutex_t m_mutex;
    std::map<std::string, std::string> m_perfflow_options;
};

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
