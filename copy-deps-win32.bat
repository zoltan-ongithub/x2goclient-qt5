if "%~3"=="" (
	echo "Usage: copy-deps-win32.bat path-to-x2goclient-contrib path-to-OpenSSL-Win32 destination"
	goto :a
)
xcopy /C /Y    %1\cygwin\20150425-2_bin %3\ || exit /b  %errorlevel%
del %3\nxproxy.exe.unstripped %3\libXcomp.a %3\libXcomp.dll.a || exit /b  %errorlevel%
xcopy /C /Y    %1\libssh\0.7.0-x2go1-mingw482_bin\bin\libssh.dll %3\ || exit /b  %errorlevel%
xcopy /C /Y    %1\libzip\0.9.3_bin\bin\libzip.dll %3\ || exit /b  %errorlevel%
xcopy /C /Y    %1\MinGW-DLLs\i686-4.8.2-release-posix-dwarf-rt_v3-rev3\libgcc_s_dw2-1.dll %3\ || exit /b  %errorlevel%
xcopy /C /Y    %1\MinGW-DLLs\i686-4.8.2-release-posix-dwarf-rt_v3-rev3\libstdc++-6.dll %3\ || exit /b  %errorlevel%
xcopy /C /Y    %1\MinGW-DLLs\i686-4.8.2-release-posix-dwarf-rt_v3-rev3\libwinpthread-1.dll %3\ || exit /b  %errorlevel%
xcopy /C /Y    %1\MSVC-DLLs\2008-9.0.21022.8-x86 %3\ || exit /b  %errorlevel%
xcopy /C /Y    %1\pulse\6.0-11.1_bin %3\ || exit /b  %errorlevel%
xcopy /C /Y    %1\PuTTY\0.64_bin %3\ || exit /b  %errorlevel%
xcopy /C /Y /I %1\VcXsrv\1.17.0.0-1_bin %3\VcXsrv || exit /b  %errorlevel%
xcopy /C /Y    %1\zlib\1.2.8_bin\zlib1.dll %3\ || exit /b  %errorlevel%
xcopy /C /Y    %1\zlib\x86-mingw4-1.2.7-1_bin\bin\libz.dll %3\ || exit /b  %errorlevel%
xcopy /C /Y    %2\bin\ssleay32.dll %3\ || exit /b  %errorlevel%
xcopy /C /Y    %2\bin\libeay32.dll %3\ || exit /b  %errorlevel%
:a
