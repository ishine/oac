# Configure paths for liboac
# Gregory Maxwell <greg@xiph.org> 08-30-2012
# Shamelessly stolen from Jack Moffitt (libogg) who
# Shamelessly stole from Owen Taylor and Manish Singh

dnl XIPH_PATH_OAC([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Test for liboac, and define OAC_CFLAGS and OAC_LIBS
dnl
AC_DEFUN([XIPH_PATH_OAC],
[dnl
dnl Get the cflags and libraries
dnl
AC_ARG_WITH(oac,AC_HELP_STRING([--with-oac=PFX],[Prefix where oac is installed (optional)]), oac_prefix="$withval", oac_prefix="")
AC_ARG_WITH(oac-libraries,AC_HELP_STRING([--with-oac-libraries=DIR],[Directory where the oac library is installed (optional)]), oac_libraries="$withval", oac_libraries="")
AC_ARG_WITH(oac-includes,AC_HELP_STRING([--with-oac-includes=DIR],[Directory where the oac header files are installed (optional)]), oac_includes="$withval", oac_includes="")
AC_ARG_ENABLE(oactest,AC_HELP_STRING([--disable-oactest],[Do not try to compile and run a test oac program]),, enable_oactest=yes)

  if test "x$oac_libraries" != "x" ; then
    OAC_LIBS="-L$oac_libraries"
  elif test "x$oac_prefix" = "xno" || test "x$oac_prefix" = "xyes" ; then
    OAC_LIBS=""
  elif test "x$oac_prefix" != "x" ; then
    OAC_LIBS="-L$oac_prefix/lib"
  elif test "x$prefix" != "xNONE" ; then
    OAC_LIBS="-L$prefix/lib"
  fi

  if test "x$oac_prefix" != "xno" ; then
    OAC_LIBS="$OAC_LIBS -loac"
  fi

  if test "x$oac_includes" != "x" ; then
    OAC_CFLAGS="-I$oac_includes"
  elif test "x$oac_prefix" = "xno" || test "x$oac_prefix" = "xyes" ; then
    OAC_CFLAGS=""
  elif test "x$oac_prefix" != "x" ; then
    OAC_CFLAGS="-I$oac_prefix/include"
  elif test "x$prefix" != "xNONE"; then
    OAC_CFLAGS="-I$prefix/include"
  fi

  AC_MSG_CHECKING(for Oac)
  if test "x$oac_prefix" = "xno" ; then
    no_oac="disabled"
    enable_oactest="no"
  else
    no_oac=""
  fi


  if test "x$enable_oactest" = "xyes" ; then
    ac_save_CFLAGS="$CFLAGS"
    ac_save_LIBS="$LIBS"
    CFLAGS="$CFLAGS $OAC_CFLAGS"
    LIBS="$LIBS $OAC_LIBS"
dnl
dnl Now check if the installed Oac is sufficiently new.
dnl
      rm -f conf.oactest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oac.h>

int main (void)
{
  system("touch conf.oactest");
  return 0;
}

],, no_oac=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
  fi

  if test "x$no_oac" = "xdisabled" ; then
     AC_MSG_RESULT(no)
     ifelse([$2], , :, [$2])
  elif test "x$no_oac" = "x" ; then
     AC_MSG_RESULT(yes)
     ifelse([$1], , :, [$1])
  else
     AC_MSG_RESULT(no)
     if test -f conf.oactest ; then
       :
     else
       echo "*** Could not run Oac test program, checking why..."
       CFLAGS="$CFLAGS $OAC_CFLAGS"
       LIBS="$LIBS $OAC_LIBS"
       AC_TRY_LINK([
#include <stdio.h>
#include <oac.h>
],     [ return 0; ],
       [ echo "*** The test program compiled, but did not run. This usually means"
       echo "*** that the run-time linker is not finding Oac or finding the wrong"
       echo "*** version of Oac. If it is not finding Oac, you'll need to set your"
       echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
       echo "*** to the installed location  Also, make sure you have run ldconfig if that"
       echo "*** is required on your system"
       echo "***"
       echo "*** If you have an old version installed, it is best to remove it, although"
       echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
       [ echo "*** The test program failed to compile or link. See the file config.log for the"
       echo "*** exact error that occurred. This usually means Oac was incorrectly installed"
       echo "*** or that you have moved Oac since it was installed." ])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
     OAC_CFLAGS=""
     OAC_LIBS=""
     ifelse([$2], , :, [$2])
  fi
  AC_SUBST(OAC_CFLAGS)
  AC_SUBST(OAC_LIBS)
  rm -f conf.oactest
])
