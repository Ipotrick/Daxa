:: usage in a custom build step for visual studio:
:: compileshaders.bat $(ProjectDir) $(TargetDir)

echo validating shaders:
for /r %%v in (*.vert *.frag) do (echo validate shader %%v & glslangValidator -V "%%v")
del *.spv

echo compiling shaders:´
if not exist "%1shaders\" mkdir %1shaders
for /r %%v in (*.vert) do (echo compile shader %%v & glslc "%%v" -o %1shaders\%%~nv.vspv)
for /r %%v in (*.frag) do (echo compile shader %%v & glslc "%%v" -o %1shaders\%%~nv.fspv)
if not exist "%2shaders\" mkdir %2shaders
for /r %%v in (*.vspv *.fspv) do (copy /Y %%v %2shaders)
