#!/bin/sh

test_description='Test basics of Perfflow Aspect python binding'

. ../../common/sharness/sharness.sh

export PYTHONPATH=../../

fixup_ctf_file(){
    ifn=$1 &&
    ofn=$2 &&
    cp ${ifn} ${ofn} &&
    echo " {}]" >> ${ofn}
}

sanity_check(){
    fixup_ctf_file $1 sanity &&
    jq . sanity &&
    test $(jq "length" sanity) -eq 25 &&
    jq ".[].name" sanity | sort | uniq > uniq_names &&
    test_cmp expected uniq_names &&
    rm -f sanity uniq_names
}

test_expect_success 'c binding: producing expected output' '
    cat > expected <<-EOF
	"bar"
	"bas"
	"foo"
	null
EOF
'

test_expect_success 'py binding: smoketest runs ok in default' '
    ../smoketest.py
'

test_expect_success 'py binding: ctf file appears good' '
    sanity_check perfflow.$(hostname).[0-9]*.pfw &&
    rm perfflow.$(hostname).[0-9]*.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: log-dir works' '
    PERFFLOW_OPTIONS="log-dir=./logdir" ../smoketest.py &&
    sanity_check ./logdir/perfflow.$(hostname).[0-9]*.pfw &&
    rm ./logdir/perfflow.$(hostname).[0-9]*.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: name can be included in filename' '
    PERFFLOW_OPTIONS="name=mycomponent:log-filename-include=name" \
	../smoketest.py &&
    test -f perfflow.mycomponent.pfw &&
    sanity_check perfflow.mycomponent.pfw &&
    rm perfflow.mycomponent.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: instance-path included in filename' '
    PERFFLOW_OPTIONS="log-filename-include=instance-path" ../smoketest.py &&
    test -f perfflow.{[a-f0-9]*}.pfw &&
    sanity_check perfflow.{[a-f0-9]*}.pfw &&
    rm perfflow.{[a-f0-9]*}.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: filename includes all' '
    PERFFLOW_OPTIONS="log-filename-include=name,instance-path,hostname,pid" \
	../smoketest.py &&
    test -f perfflow.generic.{[a-f0-9]*}.$(hostname).[0-9]*.pfw &&
    sanity_check perfflow.generic.{[a-f0-9]*}.$(hostname).[0-9]*.pfw &&
    rm perfflow.generic.{[a-f0-9]*}.$(hostname).[0-9]*.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: filename includes all in correct order' '
    PERFFLOW_OPTIONS="log-filename-include=hostname,instance-path,pid,name" \
	../smoketest.py &&
    test -f perfflow.$(hostname).{[a-f0-9]*}.[0-9]*.generic.pfw &&
    sanity_check perfflow.$(hostname).{[a-f0-9]*}.[0-9]*.generic.pfw &&
    rm perfflow.$(hostname).{[a-f0-9]*}.[0-9]*.generic.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: SLURM supported' '
    SLURM_JOB_ID=123456 SLURM_STEP_ID=1 \
PERFFLOW_OPTIONS="log-filename-include=instance-path" ../smoketest.py &&
    sha1=$(echo -n "123456.1" | sha1sum | awk "{print \$1}" | cut -c1-8) &&
    test -f perfflow.{${sha1}}.pfw &&
    sanity_check perfflow.{${sha1}}.pfw &&
    rm perfflow.{${sha1}}.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: LSF supported' '
    LSB_JOBID=123456 LS_JOBPID=1 \
PERFFLOW_OPTIONS="log-filename-include=instance-path" ../smoketest.py &&
    sha1=$(echo -n "123456.1" | sha1sum | awk "{print \$1}" | cut -c1-8) &&
    test -f perfflow.{${sha1}}.pfw &&
    sanity_check perfflow.{${sha1}}.pfw &&
    rm perfflow.{${sha1}}.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: SLURM_JOB_ID + STEP_ID must be given' '
    SLURM_JOB_ID=123456 \
PERFFLOW_OPTIONS="log-filename-include=instance-path" ../smoketest.py &&
    sha1=$(echo -n "1" | sha1sum | awk "{print \$1}" | cut -c1-8) &&
    test -f perfflow.{${sha1}}.pfw &&
    sanity_check perfflow.{${sha1}}.pfw &&
    rm perfflow.{${sha1}}.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: LSB_JOBID alone will not work' '
    LSB_JOB_ID=123456 \
PERFFLOW_OPTIONS="log-filename-include=instance-path" ../smoketest.py &&
    sha1=$(echo -n "1" | sha1sum | awk "{print \$1}" | cut -c1-8) &&
    test -f perfflow.{${sha1}}.pfw &&
    sanity_check perfflow.{${sha1}}.pfw &&
    rm perfflow.{${sha1}}.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: Flux supported' '
    FLUX_JOB_ID=123456 \
PERFFLOW_OPTIONS="log-filename-include=instance-path" ../smoketest.py &&
    test -f perfflow.{123456}.pfw &&
    sanity_check perfflow.{123456}.pfw &&
    rm perfflow.{123456}.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: Flux f58-encoded jobid supported' '
    FLUX_JOB_ID=ƒeF9QZG3 \
PERFFLOW_OPTIONS="log-filename-include=instance-path" ../smoketest.py &&
    test -f perfflow.{ƒeF9QZG3}.pfw &&
    sanity_check perfflow.{ƒeF9QZG3}.pfw &&
    rm perfflow.{ƒeF9QZG3}.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: nested instance-path supported' '
    PERFFLOW_INSTANCE_PATH=fffffff.444444 FLUX_JOB_ID=123456 \
PERFFLOW_OPTIONS="log-filename-include=instance-path" ../smoketest.py &&
    test -f perfflow.{fffffff.444444.123456}.pfw &&
    sanity_check perfflow.{fffffff.444444.123456}.pfw &&
    rm perfflow.{fffffff.444444.123456}.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: TOML config works' '
    PERFFLOW_TOML_FILE="../perfflowaspect_config.toml" ../smoketest.py
    sanity_check ./logdir-test/perfflow.helloworld.$(hostname).[0-9]*.pfw
    rm -rf ./logdir-test
'

test_done
