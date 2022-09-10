/*
** LuaFileSystem
** Copyright Kepler Project 2003 - 2016 (http://keplerproject.github.io/luafilesystem)
*/

/* Define 'chdir' for systems that do not implement it */
#ifdef NO_CHDIR
  #define chdir(p)	(-1)
  #define chdir_error	"Function 'chdir' not provided by system"
#else
  #define chdir_error	strerror(errno)
#endif

#define LFS_EXPORT
#ifdef _WIN32
  #define chdir(p) (_chdir(p))
  #define getcwd(d, s) (_getcwd(d, s))
  #define rmdir(p) (_rmdir(p))
  #ifndef fileno
    #define fileno(f) (_fileno(f))
  #endif
#endif
