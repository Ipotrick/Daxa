# Building

The tools needed are as follows:
 - C++ Compiler
      (I highly recommend clang on Windows, but it appears G++ is the only thing that works for Linux at the moment)
 - CMake
 - Ninja
 - vcpkg
 - Vulkan SDK
 
installing these things isn't too complicated, but I'll walk through the process for both Windows and Linux, for the sake of completeness

***

# Windows Dependency Installation Walkthrough

### C++ compiler (clang)
#### Visual Studio
We’ll begin by installing our compiler, which will be clang, but clang for windows actually relies on the Microsoft STL, and the easiest way to get that is by installing Visual Studio community with the C++ desktop development component.
#### Chocolatey
Once that’s done installing, we can go ahead and install clang. To install clang on Windows, the easiest way to do so is by getting the system package manager “Chocolatey”. To install choco (if the instructions found on the website haven't changed) we just need to open PowerShell as administrator and paste the following command
```
Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
```
Powershell might ask you to enable the running of scripts, which can be enabled by running `Set-ExecutionPolicy AllSigned`.
#### Clang
And now we can just run `choco install llvm` within our terminal. While we’re at it since choco is the best way I’ve found to install ninja, let's do that too. `choco install ninja`

### Misc tools
Now let's go ahead and install [Git](https://git-scm.com/download/win) and [CMake](https://cmake.org/download/). These ones are pretty self-explanatory, normal installation menus, so download them and go through the steps in the prompt. We can also go ahead and grab the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows), which is another installer, though this one will modify our path - which we’ll deal directly with later when installing vcpkg.

With the Vulkan SDK, we recommend disabling the "Debuggable Shader API Libraries", since there may be clashing symbols with glslang when it ultimately gets acquired by vcpkg.

![image](https://user-images.githubusercontent.com/28205981/212490845-1906e3bb-270a-4423-9e69-86f5ca05af5b.png)

With CMake 3.25 and the Vulkan SDK <=1.3.231, we experienced ABI issues in glslang. I believe this is because they may have changed the symbol lookup order with the newer version of CMake. Some further investigation is warranted.

### vcpkg

#### Downloading the repo
With vcpkg, the installation process is cloning [the git repository](https://github.com/Microsoft/vcpkg) (I recommend doing this in a folder near root. I usually use C:/dev/utils), running the bootstrap-vcpkg.bat script, and then creating an environment variable “VCPKG_ROOT” for your system.

```
mkdir C:/dev/utils
cd C:/dev/utils
git clone https://github.com/Microsoft/vcpkg
cd vcpkg
./bootstrap-vcpkg.bat
```

#### VCPKG_ROOT environment variable
Environment variables are really annoying, so let's go over this.

Add a new variable, either in user or system (it doesn’t really matter) called `VCPKG_ROOT`, with the value of the root directory of the vcpkg repository. In this case, that would be `C:/dev/utils/vcpkg`

Once you've edited your environment variables, you most likely want to restart the shell (simplest way is to do so by signing out and back in)

### That's it!
Now you can skip down to where we build the repo - which is just some basic CMake commands

***

# Linux Dependency Installation Walkthrough

### Misc packages
First we’ll just run this command to install all the necessary apt packages we currently require
```
sudo apt install git build-essential xorg-dev ninja-build cmake libtinfo5 curl zip unzip tar wayland-protocols libxkbcommon-dev
```

### vcpkg

#### Downloading the repo
Then we’ll download [the vcpkg repository](https://github.com/Microsoft/vcpkg) to any directory, just like in Windows (I like ~/dev/utils), and run the bootstrap-vcpkg.sh script. 

```
mkdir -p ~/dev/utils
cd ~/dev/utils
git clone https://github.com/Microsoft/vcpkg
cd vcpkg
./bootstrap-vcpkg.sh
```

#### VCPKG_ROOT environment variable
Then we need to add VCPKG_ROOT as an environment variable. We’ll do so by adding this line to either your `.profile` or `.bashrc` file in your home directory

```
export VCPKG_ROOT="$HOME/dev/utils/vcpkg"
```

### Vulkan SDK

#### Download the archive
Next we’ll install the Vulkan SDK. Download the tar.gz from [the lunarG site](https://vulkan.lunarg.com/sdk/home#linux) and extract it to some location (I recommend `~/dev/utils/vulkan-sdk/<sdk_version>`
#### Setting up the environment
Then we’ll just source the `setup-env.sh` script in the same bash file as before, by adding this line:
```
source "$HOME/dev/utils/vulkan-sdk/<sdk_version>/setup-env.sh"
```
And, since we’ve changed our shell script, we’ll want to log out and log back in so that it is sourced for the host environment.

### And that's it!

# Other notes

### Compiling

#### Windows
```
cmake --preset=cl-x86_64-windows-msvc
cmake --build --preset=cl-x86_64-windows-msvc-debug
```

#### Linux
```
cmake --preset=gcc-x86_64-linux-gnu
cmake --build --preset=gcc-x86_64-linux-gnu-debug
```

### Running a sample

#### Windows
```
./build/cl-x86_64-windows-msvc/tests/Debug/daxa_test_2_daxa_api_5_swapchain
```

#### Linux
```
./build/gcc-x86_64-linux-gnu/tests/Debug/daxa_test_2_daxa_api_5_swapchain
```

### Common issues
see [our page on common issues](https://github.com/Ipotrick/Daxa/tree/master/wiki/CommonProblems.md)

### How I recommend building and running

I recommend using VSCode with the official Microsoft C++ extension pack

Daxa comes with a launch script for launching samples with the VSCode debugger. First you need to select a build configuration. There should be the option of debug/release. To just build it without launching, you can press F7. This will make vcpkg pull in all the dependencies (build them if not already installed), build daxa itself, and will build all the samples. To run a sample, you select which one from the bottom, and press F5. If the code hasn’t already been built, this will also trigger a build step identical to pressing F7.

## Custom Validation (Only really applicable to Daxa developers):

```git clone https://github.com/GabeRundlett/Vulkan-ValidationLayers```
You must build this repo (Debug is fine, you get symbols)

Open up Vulkan Configurator and add a new layer profile
![Screenshot 2022-10-02 110620](https://user-images.githubusercontent.com/28205981/193466792-96e243a4-ee97-440e-8617-b01fce8af100.png)

Add a user-defined path
![Screenshot 2022-10-02 110800](https://user-images.githubusercontent.com/28205981/193466859-19dc5cdc-6dce-4a0f-bf67-aabd36a55003.png)

For me, it's at `C:/dev/projects/c++/Vulkan-ValidationLayers/build/debug-windows/layers`
![Screenshot 2022-10-02 110934](https://user-images.githubusercontent.com/28205981/193466910-7e0c6be9-7eb2-4d99-b60e-2fe5b38b64bb.png)

And then force override the validation layer
![Screenshot 2022-10-02 111055](https://user-images.githubusercontent.com/28205981/193467005-4fa15b24-0f77-4eee-a0b5-0f19e7fb5876.png)

And that should be it!
