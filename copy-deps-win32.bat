if "%~3"=="" (
	echo "Usage: copy-deps-win32.bat path-to-x2goclient-contrib path-to-OpenSSL-Win32 destination"
	goto :a
)
xcopy /E /Y    %1\cygwin\20160121-3_bin %3\ || exit /b  %errorlevel%
del %3\nxproxy.exe.unstripped %3\libXcomp.a %3\libXcomp.dll.a || exit /b  %errorlevel%
xcopy /E /Y    %1\libssh\0.7.4-x2go1-mingw482_bin\bin\libssh.dll %3\ || exit /b  %errorlevel%
xcopy /E /Y    %1\libssh\0.7.4-x2go1-mingw482_bin\bin\libssh_threads.dll %3\ || exit /b  %errorlevel%
xcopy /E /Y    %1\MinGW-DLLs\i686-4.8.2-release-posix-dwarf-rt_v3-rev3\libgcc_s_dw2-1.dll %3\ || exit /b  %errorlevel%
xcopy /E /Y    %1\MinGW-DLLs\i686-4.8.2-release-posix-dwarf-rt_v3-rev3\libstdc++-6.dll %3\ || exit /b  %errorlevel%
xcopy /E /Y    %1\MinGW-DLLs\i686-4.8.2-release-posix-dwarf-rt_v3-rev3\libwinpthread-1.dll %3\ || exit /b  %errorlevel%
xcopy /E /Y    %1\MSVC-DLLs\2013-12.0.21005.1-x86\msvcr120.dll %3\ || exit /b  %errorlevel%
xcopy /E /Y    %1\pulse\7.1-2.2_bin %3\ || exit /b  %errorlevel%
xcopy /E /Y    %1\PuTTY\0.70_bin %3\ || exit /b  %errorlevel%
xcopy /E /Y /I %1\VcXsrv\1.17.0.0-3_bin %3\VcXsrv || exit /b  %errorlevel%
xcopy /E /Y    %1\zlib\1.2.8_bin\zlib1.dll %3\ || exit /b  %errorlevel%
xcopy /E /Y    %1\zlib\x86-mingw4-1.2.7-1_bin\bin\libz.dll %3\ || exit /b  %errorlevel%
xcopy /E /Y    %2\bin\ssleay32.dll %3\ || exit /b  %errorlevel%
xcopy /E /Y    %2\bin\libeay32.dll %3\ || exit /b  %errorlevel%
:a
