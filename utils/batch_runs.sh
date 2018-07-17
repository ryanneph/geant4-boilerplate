function print_usage() {
    echo -e "Usage:  $0 input_filename [number_of_runs] [first_run_number]\n"
    echo -e "  Options:"
    echo -e "    input_filename:      path to valid *.in geant4 macro UI file"
    echo -e "    [number_of_runs]:    total number of batches (3)"
    echo -e "    [first_run_number]:  first number used in results numbering (1)"
}
function test_integer() {
    if [[ $1 =~ ^-?[0-9]+$ ]]; then
        return 0
    else return 1
    fi
}

# DEFAULT USER VARS
saved_runs_root='./batch_results/'
start_run_number=1  # start run number from this point
total_runs_number=3 # process this number of runs beginning from ${start_runs_number}
input_filename="$1"   # should be the name of an input file ie. full_8threads_32batches.in
binary_filename='validation'

# PARSE CLI ARGS
if (( $# < 1 )); then
    print_usage
    exit 1
fi
if test_integer $2; then
    total_runs_number="$2"
else
    echo "$2 is an invalid 'number_of_runs'. Using default ($total_runs_number)"
fi

if (( $# == 3 )); then
    if test_integer $3; then
        start_run_number="$3"
    else 
        echo "$3 is not a valid 'first_run_number'. Using default ($start_run_number)"
    fi
fi

iter=0
for run_number in $(seq ${start_run_number} $((${start_run_number} + ${total_runs_number} - 1)) ); do
    iter=$((iter+1))
    echo "Beginning Batch Run ${run_number} (${iter} of ${total_runs_number})"
    echo "-------------------------------------------"

    # GENERATED VARS
    run_results_path="${saved_runs_root}/${input_filename%.in}_run_${run_number}/"
    log_filename="log_${input_filename%.in}.txt"

    # Execute
    execute_command="./${binary_filename} ${input_filename}"
    echo "${execute_command} | tee ${log_filename}" | tee ${log_filename}
    ${execute_command} | tee -a ${log_filename}
    echo ''

    # Copy/Move results
    move_files=(*.bin "${log_filename}")
    copy_files=("${binary_filename}" "${input_filename}" 'geo.txt' 'tracked_beamlets.txt' *.in *.mac)
    echo "Setting results directory to \"${run_results_path}\""
    mkdir -p "${run_results_path}"
    echo moving files: "(${move_files[@]}) to \"${run_results_path}\"" 
    mv "${move_files[@]}" "${run_results_path}"
    echo copying files: "(${copy_files[@]}) to \"${run_results_path}\""
    cp "${copy_files[@]}" "${run_results_path}"

    echo ''
done
