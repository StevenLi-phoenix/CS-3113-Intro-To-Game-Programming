# Continuous Integration

This repository ships with a GitHub Actions workflow at `.github/workflows/make-all.yml` that runs `make` inside every tracked directory containing a Makefile on each push and pull request. Each Makefile executes in its own matrix job so failures are isolated to the directory that broke. Trigger the same behaviour locally by running `git ls-files -- '*[Mm]akefile' | xargs -I{} dirname {} | sort -u | xargs -I{} make -C {}`.

## Upstream Synchronisation

`.github/workflows/sync-upstream.yml` checks for new commits in `sebastianromerocruz/CS-3113-Intro-To-Game-Programming` every day at 09:00 UTC (and on manual dispatch). When the fork lags behind, the workflow automatically merges the upstream `main` branch into the fork's `main`, prefers upstream changes on conflicts, runs the `make` sweep for every Makefile, pushes a branch named `auto/upstream-sync`, and opens or refreshes a pull request for human review—no auto-merge is attempted. If the workflow fails, resolve the conflict locally and push fixes to the same branch to allow the automation to continue.
