xcopy /E /Y    D:\x2goclient-contrib\cygwin\20141018-5_bin %1\
del %1\nxproxy.exe.unstripped %1\libXcomp.a %1\libXcomp.dll.a
xcopy /E /Y    D:\x2goclient-contrib\libssh\0.6.4-x2go1-mingw482_bin\bin\libssh.dll %1\
xcopy /E /Y    D:\x2goclient-contrib\libzip\0.9.3_bin\bin\libzip.dll %1\
xcopy /E /Y    D:\x2goclient-contrib\MinGW-DLLs\i686-4.8.2-release-posix-dwarf-rt_v3-rev3\libgcc_s_dw2-1.dll %1\
xcopy /E /Y    D:\x2goclient-contrib\MinGW-DLLs\i686-4.8.2-release-posix-dwarf-rt_v3-rev3\libstdc++-6.dll %1\
xcopy /E /Y    D:\x2goclient-contrib\MinGW-DLLs\i686-4.8.2-release-posix-dwarf-rt_v3-rev3\libwinpthread-1.dll %1\
xcopy /E /Y    D:\x2goclient-contrib\MSVC-DLLs\2008-9.0.21022.8-x86 %1\
xcopy /E /Y    D:\x2goclient-contrib\pulse\5.0-rev18_bin %1\
xcopy /E /Y    D:\x2goclient-contrib\PuTTY\0.63_bin %1\
xcopy /E /Y /I D:\x2goclient-contrib\VcXsrv\1.15.2.5_bin %1\VcXsrv
xcopy /E /Y    D:\x2goclient-contrib\zlib\1.2.8_bin\zlib1.dll %1\
xcopy /E /Y    D:\x2goclient-contrib\zlib\x86-mingw4-1.2.7-1_bin\bin\libz.dll %1\
REM until Win32OpenSSL 1.0.1k comes out
xcopy /E /Y    D:\OpenSSL-Win32\bin\ssleay32.dll %1\
xcopy /E /Y    D:\OpenSSL-Win32\bin\libeay32.dll %1\
