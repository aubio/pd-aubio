dnl Process this file with autoconf to produce a configure script

dnl Init autoconf
AC_INIT([pd-aubio], [0.2])
AC_CONFIG_SRCDIR([aubio_setup.c])

dnl Init automake
AM_INIT_AUTOMAKE([dist-bzip2 no-define])

dnl Check for Puredata
AC_CHECK_HEADER([m_pd.h],PUREDATA=y,AC_MSG_WARN([Puredata header not found.]))
AM_CONDITIONAL(PUREDATAFOUND, test "${PUREDATA}" = "y")

dnl Check for pkg-config
AC_PATH_PROG(PKG_CONFIG,pkg-config,no)

dnl Check for aubio
PKG_CHECK_MODULES(AUBIO, aubio >= 0.4.0, HAVE_AUBIO=1)

AM_CONDITIONAL(MINGW, false)
AM_CONDITIONAL(DARWIN, false)
case "${host}" in
*mingw* | *cygwin*)
  mingw32_support="yes"
  AC_CHECK_HEADER(windows.h)
  AM_CONDITIONAL(MINGW, true)
  LDFLAGS="$LDFLAGS -no-undefined"
  ;;
*darwin* | *rhapsody* | *macosx*)
  AC_ISC_POSIX
  AM_CONDITIONAL(DARWIN, true)
  ;;
*)
  AC_ISC_POSIX
  ;;
esac

dnl Create Makefiles
AC_OUTPUT([
    Makefile
])
