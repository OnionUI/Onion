opts=":ho:n:vi:f:"
file=""

old_bin_total_time=0
new_bin_total_time=0

old_bin_iterations="Old bin iterations:\n"
new_bin_iterations="New bin iterations:\n"

fmt="%-10s %12s %18s"
printf -v header "$fmt" "Iteration:" "Old bin:" "New bin:\n"
verbose_iterations="$header"

iterations=10

old_bin=""
new_bin=""
verbose=false

perform_iterations(){

  fmt="%-6s %16s %-1s %13s %-1s"

  for (( i=0; i < "$iterations"; i++ ))do
    ts=$(date +%s%N)
    $old_bin >/dev/null 2>&1    
    old_bin_time=$((($(date +%s%N) - $ts)))

    ts=$(date +%s%N)
    $new_bin> /dev/null 2>&1
    new_bin_time=$((($(date +%s%N) - $ts)))
    printf -v line_output "$fmt" "$(( i + 1 ))" "${old_bin_time}" "ns" "${new_bin_time}" "ns"
    verbose_iterations="$verbose_iterations$line_output\n"

    old_bin_total_time=$(awk "BEGIN {print $old_bin_total_time + $old_bin_time}")
    new_bin_total_time=$(awk "BEGIN {print $new_bin_total_time + $new_bin_time}")

  done
  old_bin_total_time=$(echo "scale=5; $old_bin_total_time / $iterations" | bc -l)
  new_bin_total_time=$(echo "scale=5; $new_bin_total_time / $iterations" | bc -l)

  if [ "$new_bin_total_time" == 0 ]; then
    ratio=$(echo "scale=5; 1 / $old_bin_total_time" | bc -l)
    ratio="$ratio faster than the old binary"
  elif [ "$old_bin_total_time" == 0 ]; then
    ratio=$(echo "scale=5; 1 / $new_bin_total_time" | bc -l)
    ratio="$ratio slower than the old binary"
  else
    ratio=$(echo "scale=5; $old_bin_total_time / $new_bin_total_time" | bc -l)
    ratio="$ratio faster than the old binary"
  fi
}

print_help() {
  echo -e "How to use:\n"
  echo "Syntax: ./benchmark [h|-n|-o|-i|-v]"
  echo "options:"
  echo "-h: Prints the help menu"
  echo "-o: Defines the old binary you wish to benchmark"
  echo "-n: Defines the new binary you wish to benchmark"
  echo "-v: Set output as verbose"
  echo "-i: Set ammount of iterations (Default is 10)"
  echo "For -o and -n you need to pass it like \"binary arguments\" and the script will call it accordingly" 
}

main() {
  while getopts "$opts" opt 
  do
    case "$opt" in
      h ) print_help && exit 0 ;;
      o ) old_bin="$OPTARG" ;;
      n ) new_bin="$OPTARG" ;;
      v ) verbose=true ;;
      i ) iterations=$(echo "$OPTARG") ;;
      f) file="$OPTARG" ;;
      ? ) print_help && exit 1 ;;
    esac
  done

  if [ "$new_bin" = "" ] || [ "$old_bin" = "" ];then
    echo "Not enough arguments were passed"
    print_help
    exit 1
  fi

  old_bin=${old_bin#\"}
  old_bin=${old_bin%\"}

  new_bin=${new_bin#\"}
  new_bin=${new_bin%\"}


  perform_iterations

  if [ "$file" != "" ]; then 
    echo "The old binary took $old_bin_total_time ns" > "$file"
    echo "The new binary took $new_bin_total_time ns">> "$file"
    echo "It's $ratio">> "$file"
    echo "Iterations performed:$iterations">> "$file"

    if [ "$verbose" == true ]; then
      echo -e "$verbose_iterations\n">> "$file"
    fi

  else
    echo "The old binary took $old_bin_total_time ns" 
    echo "The new binary took $new_bin_total_time ns"
    echo "It's $ratio"
    echo "Iterations performed:$iterations"

    if [ "$verbose" == true ]; then
      echo -e "$verbose_iterations\n"
    fi
  fi

  exit 0
}

main "$@"
