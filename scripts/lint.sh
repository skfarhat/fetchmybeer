#!/bin/bash 
# lint.sh 
# This script is meant to sit alongside cpplint.py 

# TODO(sami): this file should run in two modes: commit-mode for commit scripts 
# and normal mode for when the user just wants to lint the file 

exec < /dev/tty

# color stuff 
export CLICOLOR=1
export LSCOLORS=ExFxCxDxBxegedabagacad
RED="$(tput setaf 1)"
CLEAR="$(tput sgr0)"

# get the script directory this is useful, 
# for when lint.sh is not run from its own directory but from outside
# Note: this does not follow symlinks 
SCRIPT_DIR="$(cd -P $(dirname ${BASH_SOURCE[0]}) && pwd)"
SCRIPT_PATH="$SCRIPT_DIR/$(basename $0)"

# main executable
CPPLINT="$SCRIPT_DIR/cpplint.py"

OUTPUT_FILE="$SCRIPT_DIR/linting.log"

# insure that cpplint.py resides in the same directory as lint.sh 
if [ ! -f $CPPLINT ]; then 
    echo "Could not find cpplint.py, it should be in the same directory as lint.sh"
    exit 1 
fi

#FILTERS='--filter='

#build/header_guard
#build/include
#build/include_alpha
#build/include_order
#build/include_what_you_use
#build/namespaces
#build/printf_format
#build/storage_class
#readability/alt_tokens
#readability/braces
#readability/casting
#readability/check
#readability/constructors
#readability/fn_size
#readability/inheritance
#readability/multiline_comment
#readability/multiline_string
#readability/namespace
#readability/nolint
#readability/nul
#readability/strings
#readability/todo
#readability/utf8
#runtime/arrays
#runtime/casting
#runtime/explicit
#runtime/int
#runtime/init
#runtime/invalid_increment
#runtime/member_string_references
#runtime/memset
#runtime/indentation_namespace
#runtime/operator
#runtime/printf
#runtime/printf_format
#runtime/references
#runtime/string
#runtime/threadsafe_fn
#runtime/vlog
#whitespace/blank_line
#whitespace/braces
#whitespace/comma
#whitespace/comments
#whitespace/empty_conditional_body
#whitespace/empty_if_body
#whitespace/empty_loop_body
#whitespace/end_of_line
#whitespace/ending_newline
#whitespace/forcolon
#whitespace/indent
#whitespace/line_length
#whitespace/newline
#whitespace/operators
#whitespace/parens
#whitespace/semicolon
#whitespace/tab
#whitespace/todo

# execute cpplint.py
EXTENSIONS="--extensions=hpp,cpp,c,h"
lint_out="$($CPPLINT $EXTENSIONS $FILTERS $@ 2>&1 >> $OUTPUT_FILE)"

if [ $? -eq 0 ]; then 
    # brilliant no issue 
    exit 0 
else
    # ask the use if they want to see the errors 
    errcount=$(echo "${lint_out}" | grep -e "Total errors found"  | grep -Eo "[0-9]+")

    echo "cpplint.py detected $RED$errcount$CLEAR lint errors for file $1"
    echo "Do you want to abort commit?"
    echo "(0) Continue anyway" 
    echo "(1) Abort"
    echo "(2) Abort and view log" 

    read choice
    if [ "$choice" -eq 1 ]; then 
        echo "Aborting commit"
        exit 1
    elif [ "$choice" -eq 2 ]; then 
        #TODO(sami): check that this actually works 
        echo "Aborting commit"
        less $OUTPUT_FILE
        exit 1
    elif [ "$choice" -eq 0 ];then 
        # proceed with commit 
        echo "Proceeding with commit.."
        exit 0 
    fi
fi

