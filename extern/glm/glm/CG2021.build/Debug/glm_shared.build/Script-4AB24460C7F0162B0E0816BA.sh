#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/zhangyang/Documents/Assignments/CG/Final/extern/glm/glm
  /usr/local/Cellar/cmake/3.21.3_1/bin/cmake -E cmake_symlink_library /Users/zhangyang/Documents/Assignments/CG/Final/bin/libglm_shared.dylib /Users/zhangyang/Documents/Assignments/CG/Final/bin/libglm_shared.dylib /Users/zhangyang/Documents/Assignments/CG/Final/bin/libglm_shared.dylib
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/zhangyang/Documents/Assignments/CG/Final/extern/glm/glm
  /usr/local/Cellar/cmake/3.21.3_1/bin/cmake -E cmake_symlink_library /Users/zhangyang/Documents/Assignments/CG/Final/bin/libglm_shared.dylib /Users/zhangyang/Documents/Assignments/CG/Final/bin/libglm_shared.dylib /Users/zhangyang/Documents/Assignments/CG/Final/bin/libglm_shared.dylib
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/zhangyang/Documents/Assignments/CG/Final/extern/glm/glm
  /usr/local/Cellar/cmake/3.21.3_1/bin/cmake -E cmake_symlink_library /Users/zhangyang/Documents/Assignments/CG/Final/bin/libglm_shared.dylib /Users/zhangyang/Documents/Assignments/CG/Final/bin/libglm_shared.dylib /Users/zhangyang/Documents/Assignments/CG/Final/bin/libglm_shared.dylib
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/zhangyang/Documents/Assignments/CG/Final/extern/glm/glm
  /usr/local/Cellar/cmake/3.21.3_1/bin/cmake -E cmake_symlink_library /Users/zhangyang/Documents/Assignments/CG/Final/bin/libglm_shared.dylib /Users/zhangyang/Documents/Assignments/CG/Final/bin/libglm_shared.dylib /Users/zhangyang/Documents/Assignments/CG/Final/bin/libglm_shared.dylib
fi

