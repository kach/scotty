SCOTTY(1)                           scotty                           SCOTTY(1)



NAME
       scotty - Specify Characters On a TTY


DESCRIPTION
       Much  like  readline(3)  for  binary data: ScoTTY lets you quickly send
       arbitrary binary strings to standard output.


MOTIVATION
       Often, you want to send a binary string on the command line. For  exam-
       ple,  you  may  be  sending  shellcode  over  netcat(1)  or  seeing how
       figlet(1) behaves with accented Unicode characters.

       Unfortunately, specifying binary data is rather tricky.  Using  echo(1)
       is one option. However, there are pitfalls. For example, echo(1) adds a
       trailing newline unless you use the -n option (which several  implemen-
       tations  such as that of sh(1) fail to support). Also, escaping charac-
       ters (with the -e option) gets difficult after more than one  layer  of
       backslashes, since the shell itself also tries to do some escaping.

       A  better  option might be printf(1) but it isn't interactive: you must
       specify the string to be printed in the command, and so you cannot mod-
       ify  the payload based on the output of the command you want to pipe it
       to. You might try cat(1) with argument - to read  from  stdin  interac-
       tively,  but  this  doesn't  allow arbitrary binary input and so we are
       back where we started.

       With ScoTTY, you can do things like

              $ scotty | nc evil.site.com 1337

       and then pipe whatever binary data you wish.


INSTALLATION
       Using the attached Makefile should suffice on  most  Unix  systems.  On
       OSX,  the  attached  Ruby file acts as a Homebrew Formula. Binaries for
       Linux and OSX are available on the Releases section of this repository.
       Scotty  has  no dependencies, so it is highly portable and can be user-
       installed to a home directory, or even copied to a server  with  scp(1)
       or equivalent.


KEYBINDINGS
       ^D     [D]one with message (send current message to stdout)


       ^C     [C]lear current message (press twice to [C]ancel session)


       ^X     Erase a character


       backspace
              Erase a character


       ^L     C[l]ear  and redraw (in case the terminal output breaks for some
              reason).


       \      Input an ANSI-C string escape character (supports  \a,  \b,  \e,
              \f, \n, \r, \t, \v and \x** for a raw hex value)


       \!     Input  from shell (brings up a prompt for sh(1) and inserts std-
              out value into buffer).


MISC OTHER TRICKS
       You can set an environment variable to a binary string with

              export SOMETHING=$(scotty)

       You can "silently" read (e.g. a password) with

              scotty 2>> /dev/null | auth

       If you are piping to a program that requires a TTY for stdin,  you  can
       trick it using a pty. One way to do this is with script(1) as shown:

              $  scotty  |  script  -q  /dev/null python -c "import sys; print
              sys.stdin.isatty()"

              True



ScoTTY                            APRIL 2016                         SCOTTY(1)
