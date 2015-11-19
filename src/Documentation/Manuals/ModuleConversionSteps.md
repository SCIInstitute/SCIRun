# How to convert a SCIRun 4 module to SCIRun 5 in 17 easy steps.

## 0. Set up Git+Github
https://help.github.com/articles/set-up-git/

## 1. Set up your fork of the repo.
Follow these steps with our repo.
Steps:
>https://help.github.com/articles/fork-a-repo/

Repo:
>https://github.com/SCIInstitute/SCIRun

Make sure to cover all the steps: set up Git, create a local clone of your fork, and configure upstream remotes.

## 2. Create a branch off of latest master.

Ensure your fork is synced:
> https://help.github.com/articles/syncing-a-fork/

Branch basics:
> https://help.github.com/articles/creating-and-deleting-branches-within-your-repository/

If you have an old batch of changed files (check with `git status`), you can `git stash` them or `git reset` them to continue with a clean workspace.

## 3. Build SCIRun binaries
1. Make sure Qt 4.8 is installed. 
2. You also need CMake 2.8.12 (newer versions will run into problems while building externals)
  * https://cmake.org/files/v2.8/
3. OSX/*nix instructions: Run `./build.sh` OR try this manual method:
```
cd SCIRun/bin
cmake ../Superbuild
make -j4
# after first time success, you can 
cd SCIRun 
#(that's right, SCIRun/bin/SCIRun), and just 
make -j8
```
4. Windows: 
  1. Use CMake to output Visual Studio 2013 (or newer) project files, with a binary directoy of short length. 
  2. Open bin/Superbuild.sln and build it. This will take a long time initially, since it configures the SCIRun solution file and also builds it.
  3. Once that build finishes, you can close Superbuild.sln and just work with bin/SCIRun/SCIRun.sln in Visual Studio.

Initial builds will take some time. Subsequently, unless externals have changed (rare, and we'll alert everyone), incremental builds are usually quite fast.

