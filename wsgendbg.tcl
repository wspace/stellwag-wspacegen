#!/bin/sh
# \
exec tclsh "$0" ${1+"$@"}

#################################################################
#                                                               #
#     T E S T A U T O M A T I O N                               #
#################################################################
#     wspacegen and wsdbg debugger using tcl and expect         #
# (c)2004 by Philippe Stellwag <linux [at] mp3s [dot] name>     #
#                                                               #
# *wspacegen debug:                                             #
#   start which an existent and correct ws file like:           #
#                                                               #
#       ./wsgendbg.tcl -u wspacegen/wspacegen hanoi.ws          #
#                                                               #
#   check if both files (ouputfile) and (hanoi.ws) are the      #
#   same, wspacegen works correctly                             #
#                                                               #
# *wsdbg debug:                                                 #
#   debug the debugger (-: that's really cool stuff !           #
#   starting wsdbg debugger like above, but the ws file mustn't #
#   be correct. if wsdbg crashes, there is always another bug   #
#################################################################

package require Tcl 8.4
package require Expect 5.4

exp_trap {exp_send_user "warning: abnormal termination of $argv0!\n\n"; exit} {SIGINT SIGTERM}

# functions
#
proc time_error { } {
    exit1 "error: timeout occur"
}

proc test_wsdbg { path_of_wsdbg ws_file } {
    puts stdout {Sorry, testing debugger currently not implemented.}
    exit1 "Lack of function(s)"
}

proc dump_command { command_ptr } {
    puts -nonewline stderr "dump_command: ([string length $command_ptr]) "

    for {set i 0} {$i < [string length $command_ptr]} {incr i} {
	switch -exact [string index $command_ptr $i] \
	    { } {puts -nonewline stderr {[SPACE]}} \
	    \t  {puts -nonewline stderr {[TAB]}} \
	    \n  {puts -nonewline stderr {[LF]}}
    }

    puts { }
}
	

proc test_wsgen_ui { path_of_wsgen ws_file } {
    # read correct ws_file example and build it
    # with wsgen_ui; after it, you can compare
    # the result
    #
    set cmdOutput {What would you do (CMD)?}
    set typeNumber {type number (int):}
    set typeLabel {an arbitrary combination}; #take only a part of the mesg
    set ws_stack [read_ws_file $ws_file]
    set timeout 5

    # strip all non-whitespace related chars
    regsub -all -- {[^ \t\n]} $ws_stack {} ws_stack
    
    exp_spawn $path_of_wsgen
    set uID $spawn_id
    for {set i 0} {$i < [string length $ws_stack]} {incr i} {
        switch -exact [string index $ws_stack $i] {
            { } {
                expect -i $uID -exact $cmdOutput {
                    exp_send -i $uID "1"
                } timeout {
                    puts stderr {[SPACE]}
                    time_error
                }

                #- okay, we're in the stack manip submenu now ...
                switch -exact [string index $ws_stack [expr $i + 1]] {
                    { } {
                        expect -i $uID -exact $cmdOutput { ;#- push to stack!
                            exp_send -i $uID "1"
                        } 

			expect -i $uID -exact $typeNumber {
			    exp_send -i $uID -- "[lindex [conv_number [string range $ws_stack [expr $i + 2] end]] 0]\r"
                            incr i [lindex [conv_number [string range $ws_stack [expr $i + 2] end]] 1]
                        } timeout {
                            puts stderr {[SPACE][SPACE]}
                            time_error
                        }

			#- back to main menu ...
                        expect -i $uID -exact $cmdOutput {
                            exp_send -i $uID "m"
			    incr i
                        } timeout { 
                            puts stderr {[SPACE][SPACE]..}
                            time_error
                        }

                    } "\t" {
                        expect -i $uID -exact $cmdOutput {
                            switch -exact [string index $ws_stack [expr $i + 2]] {
                                { } { ;# copy nth item to stack ...
				    exp_send -i $uID "3"
				    expect -i $uID -exact $typeNumber {
                                        exp_send -i $uID -- "[lindex [conv_number [string range $ws_stack [expr $i + 3] end]] 0]\r"
					incr i [lindex [conv_number [string range $ws_stack [expr $i + 3] end]] 1]
					exp_continue

				    } -exact $cmdOutput {
					exp_send -i $uID "m"
				    } default {
					puts stderr {[SPACE][TAB][SPACE]}
					time_error
				    }

                                } "\n" { ;# slide items off from stack
				    exp_send -i $uID "6"

				    expect -i $uID -exact $typeNumber {
                                        exp_send -i $uID -- "[lindex [conv_number [string range $ws_stack [expr $i + 3] end]] 0]\r"
					incr i [lindex [conv_number [string range $ws_stack [expr $i + 3] end]] 1]
					exp_continue

				    } -exact $cmdOutput {
                                        exp_send -i $uID "m"
                                    } default {
                                        puts stderr {[SPACE][TAB][LF]}
                                        time_error
                                    }
                                }
                            }                            
                        } default {
                            puts stderr {[SPACE][TAB]}
                            time_error
                        }
                        incr i 2
                    } "\n" {
                        expect -i $uID -exact $cmdOutput {
                            switch -exact [string index $ws_stack [expr $i + 2]] {
                                { } { ;#- duplicate item ...
                                    exp_send -i $uID "2"
                                } "\t" { ;#- swap top two items ...
                                    exp_send -i $uID "4"
                                } "\n" { ;#- discard top item ...
                                    exp_send -i $uID "5"
                                }
                            }
                        } default {
			    puts stderr {[SPACE][LF]}
                            time_error
                        }   

			expect -i $uID -exact $cmdOutput {
                            exp_send -i $uID "m"
                            incr i 2
                        } timeout {
                            puts stderr {[SPACE][LF]}
                            time_error
                        }
                    }
                }

            } "\t" { ;#- arithmetic stuff
                switch -exact [string index $ws_stack [expr $i + 1]] {
                    { } {
                        exp_send -i $uID "2"
                        expect -i $uID -exact $cmdOutput {
                            switch -exact "[string index $ws_stack [expr $i + 2]][string index $ws_stack [expr $i + 3]]" {
                                {  } { exp_send -i $uID "1" }
                                " \t" { exp_send -i $uID "2" }
                                " \n" { exp_send -i $uID "3" }
                                "\t " { exp_send -i $uID "4" }
                                "\t\t" { exp_send -i $uID "5" }
                            }
                            expect -i $uID -exact $cmdOutput {
                                exp_send -i $uID "m"
                                incr i 3
                            } timeout {
                                puts stderr {[TAB][SPACE]}
                                time_error
                            }
                        } timeout {
                            puts stderr {[TAB][SPACE]..}
                            time_error
                        }
                    } "\t" {
			# heap access ...
                        exp_send -i $uID "3"
                        expect -i $uID -exact $cmdOutput {
                            if {[string index $ws_stack [expr $i + 2]] == " " } { exp_send -i $uID "1" }
                            if {[string index $ws_stack [expr $i + 2]] == "\t" } { exp_send -i $uID "2" }
                            expect -i $uID -exact $cmdOutput {
                                exp_send -i $uID "m"
                                incr i 2
                            } timeout {
                                puts stderr {[TAB][TAB]}
                                time_error
                            }
                        } timeout {
                            puts stderr {[TAB][TAB]..}
                            time_error
                        }
                    } "\n" {
			# I/O stuff ...
			expect -i $uID default { puts stderr {[TAB][LF]}; time_error } \
			    -exact $cmdOutput { exp_send -i $uID "5" }

                        expect -i $uID -exact $cmdOutput {
                            if {"[string index $ws_stack [expr $i + 2]][string index $ws_stack [expr $i + 3]]" == "  " } {
                                exp_send -i $uID "1"
                            }
                            if {"[string index $ws_stack [expr $i + 2]][string index $ws_stack [expr $i + 3]]" == " \t" } {
                                exp_send -i $uID "2"
                            }
                            if {"[string index $ws_stack [expr $i + 2]][string index $ws_stack [expr $i + 3]]" == "\t " } {
                                exp_send -i $uID "3"
                            }
                            if {"[string index $ws_stack [expr $i + 2]][string index $ws_stack [expr $i + 3]]" == "\t\t" } {
                                exp_send -i $uID "4"
                            }
                        } timeout {
                            puts stderr {[TAB][LF]}
                            time_error
                        }
                        expect -i $uID -exact $cmdOutput {
                            exp_send -i $uID "m"
                            incr i 3
                        } timeout {
                            puts stderr {[TAB][LF]..}
                            time_error
                        }
                    }
                }
            } "\n" {
                expect -i $uID -exact $cmdOutput {
                    exp_send -i $uID "4"
                } timeout {
                    puts stderr {[LF]}
                    time_error
                }

		# okay, we should be in flow control menu, not having read prompt
                expect -i $uID -exact $cmdOutput {
                    switch -exact "[string range $ws_stack [expr $i + 1] [expr $i + 2]]" {
                        {  } {
			    # mark label ...
                            exp_send -i $uID "1"
                            expect -i $uID -exact $typeLabel { 
                                exp_send -i $uID "[lindex [get_label [string range $ws_stack [expr $i + 3] end]] 0]\r"
                                incr i [lindex [get_label [string range $ws_stack [expr $i + 3] end]] 1]
                            } timeout {
                                puts stderr {[LF][SPACE][SPACE]}
                                time_error
                            }
                        } " \t" {
			    # call subroutine
                            exp_send -i $uID "2"
                            expect -i $uID $typeLabel {
                                exp_send -i $uID "[lindex [get_label [string range $ws_stack [expr $i + 3] end]] 0]\r"
                                incr i [lindex [get_label [string range $ws_stack [expr $i + 3] end]] 1]
                            } timeout {
                                puts stderr {[LF][SPACE][TAB]}
                                time_error
                            }
                        } " \n" {
			    # unconditional jump!
                            exp_send -i $uID "3"
                            expect -i $uID $typeLabel {
                                exp_send -i $uID "[lindex [get_label [string range $ws_stack [expr $i + 3] end]] 0]\r"
                                incr i [lindex [get_label [string range $ws_stack [expr $i + 3] end]] 1]
                            } timeout {
                                puts stderr {[LF][SPACE][LF]}
                                time_error
                            }
                        } "\t " {
			    # jz
                            exp_send -i $uID "4"
                            expect -i $uID $typeLabel {
                                exp_send -i $uID "[lindex [get_label [string range $ws_stack [expr $i + 3] end]] 0]\r"
                                incr i [lindex [get_label [string range $ws_stack [expr $i + 3] end]] 1]
                            } timeout {
                                puts stderr {[LF][TAB][SPACE]}
                                time_error
                            }
                        } "\t\t" {
			    # jn
                            exp_send -i $uID "5"
                            expect -i $uID $typeLabel {
                                exp_send -i $uID "[lindex [get_label [string range $ws_stack [expr $i + 3] end]] 0]\r"
                                incr i [lindex [get_label [string range $ws_stack [expr $i + 3] end]] 1]
                            } timeout {
                                puts stderr {[LF][TAB][TAB]}
                                time_error
                            }
                        } "\t\n" {
			    # ret
                            exp_send -i $uID "6"
                        } "\n\n" {
			    # end
                            exp_send -i $uID "7"
                        } default {
			    puts stderr "duuh, something wicked happend, WTF?"
			    exit 99
			}
                    }
                } timeout {
                    puts stderr {[LF]}
                    time_error
                }
                expect -i $uID -exact $cmdOutput { 
		    exp_send -i $uID "m" 

		} default { puts stderr {[LF][LF][LF]}; time_error }

                incr i 2
            }
        }

	# end of overall switch statement, [string $ws_stack ($i+1)] should
	# point to beginning of next command ...

	#puts stdout "okay, command successfully executed, next one is:"
	#dump_command [string range $ws_stack [expr $i+1] [expr $i+8]];

        #exit 0 ;# exit after first command - look whether at least that works

    }
    exp_interact; #for debugging on unix (POSIX) systems
}

proc get_label { ws_num } \
{
    # this function reads the label from $ws_num (first character
    # of $ws_num the first character of the label)
    # and return it (e.g. "ttststts")  plus a value, which replaces
    # the length of the label (incl. [LF] at the end)
    #
    
    #set ws ""
    #regsub -all \n $ws_num {[LF]} ws
    #regsub -all \t $ws_num {[TAB]} ws
    #regsub -all \n $ws_num {[LF]} ws
    #puts stdout $ws
    
    set i 0;    # var for for loop (-:
    set lbl ""; # this constitute the label

    #puts "user wants us to translate this label ..."
    #dump_command [string range $ws_num 0 5]

    for {set i 0} {$i < [string length $ws_num]} {incr i} {
        switch -exact [string index $ws_num $i] {
            { } { set lbl "${lbl}s" }
            "\t" { set lbl "${lbl}t" }
            "\n" {
                if { $i == 0 } {
                    continue
                } else { break }
            }
        }
    }

    #puts "my result is: *[set lbl]*"
    if { $i == "" || $i == 0 } { set i 1 }

    #puts stderr "label skip-length is [expr $i + 1]"
    return "$lbl [expr $i + 1]"
}

proc conv_number { ws_num } {
    # this function gets a string from the first character number begins to
    # end of $ws_stack from test_wsgen_ui and return 2 decimal numbers (a con-
    # verted binary and $i of [LF]
    #
    set i 0;    # for reading it also after for_loop
    set bin ""; # binary number dispayed as "0" and "1"
    set num 0;  # converted binary number

    #puts stderr "gotta convert a number ..."
    #dump_command [string range $ws_num 0 15]

    for {set i 0} {$i < [string length $ws_num]} {incr i} {
        switch -exact [string index $ws_num $i] {
            { } { set bin "${bin}0" }
            "\t" { set bin "${bin}1" }
            "\n" { break }
        }
    }
    puts stderr "bin: $bin"
    
    # -1 because of that the first character makes the number positive or
    # negativ
    #
    for {set j 0} {$j < [expr [string length $bin] - 1]} {incr j} {
        set num [expr $num + [expr [string index $bin [expr [string length $bin] -1 - $j]] * [expr pow(2, $j)]]]
    }

    if {[string index $bin 0] == "1"} {set num [expr $num * (-1)]}

    #- be careful, the resulting number me be something like "65.0", 
    #- wspacegen however is somewhat picky when it comes to floating
    #- number (expects an int instead) ...

    if {[string first "." $num] > 0} {
        set num [string range $num 0 [expr [string first "." $num] - 1]]
    }

    #puts stderr "gonna return: $num"
    #puts -nonewline "conv_number, requesting to skip [expr $i + 1] chars!"
    return "$num [expr $i + 1]";
}

proc read_ws_file { ws_file } {
    # read ws_file and return [TAB]s, [LF]s and [SPACE]es
    #
    set fd [open $ws_file "r"]
        set tmp_ws_stack [read $fd]
    close $fd
    #puts stdout $tmp_ws_stack

    #set ws_stack ""; # wtf must I do this really )-:
    
    #for {set i 0} {$i < [string length $tmp_ws_stack]} {incr i} {
    #   switch -exact [string index $tmp_ws_stack $i] {
    #       { } { set ws_stack "$ws_stack\[SPACE\]" }
    #       "\t" { set ws_stack "$ws_stack\[TAB\]" }
    #       "\n" { set ws_stack "$ws_stack\[LF\]" }
    #   }
    #}
    #puts stdout $ws_stack

    return $tmp_ws_stack
}

proc exit1 { mesg } {
    puts stderr $mesg
    exit 1
}

# kind of main (-:
#
if { $argc != 3 } {
    exit1 "error: usage: $argv0 \[ -u \[path_of_wspacegen\] \[ws_file\] | -d \[path_of_wsdbg\] \[ws_file\] \]"
}

set timeout 5; #timeout interval in sec.

### debugging ###
#puts stdout "Number (-14): [lindex [conv_number "\t\t\t\t \n"] 1]"
#puts stdout "i (6):        [lindex [conv_number "\t\t\t\t \n"] 0]"
#exit1 ende_main

if { [lindex $argv 0] == "-u" } {
    puts stdout "path_of_wspacegen: [lindex $argv 1]"
    puts stdout "ws_file: [lindex $argv 2]"
    test_wsgen_ui [lindex $argv 1] [lindex $argv 2]
} else {
    if { [lindex $argv 0] == "-d" } {
        puts stdout "path_of_wsdbg: [lindex $argv 1]"
        puts stdout "ws_file: [lindex $argv 2]"
        test_wsdbg [lindex $argv 1] [lindex $argv 2]
    } else {
        exit1 "error: usage: $argv0 \[ -u \[path_of_wspacegen\] \[ws_file\] | -d \[path_of_wsdbg\] \[ws_file\] \]"
    }
}

exit 0


# -*- emacs is great :) -*-
# Local Variables:
# mode: Tcl
# indent-tabs-mode: nil
# end:
########

