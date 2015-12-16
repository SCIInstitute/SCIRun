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
  2. Open bin/Superbuild.sln and build it. This will take a long time initially, since it configures the SCIRun solution file and also builds it. Remember to switch to Release mode if you want a faster build (both in build time and executable performance).
  3. Once that build finishes, you can close Superbuild.sln and just work with bin/SCIRun/SCIRun.sln in Visual Studio.

  Initial builds will take some time. Subsequently, unless externals have changed (rare, and we'll alert everyone), incremental builds are usually quite fast.

5. Build output should produce `bin/SCIRun_test` and `bin/SCIRun` bundle on Mac, and `bin/Release/SCIRun.exe` on Windows.

## 4. Essential new/convert module changes
1. Create a new `.module` file in `src/Modules/Factory/Config`. You can copy/paste an existing file and edit it in a text editor. Be careful to check every line.
2. The format is JSON and it should be self-explanatory for those who know SCIRun. Essentially it contains bits of source code that help wire up the three components of a module: algorithm, module proper, and UI implementation. 
3. The "header" paths are relative to the `src` directory.
4. The "status" and "description" strings are documentation and can be whatever you want; all the rest will turn into compiled code so you'll get a compile or link error if there is a mistake.

To convert an old module, find the v4 source code and make sure it's in a place that makes sense. Some of the module code directory tree has been simplified.

## 5. Create Module header
TODO

## 6. Edit CMakeLists.txt (between 1-5 of them, depending on layers needed and testing)
* Some old modules are where they need to be and just need a line uncommented in the CMake file; others will need to be added to a new, existing library, or in a brand new library. Steps for this: contact @dcwhite.

## 7. Edit Module source file in order to compile
TODO

## 8. Once build is green, check if new module is present in module list (before proceeding to algo and UI layers)
TODO

## 9. Commit changes to your branch, and sync your branch.
Nice guide:
> https://guides.github.com/introduction/flow/index.html

## 10. Algo layer conversion
TODO

## 11. Commit changes to your branch, and sync your branch.
TODO

## 12. GUI layer conversion
TODO

## 13. Commit changes to your branch, and sync your branch.
Do this frequently.

## 14. Add test network for new module
TODO

## 15. Add test code for algo/module/UI
TODO

## 16. Create Pull request
> https://help.github.com/articles/using-pull-requests/

## 17. Wait for feedback from a developer. 
A developer will be assigned to review the changes, and make suggestions for fixes or improvement. The key thing to remember is any change can just go on the same branch as you PRed, and it will show up on the PR page once the branch is synced. When everything is good and working, the PR will be merged into the main repo and your new/converted module is ready for everyone to use. 

Congratulations, you made a module! Proceed to step 5 and repeat until SCIRun is out of beta.
