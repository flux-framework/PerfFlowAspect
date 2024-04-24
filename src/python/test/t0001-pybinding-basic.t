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

sanity_check_compact(){
    fixup_ctf_file $1 sanity &&
    jq . sanity &&
    test $(jq "length" sanity) -eq 13 &&
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
    sanity_check perfflow.array.$(hostname).[0-9]*.pfw &&
    rm perfflow.array.$(hostname).[0-9]*.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: log-dir works' '
    PERFFLOW_OPTIONS="log-dir=./logdir" ../smoketest.py &&
    sanity_check ./logdir/perfflow.array.$(hostname).[0-9]*.pfw &&
    rm ./logdir/perfflow.array.$(hostname).[0-9]*.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: name can be included in filename' '
    PERFFLOW_OPTIONS="name=mycomponent:log-filename-include=name" \
	../smoketest.py &&
    test -f perfflow.array.mycomponent.pfw &&
    sanity_check perfflow.array.mycomponent.pfw &&
    rm perfflow.array.mycomponent.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: instance-path included in filename' '
    PERFFLOW_OPTIONS="log-filename-include=instance-path" ../smoketest.py &&
    test -f perfflow.array.{[a-f0-9]*}.pfw &&
    sanity_check perfflow.array.{[a-f0-9]*}.pfw &&
    rm perfflow.array.{[a-f0-9]*}.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: filename includes all' '
    PERFFLOW_OPTIONS="log-filename-include=name,instance-path,hostname,pid" \
	../smoketest.py &&
    test -f perfflow.array.generic.{[a-f0-9]*}.$(hostname).[0-9]*.pfw &&
    sanity_check perfflow.array.generic.{[a-f0-9]*}.$(hostname).[0-9]*.pfw &&
    rm perfflow.array.generic.{[a-f0-9]*}.$(hostname).[0-9]*.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: filename includes all in correct order' '
    PERFFLOW_OPTIONS="log-filename-include=hostname,instance-path,pid,name" \
	../smoketest.py &&
    test -f perfflow.array.$(hostname).{[a-f0-9]*}.[0-9]*.generic.pfw &&
    sanity_check perfflow.array.$(hostname).{[a-f0-9]*}.[0-9]*.generic.pfw &&
    rm perfflow.array.$(hostname).{[a-f0-9]*}.[0-9]*.generic.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: SLURM supported' '
    SLURM_JOB_ID=123456 SLURM_STEP_ID=1 \
PERFFLOW_OPTIONS="log-filename-include=instance-path" ../smoketest.py &&
    sha1=$(echo -n "123456.1" | sha1sum | awk "{print \$1}" | cut -c1-8) &&
    test -f perfflow.array.{${sha1}}.pfw &&
    sanity_check perfflow.array.{${sha1}}.pfw &&
    rm perfflow.array.{${sha1}}.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: LSF supported' '
    LSB_JOBID=123456 LS_JOBPID=1 \
PERFFLOW_OPTIONS="log-filename-include=instance-path" ../smoketest.py &&
    sha1=$(echo -n "123456.1" | sha1sum | awk "{print \$1}" | cut -c1-8) &&
    test -f perfflow.array.{${sha1}}.pfw &&
    sanity_check perfflow.array.{${sha1}}.pfw &&
    rm perfflow.array.{${sha1}}.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: SLURM_JOB_ID + STEP_ID must be given' '
    SLURM_JOB_ID=123456 \
PERFFLOW_OPTIONS="log-filename-include=instance-path" ../smoketest.py &&
    sha1=$(echo -n "1" | sha1sum | awk "{print \$1}" | cut -c1-8) &&
    test -f perfflow.array.{${sha1}}.pfw &&
    sanity_check perfflow.array.{${sha1}}.pfw &&
    rm perfflow.array.{${sha1}}.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: LSB_JOBID + LS_JOBPID must be given' '
    LSB_JOB_ID=123456 \
PERFFLOW_OPTIONS="log-filename-include=instance-path" ../smoketest.py &&
    sha1=$(echo -n "1" | sha1sum | awk "{print \$1}" | cut -c1-8) &&
    test -f perfflow.array.{${sha1}}.pfw &&
    sanity_check perfflow.array.{${sha1}}.pfw &&
    rm perfflow.array.{${sha1}}.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: Flux supported' '
    FLUX_JOB_ID=123456 \
PERFFLOW_OPTIONS="log-filename-include=instance-path" ../smoketest.py &&
    test -f perfflow.array.{123456}.pfw &&
    sanity_check perfflow.array.{123456}.pfw &&
    rm perfflow.array.{123456}.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: Flux f58-encoded jobid supported' '
    FLUX_JOB_ID=ƒeF9QZG3 \
PERFFLOW_OPTIONS="log-filename-include=instance-path" ../smoketest.py &&
    test -f perfflow.array.{ƒeF9QZG3}.pfw &&
    sanity_check perfflow.array.{ƒeF9QZG3}.pfw &&
    rm perfflow.array.{ƒeF9QZG3}.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: nested instance-path supported' '
    PERFFLOW_INSTANCE_PATH=fffffff.444444 FLUX_JOB_ID=123456 \
PERFFLOW_OPTIONS="log-filename-include=instance-path" ../smoketest.py &&
    test -f perfflow.array.{fffffff.444444.123456}.pfw &&
    sanity_check perfflow.array.{fffffff.444444.123456}.pfw &&
    rm perfflow.array.{fffffff.444444.123456}.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: disable logging smoketest' '
    PERFFLOW_OPTIONS="log-enable=False" ../smoketest.py &&
    ! test -f perfflow.array.$(hostname).[0-9]*.pfw
    if test -f perfflow.array.$(hostname).[0-9]*.pfw; then
        rm perfflow.array.$(hostname).[0-9]*.pfw
    fi
'

test_expect_success 'PERFFLOW_OPTIONS: enable logging smoketest' '
    PERFFLOW_OPTIONS="log-enable=True" ../smoketest.py &&
    test -f perfflow.array.$(hostname).[0-9]*.pfw &&
    rm perfflow.array.$(hostname).[0-9]*.pfw
'

test_expect_success 'PERFFLOW_OPTIONS: disable logging smoketest_future' '
    PERFFLOW_OPTIONS="log-enable=False" ../smoketest_future.py &&
    ! test -f perfflow.array.$(hostname).[0-9]*.pfw
    if test -f perfflow.array.$(hostname).[0-9]*.pfw; then
        rm perfflow.array.$(hostname).[0-9]*.pfw
    fi
'

test_expect_success 'PERFFLOW_OPTIONS: disable logging smoketest_future2' '
    PERFFLOW_OPTIONS="log-enable=False" ../smoketest_future2.py &&
    ! test -f perfflow.array.$(hostname).[0-9]*.pfw
    if test -f perfflow.array.$(hostname).[0-9]*.pfw; then
        rm perfflow.array.$(hostname).[0-9]*.pfw
    fi
'

test_expect_success 'PERFFLOW_OPTIONS: disable logging smoketest_direct' '
    PERFFLOW_OPTIONS="log-enable=False" ../smoketest_direct.py &&
    ! test -f perfflow.array.$(hostname).[0-9]*.pfw
    if test -f perfflow.array.$(hostname).[0-9]*.pfw; then
        rm perfflow.array.$(hostname).[0-9]*.pfw
    fi
'

test_expect_success 'PERFFLOW_OPTIONS: disable logging smoketest_future_direct' '
    PERFFLOW_OPTIONS="log-enable=False" ../smoketest_future_direct.py &&
    ! test -f perfflow.array.$(hostname).[0-9]*.pfw
    if test -f perfflow.array.$(hostname).[0-9]*.pfw; then
        rm perfflow.array.$(hostname).[0-9]*.pfw
    fi
'

test_expect_success 'PERFFLOW_OPTIONS: use compact format smoketest' '
    PERFFLOW_OPTIONS="log-event=compact" ../smoketest.py &&
    sanity_check_compact ./perfflow.array.$(hostname).[0-9]*.pfw &&
    if test -f perfflow.array.$(hostname).[0-9]*.pfw; then
        rm perfflow.array.$(hostname).[0-9]*.pfw
    fi
'

test_expect_success 'PERFFLOW_OPTIONS: use verbose (default) format smoketest' '
    PERFFLOW_OPTIONS="log-event=verbose" ../smoketest.py &&
    sanity_check ./perfflow.array.$(hostname).[0-9]*.pfw &&
    if test -f perfflow.array.$(hostname).[0-9]*.pfw; then
        rm perfflow.array.$(hostname).[0-9]*.pfw
    fi
'

test_expect_success 'PERFFLOW_OPTIONS: output object format smoketest' '
    PERFFLOW_OPTIONS="log-enable=True:log-format=object" ../smoketest.py &&
    test -f perfflow.object.$(hostname).[0-9]*.pfw &&
    rm perfflow.object.$(hostname).[0-9]*.pfw
'

# Run cuda tests if NVIDIA GPU is present
lspci=$(lspci | grep -i nvidia 2>/dev/null)
if [ -n "${lspci}" ]; then
    test_expect_success 'py binding: smoketest_cuda runs ok in default' '
        ../smoketest_cuda.py &&
        rm perfflow.array.$(hostname).[0-9]*.pfw
    '

    test_expect_success 'PERFFLOW_OPTIONS: disable logging smoketest_cuda' '
        PERFFLOW_OPTIONS="log-enable=False" ../smoketest_cuda.py &&
        ! test -f perfflow.array.$(hostname).[0-9]*.pfw &&
        if test -f perfflow.array.$(hostname).[0-9]*.pfw; then rm perfflow.array.$(hostname).[0-9]*.pfw; fi
    '

    test_expect_success 'PERFFLOW_OPTIONS: enable logging smoketest_cuda' '
        PERFFLOW_OPTIONS="log-enable=True" ../smoketest_cuda.py &&
        test -f perfflow.array.$(hostname).[0-9]*.pfw &&
        rm perfflow.array.$(hostname).[0-9]*.pfw
    '
else
    say "Skipping CUDA smoketests...NVIDIA GPU not found."
fi

test_done
