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
#include <vector>
#include <system_error>
#include <iomanip>
#include <cstdlib>
#include <unistd.h>
#include <cxxabi.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <openssl/sha.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/resource.h>

// TODO: only works for Linux
#define my_gettid() ((pid_t)syscall(SYS_gettid))
#include "advice_chrome_tracing.hpp"


/******************************************************************************
 *                                                                            *
 *           Private Methods of advice_chrome_tracing_t Class                 *
 *                                                                            *
 ******************************************************************************/

int advice_chrome_tracing_t::get_timestamp(double &tstamp)
{
    int rc;
    struct timeval ts;
    if ((rc = gettimeofday(&ts, NULL)) < 0)
    {
        return rc;
    }
    tstamp = (double)ts.tv_sec * 1000000.f + (double)ts.tv_usec;
    return 0;
}

int advice_chrome_tracing_t::create_event(json_t **o,
        const char *module,
        const char *function,
        double &ts)
{
    int rc, status;
    char *cxa;
    json_t *json;
    pid_t pid, tid;
    std::string demangle;

    if (ts == 0)
    {
        if ((rc = get_timestamp(ts)) < 0)
        {
            return rc;
        }
    }

    pid = getpid();
    tid = my_gettid();
    cxa = abi::__cxa_demangle(function, 0, 0, &status);
    demangle = (cxa) ? cxa : function;
    std::size_t pos = demangle.find_first_of('(');
    if (pos != std::string::npos)
    {
        demangle = demangle.substr(0, pos);
    }

    if (!(json = json_pack("{s:s s:s s:I s:I s:f}",
                           "name", demangle.c_str(),
                           "cat", module,
                           "pid", static_cast<int64_t>(pid),
                           "tid", static_cast<int64_t>(tid),
                           "ts", ts)))
    {
        free(cxa);
        errno = ENOMEM;
        return -1;
    }
    *o = json;
    free(cxa);
    return 0;
}

int advice_chrome_tracing_t::encode_event(json_t *o, const char *ph,
        const char *scope,
        const char *flow_enclose, int64_t id,
        double cpu_usage, long mem_usage)
{
    char *temp;
    json_t *args_o = json_object();
    if (!o || !ph)
    {
        errno = EINVAL;
        return -1;
    }

    json_t *ph_o;
    if (!(ph_o = json_string(ph)))
    {
        goto json_memerror;
    }
    if (json_object_set_new(o, "ph", ph_o) < 0)
    {
        goto json_memerror;
    }
    if (scope)
    {
        json_t *sc;
        if (!(sc = json_string(scope)))
        {
            goto json_memerror;
        }
        if (json_object_set_new(o, "scope", sc) < 0)
        {
            goto json_memerror;
        }
    }
    if (flow_enclose)
    {
        json_t *enclose;
        if (!(enclose = json_string("e")))
        {
            goto json_memerror;
        }
        if (json_object_set_new(o, "bp", enclose) < 0)
        {
            goto json_memerror;
        }
    }
    if (id >= 0)
    {
        json_t *id_o;
        if (!(id_o = json_integer(static_cast<json_int_t>(id))))
        {
            goto json_memerror;
        }
        if (json_object_set_new(o, "id", id_o) < 0)
        {
            goto json_memerror;
        }
    }
    if (std::string("C") == ph)
    {
        json_t *cpu_usage_o, *mem_usage_o;
        if (!(cpu_usage_o = json_real(cpu_usage)))
        {
            goto json_memerror;
        }
        if (json_object_set_new(args_o, "cpu_usage", cpu_usage_o) < 0)
        {
            goto json_memerror;
        }
        if (!(mem_usage_o = json_integer(static_cast<json_int_t>(mem_usage))))
        {
            goto json_memerror;
        }
        if (json_object_set_new(args_o, "memory_usage", mem_usage_o) < 0)
        {
            goto json_memerror;
        }
        if (json_object_set_new(o, "args", args_o) < 0)
        {
            goto json_memerror;
        }
    }
    return 0;

json_memerror:
    errno = ENOMEM;
    return -1;
}

int advice_chrome_tracing_t::flush_if(size_t size)
{
    int rc = 0;
    try
    {
        if ((rc = pthread_mutex_lock(&m_mutex)) < 0)
        {
            return rc;
        }
        m_oss.seekp(0, std::ios::end);
        std::stringstream::pos_type offset = m_oss.tellp();
        if (offset >= size)
        {
            if (!m_ofs.is_open())
            {
                m_ofs.open(m_fn, std::ofstream::out);
                m_ofs << "[" << std::endl;
            }
            m_ofs << m_oss.str();
            m_oss.str("");
            m_oss.clear();
        }
        if ((rc = pthread_mutex_unlock(&m_mutex)) < 0)
        {
            return rc;
        }
    }
    catch (std::ostream::failure &e)
    {
        if (errno == 0)
        {
            errno = EIO;
        }
        return -1;
    }
    return 0;
}

int advice_chrome_tracing_t::cannonicalize_perfflow_options()
{
    if (m_perfflow_options.find("name") == m_perfflow_options.end())
    {
        m_perfflow_options["name"] = "generic";
    }
    if (m_perfflow_options.find("log-filename-include")
        == m_perfflow_options.end())
    {
        m_perfflow_options["log-filename-include"] = "hostname,pid";
    }
    if (m_perfflow_options.find("log-dir") == m_perfflow_options.end())
    {
        m_perfflow_options["log-dir"] = "./";
    }
    if (m_perfflow_options.find("log-enable") == m_perfflow_options.end())
    {
        m_perfflow_options["log-enable"] = "True";
    }
    if (m_perfflow_options.find("cpu_mem_usage") == m_perfflow_options.end())
    {
        m_perfflow_options["cpu_mem_usage"] = "False";
    }
    return 0;
}

int advice_chrome_tracing_t::parse_perfflow_options()
{
    int rc = 0;
    std::vector<std::string> options_list;
    char *options = getenv("PERFFLOW_OPTIONS");
    if (options)
    {
        std::stringstream ss;
        std::string entry;
        ss << options;
        while (getline(ss, entry, ':'))
        {
            options_list.push_back(entry);
        }
    }
    for (auto &opt : options_list)
    {
        size_t found = opt.find_first_of("=");
        if (found != std::string::npos)
        {
            std::string k = opt.substr(0, found);
            std::string v = opt.substr(found + 1);
            m_perfflow_options[k] = v;
        }
        else
        {
            errno = EINVAL;
            rc -= 1;
        }
    }
    return (cannonicalize_perfflow_options() + rc);
}

const std::string advice_chrome_tracing_t::get_foreign_wm()
{
    char *foreign_job_id = getenv("SLURM_JOB_ID");
    if (foreign_job_id)
    {
        return "slurm";
    }
    foreign_job_id = getenv("LSB_JOBID");
    if (foreign_job_id)
    {
        return "lsf";
    }
    return "";
}

const std::string advice_chrome_tracing_t::get_uniq_id_from_foreign_wm()
{
    std::string uniq_id = "";
    const std::string foreign_wm = get_foreign_wm();
    if (foreign_wm == "slurm")
    {
        char *job_id = getenv("SLURM_JOB_ID");
        char *step_id = getenv("SLURM_STEP_ID");
        if (job_id && step_id)
        {
            uniq_id = std::string(job_id) + "." + std::string(step_id);
        }
    }
    else if (foreign_wm == "lsf")
    {
        char *job_id = getenv("LSB_JOBID");
        char *step_id = getenv("LS_JOBPID");
        if (job_id && step_id)
        {
            uniq_id = std::string(job_id) + "." + std::string(step_id);
        }
    }
    return uniq_id;
}

const std::string advice_chrome_tracing_t::get_perfflow_instance_path()
{
    int i;
    std::stringstream ss;
    std::string instance_id = "";
    std::string instance_path = "";
    char *instance_path_c_str = nullptr;
    char *flux_job_id = getenv("FLUX_JOB_ID");
    if (!flux_job_id)
    {
        const std::string foreign_id = get_uniq_id_from_foreign_wm();
        if (foreign_id != "")
        {
            unsigned char sha1[SHA_DIGEST_LENGTH];
            unsigned char *data = reinterpret_cast<unsigned char *>(
                                      const_cast<char *>(foreign_id.c_str()));
            SHA1(data, foreign_id.length(), sha1);
            for (i = 0; i < 4; i++)
            {
                ss << std::setw(2) << std::setfill('0')
                   << std::hex << static_cast<int>(sha1[i]);
            }
            instance_id = ss.str();
        }
        else
        {
            std::string uniq_id = "1";
            char *uri = getenv("FLUX_URI");
            if (uri)
            {
                std::string uri_copy = uri;
                size_t found = uri_copy.find_first_of(":");
                if (found != std::string::npos
                    && (found + 2) < uri_copy.length())
                {
                    std::string uri_path = uri_copy.substr(found + 3);
                    found = uri_path.find_last_of("/");
                    if (found != std::string::npos)
                    {
                        uniq_id = uri_path.substr(0, found);
                    }
                }
            }
            unsigned char sha1[SHA_DIGEST_LENGTH];
            unsigned char *data = reinterpret_cast<unsigned char *>(
                                      const_cast<char *>(uniq_id.c_str()));
            SHA1(data, uniq_id.length(), sha1);
            for (i = 0; i < 4; i++)
            {
                ss << std::setw(2) << std::setfill('0')
                   << std::hex << static_cast<int>(sha1[i]);
            }
            instance_id = ss.str();
        }
    }
    else
    {
        instance_id = flux_job_id;
    }

    instance_path_c_str = getenv("PERFFLOW_INSTANCE_PATH");
    if (!instance_path_c_str)
    {
        instance_path = instance_id;
    }
    else
    {
        instance_path = std::string(instance_path_c_str) + "." + instance_id;
    }

    return instance_path;
}

int advice_chrome_tracing_t::set_perfflow_instance_path(
    const std::string &path)
{
    return setenv("PERFFLOW_INSTANCE_PATH", path.c_str(), 1);
}


/******************************************************************************
 *                                                                            *
 *             Public Methods of advice_chrome_tracing_t Class                *
 *                                                                            *
 ******************************************************************************/

// Can throw std:::system_error or std::ofstream::failure exceptions
advice_chrome_tracing_t::advice_chrome_tracing_t ()
{
    // PERFFLOW_OPTIONS envVar
    // To specify the name of this workflow component (default: name=generic)
    //     PERFFLOW_OPTIONS="name=foo"
    // To constomize output filename (default: log-filename-include=hostname,pid)
    //     PERFFLOW_OPTIONS="log-filename-include=<metadata1, metadata2, ...>
    //         Supported metadata:
    //             name: workflow component name
    //             instance-path: hierarchical component path
    //             hostname: name of the host where this process is running
    //             pid: process id
    // To change the directory in which the log file is created
    //     PERFFLOW_OPTIONS="log-dir=DIR"
    // To disable logging (default: log-enable=True)
    //     PERFFLOW_OPTIONS="log-enable=False"
    // You can combine the options in colon (:) delimited format

    if (parse_perfflow_options() < 0)
        throw std::system_error(errno, std::system_category(),
                                "parse_perfflow_options");

    std::string inst_path = get_perfflow_instance_path();
    if (set_perfflow_instance_path(inst_path))
        throw std::system_error(errno, std::system_category(),
                                "set_perfflow_instance_path");

    std::string include = m_perfflow_options["log-filename-include"];
    std::vector<std::string> include_list;
    std::stringstream ss;
    std::string entry;

    ss << include;
    while (getline(ss, entry, ','))
    {
        include_list.push_back(entry);
    }

    char hn[128];
    if (gethostname(hn, 128) < 0)
        throw std::system_error(errno,
                                std::system_category(),
                                "gethostname failed");

    m_fn = "perfflow";
    for (auto &inc : include_list)
    {
        if (inc == "name")
        {
            m_fn += "." + m_perfflow_options["name"];
        }
        else if (inc == "instance-path")
        {
            m_fn += ".{" + inst_path + "}";
        }
        else if (inc == "hostname")
        {
            m_fn += "." + std::string(hn);
        }
        else if (inc == "pid")
        {
            m_fn += "." + std::to_string(getpid());
        }
    }

    m_fn += ".pfw";

    std::string log_dir = m_perfflow_options["log-dir"];
    if (mkdir(log_dir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) < 0
        && errno != EEXIST)
        throw std::system_error(errno,
                                std::system_category(),
                                "log-dir creation failed");
    m_fn = log_dir + "/" + m_fn;

    std::string log_enable = m_perfflow_options["log-enable"];
    if (log_enable == "True")
    {
        m_enable_logging = 1;
    }
    else if (log_enable == "False" || log_enable == "false" ||
             log_enable == "FALSE")
    {
        m_enable_logging = 0;
    }
    else
    {
        throw std::system_error(errno,
                                std::system_category(),
                                "invalid log-enable value");
    }

    std::string usage_enable = m_perfflow_options["cpu_mem_usage"];
    if (usage_enable == "False")
    {
        m_usage_enable = 0;
    }
    else if (usage_enable == "True" || usage_enable == "true" ||
             usage_enable == "TRUE")
    {
        m_usage_enable = 1;
    }
    else
    {
        throw std::system_error(errno,
                                std::system_category(),
                                "invalid usage-enable value");
    }

    m_before_counter_mutex = PTHREAD_MUTEX_INITIALIZER;
    m_after_counter_mutex = PTHREAD_MUTEX_INITIALIZER;
    m_mutex = PTHREAD_MUTEX_INITIALIZER;
}

advice_chrome_tracing_t::~advice_chrome_tracing_t ()
{
    flush_if(1);
    if (m_ofs.is_open())
    {
        m_ofs.close();
    }
}

int advice_chrome_tracing_t::write_to_sstream(const char *str)
{
    int rc;
    if ((rc = pthread_mutex_lock(&m_mutex)) < 0)
    {
        return rc;
    }
    m_oss << str << "," << std::endl;
    if ((rc = pthread_mutex_unlock(&m_mutex)) < 0)
    {
        return rc;
    }
    return rc;
}

double advice_chrome_tracing_t::get_wall_time()
{
    struct timeval start;
    double time_d;
    if (gettimeofday(&start, NULL))
    {
        return 0;
    }
    time_d = (double)start.tv_sec + (double)start.tv_usec * .000001;
    return time_d;
}

double advice_chrome_tracing_t::get_cpu_time()
{
    struct rusage usage;
    struct timeval time_u;
    double time_d;
    getrusage(RUSAGE_SELF, &usage);
    time_u = usage.ru_utime;
    time_d = (double) time_u.tv_sec + (double) time_u.tv_usec / 1000000;
    return time_d;
}

long advice_chrome_tracing_t::get_memory_usage()
{
    struct rusage usage;
    long max_ram_usage;
    getrusage(RUSAGE_SELF, &usage);
    max_ram_usage = usage.ru_maxrss;
    return max_ram_usage;
}

int advice_chrome_tracing_t::with_flow(const char *module,
                                       const char *function,
                                       const char *flow,
                                       int64_t id)
{
    int rc;
    char *json_str;
    json_t *event, *ph;
    double my_ts;

    if (m_enable_logging)
    {
        if ((rc = create_event(&event, module, function, my_ts)) < 0)
        {
            return rc;
        }
        if (std::string("in") == flow)
        {
            if ((rc = encode_event(event, "f", nullptr, "e", -1, -1, -1)) < 0)
            {
                json_decref(event);
                return rc;
            }
        }
        else if (std::string("out") == flow)
        {
            if ((rc = encode_event(event, "s", nullptr, "e", -1, -1, -1)) < 0)
            {
                json_decref(event);
                return rc;
            }
        }
        else
        {
            json_decref(event);
            errno = EINVAL;
            return -1;
        }
        json_t *id_o;
        if (!(id_o = json_integer(static_cast<json_int_t>(id))))
        {
            json_decref(event);
            errno = ENOMEM;
            return -1;
        }
        if (json_object_set_new(event, "id", id_o) < 0)
        {
            json_decref(event);
            errno = ENOMEM;
            return -1;
        }
        if (!(json_str = json_dumps(event, JSON_INDENT(0))))
        {
            json_decref(event);
            errno = ENOMEM;
            return -1;
        }
        if ((rc = write_to_sstream(json_str)) < 0)
        {
            json_decref(event);
            return rc;
        }
        free(json_str);
        json_decref(event);
        return flush_if(FLUSH_SIZE);
    }
    else
    {
        return 0;
    }
}

int advice_chrome_tracing_t::before(const char *module,
                                    const char *function,
                                    const char *flow,
                                    const char *pcut)
{
    int rc;
    char *json_str;
    json_t *event, *ph, *jtemp;
    double my_ts = 0, cpu_start, wall_start;
    std::string fname;
    long mem_start;

    if (m_enable_logging)
    {
        if ((rc = create_event(&event, module, function, my_ts)) < 0)
        {
            return rc;
        }
        if ((rc = encode_event(event, "B", nullptr, nullptr, -1, -1, -1)) < 0)
        {
            json_decref(event);
            return rc;
        }
        if (!(json_str = json_dumps(event, JSON_INDENT(0))))
        {
            json_decref(event);
            errno = ENOMEM;
            return -1;
        }
        if ((rc = write_to_sstream(json_str)) < 0)
        {
            json_decref(event);
            return rc;
        }

        if (std::string("around") == pcut && m_usage_enable == 1)
        {
            jtemp = json_object_get(event, "name");
            std::string my_name = json_string_value(jtemp);
            jtemp = json_object_get(event, "pid");
            int my_pid = (int) json_integer_value(jtemp);
            jtemp = json_object_get(event, "tid");
            int my_tid = (int) json_integer_value(jtemp);

            fname = my_name + "_" + std::to_string(my_pid) + "_" + std::to_string(
                        my_tid) + ".txt";
            std::ofstream myfile(fname.c_str());
            cpu_start = get_cpu_time();
            wall_start = get_wall_time();
            mem_start = get_memory_usage();

            if (myfile.is_open())
            {
                myfile << std::to_string(cpu_start) << "\n";
                myfile << std::to_string(wall_start) << "\n";
                myfile << std::to_string(my_ts) << "\n";
                myfile << std::to_string(mem_start) << "\n";
                myfile.close();
            };
        }

        free(json_str);
        json_decref(event);
        if ((rc = flush_if(FLUSH_SIZE)) < 0)
        {
            return rc;
        }
        if (std::string("NA") != flow)
        {
            const char *eff_flow = flow;
            if (std::string("inout") == flow)
            {
                eff_flow = "in";
            }
            else if (std::string("outin") == flow)
            {
                eff_flow = "out";
            }
            rc = with_flow("flow", "flow", eff_flow, 100);
        }
        return rc;
    }
    else
    {
        return 0;
    }
}

int advice_chrome_tracing_t::after(const char *module,
                                   const char *function,
                                   const char *flow,
                                   const char *pcut)
{
    double cpu_usage, wall_time;
    long mem_usage;
    if (std::string("around") == pcut)
    {
        cpu_usage = get_cpu_time();
        wall_time = get_wall_time();
        mem_usage =  get_memory_usage();
    }

    int rc;
    char *json_str;
    json_t *event, *ph, *jtemp;
    double my_ts = 0, prev_ts, cpu_start, wall_start, cpu_percentage;
    std::string line, fname;
    long mem_start;

    if (m_enable_logging)
    {
        if (std::string("NA") != flow)
        {
            const char *eff_flow = flow;
            if (std::string("inout") == flow)
            {
                eff_flow = "out";
            }
            else if (std::string("outin") == flow)
            {
                eff_flow = "in";
            }
            if ((rc = with_flow("flow", "flow", eff_flow, 100)) < 0)
            {
                return rc;
            }
        }
        if ((rc = create_event(&event, module, function, my_ts)) < 0)
        {
            return rc;
        }
        if ((rc = encode_event(event, "E", nullptr, nullptr, -1, -1, -1)) < 0)
        {
            json_decref(event);
            return rc;
        }
        if (!(json_str = json_dumps(event, JSON_INDENT(0))))
        {
            json_decref(event);
            errno = ENOMEM;
            return -1;
        }
        if ((rc = write_to_sstream(json_str)) < 0)
        {
            json_decref(event);
            return rc;
        }

        if (std::string("around") == pcut && m_usage_enable == 1)
        {
            jtemp = json_object_get(event, "name");
            std::string my_name = json_string_value(jtemp);
            jtemp = json_object_get(event, "pid");
            int my_pid = (int) json_integer_value(jtemp);
            jtemp = json_object_get(event, "tid");
            int my_tid = (int) json_integer_value(jtemp);

            fname = my_name + "_" + std::to_string(my_pid) + "_" + std::to_string(
                        my_tid) + ".txt";
            std::ifstream myfile(fname.c_str());
            if (myfile.is_open())
            {
                std::vector<std::string> lines;
                while (getline(myfile, line))
                {
                    lines.push_back(line);
                }
                cpu_start = std::stod(lines[0]);
                wall_start = std:: stod(lines[1]);
                prev_ts = std::stod(lines[2]);
                mem_start = std::stol(lines[3]);
                myfile.close();

                int status = remove(fname.c_str());
                if (status != 0)
                {
                    std::perror("Error deleting file\n");
                }
            }

            cpu_usage = cpu_usage - cpu_start;
            if (cpu_usage < 0.0001)
            {
                cpu_usage = 0;
            }
            wall_time = wall_time - wall_start;
            cpu_percentage = (cpu_usage / wall_time) * 100;

            if ((rc = create_event(&event, module, function, prev_ts)) < 0)
            {
                return rc;
            }
            if ((rc = encode_event(event, "C", nullptr, nullptr, -1, cpu_percentage,
                                   mem_usage)) < 0)
            {
                json_decref(event);
                return rc;
            }
            if (!(json_str = json_dumps(event, JSON_INDENT(0))))
            {
                json_decref(event);
                errno = ENOMEM;
                return -1;
            }
            if ((rc = write_to_sstream(json_str)) < 0)
            {
                json_decref(event);
                return rc;
            }

            if ((rc = create_event(&event, module, function, my_ts)) < 0)
            {
                return rc;
            }
            if ((rc = encode_event(event, "C", nullptr, nullptr, -1, 0.0, 0.0)) < 0)
            {
                json_decref(event);
                return rc;
            }
            if (!(json_str = json_dumps(event, JSON_INDENT(0))))
            {
                json_decref(event);
                errno = ENOMEM;
                return -1;
            }
            if ((rc = write_to_sstream(json_str)) < 0)
            {
                json_decref(event);
                return rc;
            }
        }

        free(json_str);
        json_decref(event);
        return flush_if(FLUSH_SIZE);
    }
    else
    {
        return 0;
    }
}

int advice_chrome_tracing_t::before_async(const char *module,
        const char *function,
        const char *scope,
        const char *flow)
{
    int rc;
    char *json_str;
    json_t *event, *ph, *sc;
    double my_ts;

    if (m_enable_logging)
    {
        if ((rc = create_event(&event, module, function, my_ts)) < 0)
        {
            return rc;
        }
        if ((rc = pthread_mutex_lock(&m_before_counter_mutex)) < 0)
        {
            json_decref(event);
            return rc;
        }
        if ((rc = encode_event(event, "b", scope, nullptr,
                               m_before_counter, -1, -1)) < 0)
        {
            json_decref(event);
            return rc;
        }
        m_before_counter++;
        if ((rc = pthread_mutex_unlock(&m_before_counter_mutex)) < 0)
        {
            json_decref(event);
            return rc;
        }
        if (!(json_str = json_dumps(event, JSON_INDENT(0))))
        {
            json_decref(event);
            errno = ENOMEM;
            return -1;
        }
        if ((rc = write_to_sstream(json_str)) < 0)
        {
            json_decref(event);
            return rc;
        }
        free(json_str);
        json_decref(event);
        if ((rc = flush_if(FLUSH_SIZE)) < 0)
        {
            return rc;
        }
        if (std::string("NA") != flow)
        {
            const char *eff_flow = flow;
            if (std::string("inout") == flow)
            {
                eff_flow = "in";
            }
            else if (std::string("outin") == flow)
            {
                eff_flow = "out";
            }
            rc = with_flow("flow", "flow", flow, 100);
        }
        return rc;
    }
    else
    {
        return 0;
    }
}

int advice_chrome_tracing_t::after_async(const char *module,
        const char *function,
        const char *scope,
        const char *flow)
{
    int rc;
    char *json_str;
    json_t *event, *ph, *sc;
    double my_ts;

    if (m_enable_logging)
    {
        if (std::string("NA") != flow)
        {
            const char *eff_flow = flow;
            if (std::string("inout") == flow)
            {
                eff_flow = "out";
            }
            else if (std::string("outin") == flow)
            {
                eff_flow = "in";
            }
            if ((rc = with_flow("flow", "flow", eff_flow, 100)) < 0)
            {
                return rc;
            }
        }
        if ((rc = create_event(&event, module, function, my_ts)) < 0)
        {
            return rc;
        }
        if ((rc = pthread_mutex_lock(&m_after_counter_mutex)) < 0)
        {
            json_decref(event);
            return rc;
        }
        if ((rc = encode_event(event, "e", scope,
                               nullptr, m_after_counter, -1, -1)) < 0)
        {
            json_decref(event);
            return rc;
        }
        m_after_counter++;
        if ((rc = pthread_mutex_unlock(&m_before_counter_mutex)) < 0)
        {
            json_decref(event);
            return rc;
        }
        if (!(json_str = json_dumps(event, JSON_INDENT(0))))
        {
            json_decref(event);
            errno = ENOMEM;
            return -1;
        }
        if ((rc = write_to_sstream(json_str)) < 0)
        {
            json_decref(event);
            return rc;
        }
        free(json_str);
        json_decref(event);
        return flush_if(FLUSH_SIZE);
    }
    else
    {
        return 0;
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
