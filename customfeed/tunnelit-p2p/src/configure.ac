AC_REVISION($Revision $)
AC_INIT(tunnelit, 0.0.1.1514)
AC_CONFIG_HEADER(config.h)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_AIX
AC_GNU_SOURCE

dnl Check required headers
AC_CHECK_HEADERS(arpa/inet.h errno.h fcntl.h netdb.h netinet/in.h stdarg.h stdio.h stdlib.h string.h sys/ioctl.h sys/select.h sys/socket.h sys/stat.h sys/types.h sys/wait.h unistd.h)
AC_CHECK_HEADERS(linux/if.h linux/if_tun.h,,, [#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif])
AC_HEADER_TIME

dnl Debug mode
AC_ARG_ENABLE(debug, AS_HELP_STRING([--enable-debug], [Enable debugging options]), [DEBUGMODE="${enableval}"], [DEBUGMODE=no])
if test "${DEBUGMODE}" = "yes"; then
	CFLAGS="-Wall -Werror -pedantic -UNDEBUG -DDEBUG=1 ${CFLAGS}"
	CFLAGS=`echo " ${CFLAGS} " | sed 's@ -g[0-9]* @ @g' | sed 's@ -O[0-9]* @ @g'`
	CFLAGS="${CFLAGS} -g3"
else
	CFLAGS="-Wall -DNDEBUG=1 -UDEBUG ${CFLAGS}"
fi

dnl Create makefile
AC_OUTPUT(Makefile)
AC_OUTPUT(tunnelit-p2p.8)
