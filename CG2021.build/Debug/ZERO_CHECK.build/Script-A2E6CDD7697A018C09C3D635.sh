#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/zhangyang/Documents/Assignments/CG/Final
  make -f /Users/zhangyang/Documents/Assignments/CG/Final/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/zhangyang/Documents/Assignments/CG/Final
  make -f /Users/zhangyang/Documents/Assignments/CG/Final/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/zhangyang/Documents/Assignments/CG/Final
  make -f /Users/zhangyang/Documents/Assignments/CG/Final/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/zhangyang/Documents/Assignments/CG/Final
  make -f /Users/zhangyang/Documents/Assignments/CG/Final/CMakeScripts/ReRunCMake.make
fi

