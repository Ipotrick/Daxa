:: usage in a custom build step for visual studio:
:: compileshaders.bat $(ProjectDir) $(TargetDir)

echo validating shaders:
for /r %%v in (*.vert *.frag) do (echo validate shader %%v & glslangValidator -V "%%v")
del *.spv

echo compiling shaders:´
if not exist "%1shaders\" mkdir %1shaders
if not exist "%2shaders\" mkdir %2shaders
cd %2
for /r %%v in (*.spv) do copy /Y %%v %1shaders
cd %1
for /r %%v in (*.vert) do (echo compile shader %%v & glslc "%%v" -o %1shaders\%%~nxv.vspv)
for /r %%v in (*.frag) do (echo compile shader %%v & glslc "%%v" -o %1shaders\%%~nxv.fspv)
for /r %%v in (*.spv) do copy /Y %%v %2shaders
