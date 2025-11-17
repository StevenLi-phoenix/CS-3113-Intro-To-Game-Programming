# CS-UY 3113 Introduction to Game Programming

[![Make All Projects](https://github.com/StevenLi-phoenix/CS-3113-Intro-To-Game-Programming/actions/workflows/make-all.yml/badge.svg)](https://github.com/StevenLi-phoenix/CS-3113-Intro-To-Game-Programming/actions/workflows/make-all.yml)

A fork of forked from [sebastianromerocruz/CS-3113-Intro-To-Game-Programming](https://github.com/sebastianromerocruz/CS-3113-Intro-To-Game-Programming)
Leaved network for LFS.

Original README: [ORG_README.md](ORG_README.md)

# Continuous Integration

This repository ships with a GitHub Actions workflow at `.github/workflows/make-all.yml` that runs `make` inside every tracked directory containing a Makefile on each push and pull request. Each Makefile executes in its own matrix job so failures are isolated to the directory that broke. The workflow bootstraps Raylib (tag `5.0`) from source following the [official GNU/Linux instructions](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux), caches the install under `~/.local`, restores X11 build dependencies via `awalsh128/cache-apt-pkgs-action@v1`, and uploads the resulting binaries as per-directory artifacts (`build-<dir>`). Every job runs `make clean` prior to `make` to ensure a fresh build. Trigger the same behaviour locally by running `git ls-files -- '*[Mm]akefile' | xargs -I{} dirname {} | sort -u | xargs -I{} make -C {}`.

## Upstream Synchronisation

`.github/workflows/sync-upstream.yml` checks for new commits in `sebastianromerocruz/CS-3113-Intro-To-Game-Programming` every day at 09:00 UTC (and on manual dispatch). When the fork lags behind, the workflow automatically merges the upstream `main` branch into the fork's `main`, prefers upstream changes on conflicts, pushes a branch named `auto/upstream-sync`, and opens or refreshes a pull request for human review—no auto-merge is attempted and no builds are executed during this job. If the workflow fails, resolve the conflict locally and push fixes to the same branch to allow the automation to continue.
