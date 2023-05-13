#!/usr/bin/env sh

# Variable explain:
# $y: the current argument in the whole content
# $yy: the current argument in the list
# $pos: the current position on argument y
# $y2: the current position on current screen position
# $num: numbers of lines per argument
# $fin: bottom for printable area, depends on num
# $token: indicator for actions
# Dimension is adjustable by $top & $bottom

# Stolen from shfm
esc() {
    case $1 in
        # vt100 (IL is vt102) (DECTCEM is vt520)
	CUP)     printf '%s[%s;%sH' "$esc_c" "$2" "$3" ;;
	    # cursor to LINES($2), COLUMNS($3)
        CUU)     printf '%s[%sA'    "$esc_c" "$2"      ;;
	    # cursor up
        CUD)     printf '%s[%sB'    "$esc_c" "$2"      ;;
	    # cursor down
        CUR)     printf '%s[%sC'    "$esc_c" "$2"      ;;
	    # cursor right
	CUL)     printf '%s[%sD'    "$esc_c" "$2"      ;;
	    # cursor left
	DECAWM)  printf '%s[?7%s'   "$esc_c" "$2"      ;;
	    # (h: set; l: unset) line wrap
        DECRC)   printf '%s8'       "$esc_c"           ;;
	    # cursor restore
        DECSC)   printf '%s7'       "$esc_c"           ;;
	    # cursor save
        DECSTBM) printf '%s[%s;%sr' "$esc_c" "$2" "$3" ;;
	    # scroll region
	DECTCEM) printf '%s[?25%s'  "$esc_c" "$2"      ;;
	    # (h: show; l: hide) cursor visible
	ED[0-2]) printf '%s[%sJ'    "$esc_c" "${1#ED}" ;;
	    # clear screen
        EL[0-2]) printf '%s[%sK'    "$esc_c" "${1#EL}" ;;
	    # clear line
        IL)      printf '%s[%sL'    "$esc_c" "$2"      ;;
	    # insert line
	SGR)     printf '%s[%s;%sm' "$esc_c" "$2" "$3" ;;
	    # colors ($2); attribute ($3)

        # xterm (since 1988, supported widely)
	screen_alt) printf '%s[?1049%s' "$esc_c" "$2" ;;
	    # (h: to; l: back from) alternate buffer
    esac
}

term_setup() {
    bottom=$((LINES - 2))				# space for bottom status_line
    top=3						# space for top status_line
    fin=$(( bottom - (bottom - (top - 1)) % num ))	# bottom for content printed

    stty=$(stty -g)
    stty -icanon -echo
    esc screen_alt h
    esc DECAWM l
    esc DECTCEM l
    esc ED2
    esc DECSTBM "$top" "$fin"
}

term_getsize() {
    # false-positive, behavior intentional, globbing is disabled.
    # shellcheck disable=2046
    {
        set -f -- $(stty size)
        set +f
    }

    LINES=$1 COLUMNS=$2
}

term_reset() {
    esc DECAWM h     >&2
    esc DECTCEM h    >&2
    esc ED2          >&2
    esc DECSTBM      >&2
    esc screen_alt l >&2
    stty "$stty"
}

old_save() {
    old_num=$num
    old_y=$y
    old_yy=$yy
    old_y2=$y2
    old_pos=$pos
    old_format=$format
    old_msg=$msg
    old_ltype=$ltype
    old_last=$last
    old_len=$len
}

old_reset() {
    num=$old_num
    y=$old_y
    yy=$old_yy
    y2=$old_y2
    pos=$old_pos
    format=$old_format
    msg=$old_msg
    ltype=$old_ltype
    last=$old_last
    len=$old_len
}

input_assign() {
    offset=$1
    length=$2

    shift "$(( offset > 0 ? offset : 0 ))"; shift 2;

    i=1
    while [ "$i" -le "$length" ]; do
	save_list="$save_list${ifs:=$nl}$1"
	shift $(( $# > 0 ? 1 : 0 ))
	i=$(( i + 1 ))
    done

    save_list="${save_list#${ifs:=$nl}}"
    list=$save_list

    unset i offset length save_list
}


redraw() {

    pos=$(( num * y - num + top ))
    end=$(( fin + 1 ))
    width=$(( (end - top) / num ))

    input_assign "$(( y >= width ? y - width : 0 ))" "$width" "$@"

    IFS=${ifs:=$nl}
    # shellcheck disable=2086
    set -- $list
    unset IFS

    esc ED2

    status_line "$last"
    list_print "$@"

    unset list
}


list_print() {
    esc CUP "$top"

    unset cur

    i=1

    for file do

	case $(( num * i - num + top )) in
	    "$y2") cur=$file; save_cur=$file ;;
	    *) cur= ;;
	esac

	case $(( num * i - num + top - end )) in
	    -*)
		arg_format "$file"
		esc CUD
		i=$((i + 1))
		;;
	    0|*) break ;;
	esac

    done

    esc CUP "$(( pos > y2 ? y2 : pos ))"

    cur=$save_cur
    unset save_cur
}

arg_print() {
    shift "$1"
    arg_format "$1"
}


arg_format() {
    esc EL0

    # overall display rule
    case "$cur" in
	'') esc CUR 2; esc SGR '' 2 ;;
	*) esc CUR -2; esc SGR 35 1; printf '>'; esc SGR 33 1; esc CUR 1 ;;
    esac

    # content display format
    case "$format" in
	basename) printf '%s\r' "${1##*/}" ;;
	nldel) printf '%s\r' "${1%$nl}" ;;
	'') printf '%s\r' "$1" ;;
    esac

    esc SGR
}


term_move_up() {

    y=$(( y - move )) yy=$(( yy - move ))
    pos=$(( num * y - num + top ))

    # Generate list for partial input
    input_assign "$(( yy - 1 ))" "$(( move + 1 ))" "$@"
    IFS=${ifs:=$nl}
    # shellcheck disable=2086
    set -- $list
    unset IFS list cur

    i=1
    while [ "$i" -le "$move" ] ; do
	arg_print "$(( move - i + 2 ))" "$@" # print the latter item first

	case "$y2" in # adjust for upper bound
	    "$top") [ "$num" -gt 1 ] && esc CUU $(( num - 1 )); esc IL "$num" ;;
	    *) esc CUU "$(( num * 2 - 1 ))"; y2=$(( y2 > top ? y2 - num : top )) ;;
	esac

	i=$(( i + 1 ))
    done

    offset=$y
    cur=$1
    arg_format "$1"

    # Restore cursor to match y2
    [ "$num" -gt 1 ] && esc CUU "$(( num - 1 ))"

    status_line "$last"

    unset i offset
}

term_move_down() {

    y=$(( y + move )) yy=$(( yy + move ))
    pos=$(( num * y - num + top ))
    y2=$(( y2 + num * move < fin - (num - 1) ? y2 + num * move : fin - (num - 1) ))

    # Generate list for partial input
    input_assign "$(( yy - move - 1 ))" "$(( move + 1 ))" "$@"
    IFS=${ifs:=$nl}
    # shellcheck disable=2086
    set -- $list
    unset IFS cur

    i=1
    while [ "$i" -le "$move" ] ; do
	arg_format "$1"
	shift 1
	printf '\n'

	i=$(( i + 1 ))
    done

    offset=$y
    cur=$1
    arg_format "$1"

    [ "$num" -gt 1 ] && esc CUU "$(( num - 1 ))"

    status_line "$last"

    unset i offset
}

status_line () {
    esc DECSC
    esc CUP 1; esc EL2; printf '%s %s' "($y/$1)" "$msg"
    [ -n "$ltype" ] && { esc CUP "$LINES"; esc EL2; printf '%s' "$ltype"; }
    esc DECRC
}

prompt() {
    esc DECSC
    esc CUP "$LINES"
    printf %s "$1"
    esc DECTCEM h
    esc EL0

    case $2 in
        r)
	    stty -cread icanon echo 1>/dev/null 2>&1
	    read -r ans ||:
	    stty -icanon -echo
	    ;;
	l) press=$(dd ibs=1 count=1 2>/dev/null) ;;
    esac

    esc DECRC
    esc DECTCEM l
}

tab_complete() {
    shift "$(( count - 1 > 0 ? count - 1 : 0 ))"
    ans=$1
}

search() {
    search_list="$1"
    IFS="$2"
    str="$3"

    case $case_insense in
	1) # case-insensitive filter

	    # lowercase both search_list and str
	    lower_search_list=$(printf '%s' "$search_list" | dd conv=lcase 2>/dev/null)
	    lower_str=$(printf '%s' "$str" | dd conv=lcase 2>/dev/null)

	    # First run: match lowercase and record the number of positional parameter
	    i=1 s=1
	    # shellcheck disable=2086
	    set -- $lower_search_list
	    for line do
		case $line in
		    *$lower_str*)
			case $token in
			    l) # live search: only search until bound
				case $((s - bound)) in
				    -*|0) posnum="$posnum $i" ;;
				    *) break ;;
				esac
				s=$(( s + 1 ))
				;;
			    /) posnum="$posnum $i" ;;
			esac
		esac
		i=$(( i + 1 ))
	    done
	    posnum=${posnum#* } # delete first space

	    # Second run: match the item based on posnum above
	    # shellcheck disable=2086
	    set -- $search_list

	    j=1
	    for line do
		[ -z "$posnum" ] && break
		n=${posnum%% *} # first line number
		case $j in
		    "$n")
			filter="$filter$IFS$line"

			case $posnum in  # delete first number
			    *[[:space:]]*) posnum=${posnum#* } ;;
			    *) posnum= ;;
			esac
			;;
		esac
		j=$(( j + 1 ))
	    done

	    ;;
	*) # case-sensitive filter

	    s=1

	    # shellcheck disable=2086
	    set -- $search_list
	    for line do
		case $line in
		    *$str*)
			case $token in
			    l) # live search: only search until bound
				case $((s - bound)) in
				    -*|0) filter="$filter$IFS$line" ;;
				    *) break ;;
				esac
				s=$(( s + 1 ))
				;;
			    /) filter="$filter$IFS$line" ;;
			esac
		esac
	    done

	    ;;
    esac

    filt_out=${filter#*$IFS}

    unset search_list IFS str filter posnum lower_list lower_str i j n s

}

key() {

    loop=$(( loop + 1 ))

    # Generate new list
    case $(( y - move )) in
        1) y=1; yy=1; input_assign "0" "$len" "$@" ;;
	$(( last - move - 1 ))) y=$last; input_assign "$(( y - len ))" "$len" "$@" ;;
	*)
	    case $(( y - len )) in
	        -*) input_assign "$(( yy > 1 ? y - yy : y - 1 ))" "$len" "$@" ;;
		*) input_assign "$(( yy > 1 ? y - len : y - 1 ))" "$len" "$@" ;;
	    esac
    esac

    IFS=${ifs:=$nl}
    # shellcheck disable=2086
    set -- $list
    unset IFS

    while key=$(dd ibs=1 count=1 2>/dev/null); do

	

	case $key${esc:=0} in

	    "$ctrl_u"?|~5) # Ctrl-u / PageUp
		move=$(( width / 4 > 2 ? width / 4 : 2 ))

		# Terminal condition
		case $y in
		    -*|0|1) continue ;;
		esac

		# Adjust move step
		case $(( y - move - 1 )) in
		    -*|0) move=$(( y - 1 )) ;;
		esac

		# Move or update list
		case $(( yy - move )) in
		    -*|0) yy=$(( y > len ? len : y )); return 0 ;;
		    *) term_move_up "$@" ;;
		esac
	    ;;

	    "$ctrl_f"?|~6) # Ctrl-f / PageDown
		move=$(( width / 4 > 2 ? width / 4 : 2 ))

		# Terminal condition
		case $y in
		    "$last") continue ;;
		esac

		# Adjust move step
		case $(( y - last + move )) in
		    [1-9]*) move=$(( last - y )) ;;
		esac

		# Move or update list
		case $(( yy - len + move )) in
		    -*|0) term_move_down "$@" ;;
		    *) yy=1; return 0 ;;
		esac
	    ;;

	    k?|A2|"$ctrl_p"?) # k / Arrow Up / Ctrl-p
		move=1

		# Terminal condition
		case $y in
		    -*|0|1) continue ;;
		esac

		# Move or update list
		case $(( yy - move )) in
		    -*|0) yy=$len; return 0 ;;
		    *) term_move_up "$@" ;;
		esac
	    ;;

	    j?|B2|"$ctrl_n"?) # j / Arrow Down / Ctrl-n
		move=1

		# Terminal condition
		case $y in
		    "$last") continue ;;
		esac

		# Move or update list
		case $(( yy - len + move )) in
		    -*|0) term_move_down "$@" ;;
		    *) yy=1; return 0 ;;
		esac
	    ;;

	    g?|H2|"$ctrl_a"?) # g / Home / Ctrl-a

		# Terminal condition
		case $y in
		    1) continue ;;
		esac

		# Normal mode v.s. Search mode
		case $token in
		    '/')
			IFS=${ifs:=$nl}
			# shellcheck disable=2086
			set -- $filt_out
			unset IFS
			;;
		    '')
			IFS=${ifs:=$nl}
			# shellcheck disable=2086
			set -- $content
			unset IFS
		esac

		y=1 yy=1 y2=$top pos=$top
		redraw "$@"
		return 0
	    ;;

	    G?|F2|"$ctrl_e"?) # G / End / Ctrl-e

		# Terminal condition
		case $y in
		    "$last") continue ;;
		esac

		# Normal mode v.s. Search mode
		case $token in
		    '/')
			IFS=${ifs:=$nl}
			# shellcheck disable=2086
			set -- $filt_out
			unset IFS
			;;
		    '')
			IFS=${ifs:=$nl}
			# shellcheck disable=2086
			set -- $content
			unset IFS
		esac

		y=$last yy=$len pos=$(( num * y - num + top ))
		y2=$(( pos < fin - (num - 1) ? pos : fin - (num - 1) ))

		redraw "$@"
		return 0
	    ;;

	    l?|C2|"$space"?) # l / Arrow Right / Esc
		case $token in
		    '?') continue ;;
		    *)
			term_reset
			return 1
			;;
		esac
	    ;;

	    "$esc") # l / Arrow Right / Esc
		 	pkill -9 st
	    ;;
		
	    h?|D2|"$bs_char"?) # h / Arrow Left / Backspace

		case $token in
		    '?'|'/')
			unset token filt_out ans
			old_reset
			IFS=${ifs:=$nl}
			# shellcheck disable=2086
			set -- $content
			unset IFS
			y=1 yy=1 y2=$top
			redraw "$@"
			return 0
			;;
		esac
	    ;;

	    /?) # /
		count=0 bound=$width

		old_save

		while :; do

		    case $live_search in
		        '')
			    token='/'
			    prompt / r
			    ;;
			1)
			    token='l'
			    prompt "/$ans" l
				
			    case $press${esc:=0} in
				"$esc") # Enter
				    case $cur in
				        'no result') continue ;;
					*)
					    ltype="Searching..."
					    status_line "$last"
					    ltype="Search mode"
					    token='/'
					    case $# in
						1) # only one result, directly output it
						    term_reset
						    return 1
						    ;;
					    esac
					    ;;
				    esac
				    ;;
				"040") # Enter
				    case $cur in
				        'no result') continue ;;
					*)
					    ltype="Searching..."
					    status_line "$last"
					    ltype="Search mode"
					    token='/'
					    case $# in
						1) # only one result, directly output it
						    term_reset
						    return 1
						    ;;
					    esac
					    ;;
				    esac
				    ;;
				"$bs_char"?) # Backspace
				    case $ans in
				        '') continue ;; # Inactive when no ans
					*)
					    ans=${ans%%?}
					    count=0
					    unset complist
					    ;;
				    esac
				    ;;
				"$tab"?) # Tab
				    case $count in
				        0) # First run, record complist & $#
					    complist=$filt_out
					    IFS=${ifs:=$nl}
					    # shellcheck disable=2086
					    set -- $complist
					    unset IFS
					    tot=$#
					    count=$(( count + 1 <= $# ? count + 1 : $# ))
					    tab_complete "$@"
					    ;;
					*) # Other run, compare count and $#
					    case $(( count - tot )) in
					        -*)
						    IFS=${ifs:=$nl}
						    # shellcheck disable=2086
						    set -- $complist
						    unset IFS
						    count=$(( count + 1 <= $# ? count + 1 : $# ))
						    tab_complete "$@"
						    ;;
						*) continue ;;
					    esac
					    ;;
				    esac
				    ;;
				Z2) # Shift-Tab
				    case $count in
				        0) # First run, record complist
					    complist=$filt_out
					    IFS=${ifs:=$nl}
					    # shellcheck disable=2086
					    set -- $complist
					    unset IFS
					    count=$(( count - 1 >= 1 ? count - 1 : 1 ))
					    tab_complete "$@"
					    esc=0
					    ;;
					*) # Other run, compare count and 1
					    case $(( count - 1 )) in
					        -*|0) esc=0; continue ;;
						*)
						    IFS=${ifs:=$nl}
						    # shellcheck disable=2086
						    set -- $complist
						    unset IFS
						    count=$(( count - 1 >= 1 ? count - 1 : 1 ))
						    tab_complete "$@"
						    esc=0
						    ;;
					    esac
					    ;;
				    esac
				    ;;
				"$esc_c"*) esc=1; continue ;;
				'[1') esc=2; continue ;;
				[[:cntrl:]]?) esc=0; continue ;; # Do not accept other control char
				*?) # Others
				    esc=0
				    ans="$ans$press"
				    count=0
				    unset complist
				    ;;
			    esac
			    ;;
		    esac

		    search "$content" "$ifs" "$ans"

		    # redraw if different & not Enter
		    [ "$filt_out" = "$last_filt_out" ] && [ -n "$press" ] && continue

		    IFS=${ifs:=$nl}
		    # shellcheck disable=2086
		    set -- $filt_out
		    unset IFS


		    msg="Search by *${ans%%$nl*}*";

		    case $# in
		        0) num=1; set -- 'no result' ;;
		    esac

		    case $token in
		        l)
			    last='??'
			    last_filt_out="$filt_out"
			    redraw "$@"
			    ;;
			/)
			    last=$#
			    len=$(( $# > 500 ? 500 : $# ))
			    num=$old_num
			    term_getsize
			    term_setup
			    redraw "$@"
			    break
			    ;;
		    esac
		done

		unset count complist bound last_filt_out

		return 0
	    ;;

	    \??) # ?
		set -- 'k/↑/Ctrl-p - up' \
		       'j/↓/Ctrl-n - down' \
		       'l/→ - right' \
		       'h/← - left' \
		       'Ctrl-f/PageDown - PageDown' \
		       'Ctrl-u/PageUp - PageUp' \
		       'g/Home/Ctrl-a - go to top' \
		       'G/End/Ctrl-e - go to bottom' \
		       '/ - search' \
		       '    live-search detail:' \
		       '        Enter - confirm' \
		       '        Backspace - delete previous character' \
		       '        Tab - Tab-completion forward' \
		       '        Shift-Tab - Tab-completion backward' \
		       '        control char ignore' \
		       '        others print out' \
		       '? - show keybinds' \
		       'q - quit'

		unset format
		old_save
		num=1; y=1; yy=1; y2=$top; last=$#; len=$#; ltype=""; msg=keybinds; token='?';
		redraw "$@"
	    ;;

	    q?) # q
		term_reset
		exit 0
		;;
	    "$esc_c"*) esc=1 ;;
		 '[1') esc=2 ;;
		   5?) esc=5 ;; # PageUp
		   6?) esc=6 ;; # PageDown
		    *) esc=0 ;;
	esac
    done
}

main() {

    set -e

    trap 'term_reset; exit 0' INT QUIT
    trap 'term_getsize; term_setup; y=1; yy=1; y2=$top; pos=$top; redraw "$@"' WINCH

    term_getsize
    term_setup
    y=1 yy=1 y2=$top pos=$top

    IFS=${ifs:=$nl}
    # shellcheck disable=2086
    set -- $content
    unset IFS
    last=$#
    len=$(( $# > 500 ? 500 : $# ))

    input_assign "0" "$len" "$@"

    IFS=${ifs:=$nl}
    # shellcheck disable=2086
    set -- $list
    unset IFS

    redraw "$@"

    # shellcheck disable=2181
    while key "$@"; do
	case $token in
	    '/') # Search mode
		IFS=${ifs:=$nl}
		# shellcheck disable=2086
		set -- $filt_out
		unset IFS
		;;
	    '') # Normal
		IFS=${ifs:=$nl}
		# shellcheck disable=2086
		set -- $content
		unset IFS
	esac

    done
}

usage () {
cat << 	EOF
Usage:

shellect [OPTIONS] ([ARGS])

  -h,			Show help options
  -i,			Set case-insensitive search
  -l,			Set live-search
  -n=[num],		Set numbers of line per entry
  -d=[delim],		Set delimiter (IFS, internal field separator)
  -c=[content],		Set content to display
  -f=[format],		Set the format to print out content
  -t=[msg],		Set top status bar message
  -b=[msg],		Set bottom status bar message

format detail:
  nldel			delete last nl, equiv to "\${1%\$nl}"
  basename		only print basename, equiv to "\${1##*/}" ;;

  if unset or empty, then equiv to "\$1"

live-search detail:
  Enter 		confirm
  Backspace 		delete previous character
  Tab 			Tab-completion forward
  Shift-Tab		Tab-completion backward
  control char		ignore
  others		print out
EOF
}


nl='
'
tab='	'

# special key setting
esc_c=$(printf '\033')
bs_char=$(printf '\177')
ctrl_f=$(printf '\006')
ctrl_u=$(printf '\025')
ctrl_n=$(printf '\016')
ctrl_p=$(printf '\020')
ctrl_a=$(printf '\001')
ctrl_e=$(printf '\005')
space=$(printf '\040')
escape=$(printf '\033')

while getopts "t:b:n:d:c:f:hil" result; do
    case "${result}" in
        n) num=${OPTARG} ;;
        d) ifs=${OPTARG} ;;
        c) content=${OPTARG} ;;
        f) format=${OPTARG} ;;
	i) case_insense=1 ;;
	l) live_search=1 ;;
	t) msg="${OPTARG}" ;;
	b) ltype="${OPTARG}" ;;
	h) usage && exit 0 ;;
	*) printf 'Invalid argument' && exit 0 ;;
    esac
done

case "$content" in
    '') content=$(cat -u -) ;; # Accept pipe stdin
esac

num=${num:=1} # num=1 if unset
main <&2 >/dev/tty # why it works, I don't know.
printf '%s' "$cur" >&1
