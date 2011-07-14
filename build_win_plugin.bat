mingw32-make
copy release\npx2goplugin.dll d:\share\plugin\x2goplugin\
d:
cd  \share\plugin\x2goplugin\
idc.exe npx2goplugin.dll /idl npx2goplugin.idl -version 1.0
midl npx2goplugin.idl /nologo /tlb npx2goplugin.tlb
idc.exe npx2goplugin.dll /tlb npx2goplugin.tlb
